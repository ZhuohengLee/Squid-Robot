/**********************************************************************
 * ESP32.ino  —  V6
 *
 * 版本变更（V6）：
 *   - 新增 HC-12 无线串口接收通道（Serial2, IO1/IO2/IO47）；
 *   - 新增 HC-12 信道配对命令 ESP001~ESP127（CommandHandler 拦截处理）；
 *   - OTA WiFi 新增网页配网（Captive Portal），支持 NVS 凭据持久化，
 *     30s 未连接自动开启热点 SquidRobot-Setup；
 *   - 新增水下 WiFi 管理：启动时 3s 超声波预检测，水下跳过 WiFi；
 *     运行中检测到超声波读数则关闭 WiFi；出水 30s 后自动重启重连；
 *   - CH9434A SPI 初始化失败时自动重试 5 次，全失败后软复位 MCU；
 *   - 前进阀切换间隔调整为 500 ms。
 *
 * ESP32 负责：
 * 1. 读取深度和超声波传感器；
 * 2. 对传感器读数做卡尔曼滤波；
 * 3. 管理手动/自动模式；
 * 4. 计算前进、转向、浮沉三个子系统的执行器掩码；
 * 5. 把最终掩码发送给 Minima 执行。
 *
 * WiFi 管理策略：
 * - setup() 阶段：SPI + CH9434A + 超声波初始化完成后，做 3s 水下预检测；
 *   若检测到有效超声波读数（机器人已在水中）→ 跳过 WiFi，直接运行；
 *   否则正常启动 WiFi + OTA。
 * - loop() 阶段：若 WiFi 已启动，实时监测超声波读数；
 *   检测到读数 → 关闭 WiFi（水下减少干扰）；
 *   WiFi 已关闭且超声波超过 30s 无读数 → 重启（上岸后重新连 WiFi）。
 *********************************************************************/

#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include "AutoNavigator.h"
#include "CH9434A.h"
#include "CommandHandler.h"
#include "DepthController.h"
#include "DepthSensorManager.h"
#include "ForwardControl.h"
#include "LeftTurnControl.h"
#include "MotionLink.h"
#include "OtaManager.h"
#include "Protocol.h"
#include "RightTurnControl.h"
#include "SensorHub.h"
#include "StatusDisplay.h"
#include "UltrasonicManager.h"

CH9434A ch9434(SPI_CS, CH9434A_INT);
DepthSensorManager depthMgr;
UltrasonicManager ultrasonicMgr(&ch9434);
SensorHub sensorHub;

MotionLink motionLink;
ForwardControl forwardControl;
LeftTurnControl leftTurnControl;
RightTurnControl rightTurnControl;
DepthController depthController;
AutoNavigator autoNavigator;
OtaManager otaManager;

CommandHandler cmdHandler;
StatusDisplay statusDisplay;

namespace {
constexpr uint32_t STARTUP_DEPTH_CALIBRATION_DELAY_MS = 2000;
constexpr uint8_t  DEPTH_INIT_RETRIES                  = 3;
constexpr uint32_t DEPTH_INIT_RETRY_DELAY_MS           = 200;

// 水下预检测：上电后在此时间窗口内扫描超声波，决定是否跳过 WiFi
constexpr uint32_t UNDERWATER_PRECHECK_MS = 3000;

// loop() 中：WiFi 关闭后，超声波连续无读数多久后重启（认为已出水）
constexpr uint32_t SURFACE_RESTART_TIMEOUT_MS = 30000;

bool gStartupDepthCalibrationDone = false;

// WiFi 关闭状态追踪
bool     gWifiKilled    = false;   // loop() 中已主动关闭 WiFi
uint32_t gLastSonarMs   = 0;       // 最后一次收到有效超声波读数的时间戳
}

// 打印当前系统拓扑，方便串口检查运行模式。
static void printWelcome() {
    Serial.println(F("\nESP32-S3 Main Controller"));
    Serial.println(F("---------------------------------------------"));
    Serial.println(F("Sensor topology:"));
    Serial.println(F("  MS5837        -> ESP32 I2C (SDA=IO4, SCL=IO5, 400kHz)"));
    Serial.println(F("  CH9434A UART1 -> Front ultrasonic"));
    Serial.println(F("  CH9434A UART2 -> Left ultrasonic"));
    Serial.println(F("  CH9434A UART0 -> Right ultrasonic"));
    Serial.println(F("  CH9434A UART3 -> Unused"));
    Serial.println(F("Control topology:"));
    Serial.println(F("  ESP32         -> Timing / Kalman / Auto nav"));
    Serial.println(F("  Minima        -> Pure actuator executor"));
    Serial.println(F("---------------------------------------------"));
}

// 合成当前三个子系统的输出掩码。
static uint16_t composeActuatorMask() {
    uint16_t mask = 0;

    mask |= forwardControl.getMask();

    // 转向是互斥子系统，左右不能同时生效；最后只取一个方向。
    if (leftTurnControl.isBusy()) {
        mask |= leftTurnControl.getMask();
    } else if (rightTurnControl.isBusy()) {
        mask |= rightTurnControl.getMask();
    }

    return mask;
}

static void stayAliveForOta() {
    while (1) {
        otaManager.handle();
        delay(10);
    }
}

static void handleStartupDepthCalibration(uint32_t nowMs) {
    if (gStartupDepthCalibrationDone || nowMs < STARTUP_DEPTH_CALIBRATION_DELAY_MS) {
        return;
    }

    if (!depthMgr.isValid()) {
        return;
    }

    sensorHub.calibrateDepthZero();
    depthController.resetAfterCalibration();
    gStartupDepthCalibrationDone = true;
    Serial.println(F("Startup depth auto-calibration complete."));
}

// 返回当前是否有任意一路超声波读数有效
static bool anySonarValid() {
    return ultrasonicMgr.isValid(SENSOR_FRONT) ||
           ultrasonicMgr.isValid(SENSOR_LEFT)  ||
           ultrasonicMgr.isValid(SENSOR_RIGHT);
}

void setup() {
    Serial.begin(115200);
    delay(1500);
    printWelcome();

    // ── SPI + CH9434A（必须在超声波之前）──────────────────────────────
    Serial.println(F("Initializing SPI..."));
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_CS);

    Serial.print(F("Initializing CH9434A"));
    {
        constexpr uint8_t  CH_RETRIES      = 5;
        constexpr uint32_t CH_RETRY_DELAY  = 500;
        bool ch_ok = false;
        for (uint8_t i = 1; i <= CH_RETRIES; i++) {
            if (ch9434.begin(10000000)) {
                ch_ok = true;
                break;
            }
            Serial.printf(" [%d/%d failed]", i, CH_RETRIES);
            delay(CH_RETRY_DELAY);
        }
        if (ch_ok) {
            Serial.println(F(" OK"));
        } else {
            Serial.println(F("\nCH9434A init failed after 5 attempts — restarting in 2s..."));
            delay(2000);
            ESP.restart();
        }
    }

    // ── 超声波 UART 初始化 ─────────────────────────────────────────────
    Serial.print(F("Initializing ultrasonic UARTs... "));
    if (ultrasonicMgr.begin()) {
        Serial.println(F("OK"));
    } else {
        Serial.println(F("FAILED"));
    }

    // ── 水下预检测（3s）──────────────────────────────────────────────
    // 若开机时传感器已读到障碍物，说明机器人已在水中，跳过 WiFi
    Serial.print(F("Underwater pre-check (3s)..."));
    bool underwaterAtBoot = false;
    {
        const uint32_t precheckEnd = millis() + UNDERWATER_PRECHECK_MS;
        while (millis() < precheckEnd) {
            ultrasonicMgr.update();
            if (anySonarValid()) {
                underwaterAtBoot = true;
                break;
            }
            delay(50);
        }
    }
    if (underwaterAtBoot) {
        Serial.println(F(" UNDERWATER detected, skipping WiFi."));
        gWifiKilled  = true;
        gLastSonarMs = millis();
    } else {
        Serial.println(F(" clear."));
    }

    // ── OTA / WiFi（水下时跳过）──────────────────────────────────────
    otaManager.begin(underwaterAtBoot);

    // ── 深度传感器 ────────────────────────────────────────────────────
    Serial.print(F("Initializing MS5837 depth sensor"));
    bool depthReady = false;
    for (uint8_t attempt = 1; attempt <= DEPTH_INIT_RETRIES; ++attempt) {
        if (attempt > 1) {
            Serial.print(F(" | retry "));
            Serial.print(attempt);
        }

        if (depthMgr.begin()) {
            depthReady = true;
            break;
        }

        delay(DEPTH_INIT_RETRY_DELAY_MS);
    }

    if (depthReady) {
        Serial.println(F("... OK"));
    } else {
        Serial.print(F("... FAILED ("));
        Serial.print(depthMgr.getStatusText());
        Serial.println(F(")"));
    }

    // ── 传感器 Hub 配置 ───────────────────────────────────────────────
    sensorHub.setDepthSensorManager(&depthMgr);
    sensorHub.setStatusDisplay(&statusDisplay);
    sensorHub.setUltrasonicManager(&ultrasonicMgr);

    // ── 运动控制初始化 ────────────────────────────────────────────────
    motionLink.begin();
    forwardControl.begin();
    leftTurnControl.begin();
    rightTurnControl.begin();
    depthController.begin();
    autoNavigator.begin(&forwardControl, &leftTurnControl, &rightTurnControl);

    // ── 命令处理器配置 ────────────────────────────────────────────────
    cmdHandler.begin();
    cmdHandler.setSensorHub(&sensorHub);
    cmdHandler.setStatusDisplay(&statusDisplay);
    cmdHandler.setMotionLink(&motionLink);
    cmdHandler.setForwardControl(&forwardControl);
    cmdHandler.setLeftTurnControl(&leftTurnControl);
    cmdHandler.setRightTurnControl(&rightTurnControl);
    cmdHandler.setDepthController(&depthController);
    cmdHandler.setAutoNavigator(&autoNavigator);

    Serial.println(F("System ready. Type h for commands.\n"));
}

void loop() {
    const uint32_t nowMs = millis();

    cmdHandler.processSerialInput();
    cmdHandler.processHC12Input();

    depthMgr.update();
    ultrasonicMgr.update();
    handleStartupDepthCalibration(nowMs);

    // ── 水下 WiFi 管理 ────────────────────────────────────────────────
    if (anySonarValid()) {
        gLastSonarMs = nowMs;
        if (!gWifiKilled && otaManager.isReady()) {
            // 入水后检测到超声波读数 → 关闭 WiFi，减少水下干扰
            WiFi.mode(WIFI_OFF);
            gWifiKilled = true;
            Serial.println(F("Underwater detected: WiFi disabled."));
        }
    } else if (gWifiKilled && gLastSonarMs > 0 &&
               (nowMs - gLastSonarMs) > SURFACE_RESTART_TIMEOUT_MS) {
        // WiFi 已关闭且超声波 30s 无读数 → 认为已出水，重启以重连 WiFi
        Serial.println(F("Surfaced detected: restarting to re-enable WiFi..."));
        delay(500);
        ESP.restart();
    }
    // ─────────────────────────────────────────────────────────────────

    autoNavigator.update(ultrasonicMgr, nowMs);
    depthController.update(
        depthMgr.isValid(),
        depthMgr.getDepthCm(),
        depthMgr.getDepthSpeedCmS(),
        depthMgr.getDepthAccelCmS2(),
        nowMs
    );
    forwardControl.update(nowMs);
    leftTurnControl.update(nowMs);
    rightTurnControl.update(nowMs);

    motionLink.applyMask(composeActuatorMask());
    motionLink.applyBuoyancy(depthController.getBuoyancyDirection(), depthController.getBuoyancyPwm());
    statusDisplay.processMinimaFeedback();
    sensorHub.displayAll();
    otaManager.handle();
}
