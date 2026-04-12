/**********************************************************************
 * ESP32.ino  —  V7
 *
 * 版本变更（V7）：
 *   - OTA 配网改为纯网页配网（Captive Portal），不再依赖硬编码 SSID；
 *     上电后优先尝试 NVS 保存的 WiFi（10s），失败则开启 SquidRobot-Setup 热点；
 *   - j/k 浮沉命令改为切换式（toggle）：同向再按停止，异向直接切换；
 *   - 浮沉 s 急停新增气压平衡（E/F 同时交替开关，500ms/5s）；
 *   - 前进阀切换间隔调整为 1s。
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
#include "TeeStream.h"
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
#include "SDLogger.h"
#include "SensorHub.h"
#include "StatusDisplay.h"
#include "UltrasonicManager.h"

// HC-12 初始化前指向 USB Serial，cmdHandler.begin() 后切换到 TeeStream。
static TeeStream _dbgTee(Serial, HC12_SERIAL);
Print* g_dbg = &Serial;

CH9434A ch9434(SPI_CS, CH9434A_INT);
DepthSensorManager depthMgr;
SDLogger sdLogger;
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
constexpr uint32_t SURFACE_TIMEOUT_MS                  = 10000; // 10s 无超声波 → 出水
constexpr uint32_t SUBMERGE_CONFIRM_MS                 = 10000; // 三路超声波持续 10s 有效 → 入水
constexpr uint32_t SD_LOG_INTERVAL_MS                  = 200;   // 5 Hz

bool     gStartupDepthCalibrationDone = false;
uint32_t gLastLogMs                   = 0;
}

// ── 机器人运行模式 ────────────────────────────────────────────────────
enum RobotMode : uint8_t { ROBOT_DEBUG, ROBOT_TEST };
RobotMode gRobotMode          = ROBOT_DEBUG;
uint32_t  gLastSonarValidMs   = 0;
uint32_t  gAllSonarValidSince = 0;    // 三路同时有效起始时刻，0 表示未开始计时
bool      gManualTestMode     = false;  // true = mt 手动进入，不自动退出

// 前向声明
void enterTestMode();
void enterDebugMode();
void enterTestModeManual();

void enterTestMode() {
    if (gRobotMode == ROBOT_TEST) {
        Serial.println(F("已在测试模式中。"));
        return;
    }
    gRobotMode = ROBOT_TEST;
    g_dbg = &Serial;   // WebConsole 断开，切回 USB 串口
    WiFi.mode(WIFI_OFF);

    char folder[32];
    otaManager.getSessionFolderName(folder, sizeof(folder));
    const bool sdOk = sdLogger.startSession(folder);
    if (sdOk) {
        sdLogger.logEvent(millis(), "TEST mode started");
        Serial.println(F(">>> 测试模式：WiFi 已关闭，SD 记录已开始。"));
    } else {
        Serial.println(F(">>> 测试模式：WiFi 已关闭，但 SD 记录启动失败！"));
        Serial.println(F("    检查 SD 卡是否插好/格式化，本次 session 不会有数据。"));
    }
    Serial.println(F("    WiFi 关闭期间 FTP/OTA 不可用。输入 'md' 返回调试模式后可访问 FTP。"));
    if (gManualTestMode) {
        Serial.println(F("    手动模式：不会自动退出，输入 'md' 退出。"));
    }
}

// mt 命令触发：手动进入 TEST 模式，不受超声波超时自动退出。
void enterTestModeManual() {
    gManualTestMode = true;
    enterTestMode();
}

void enterDebugMode() {
    if (gRobotMode == ROBOT_DEBUG) {
        g_dbg->println(F("已在调试模式中。"));
        return;
    }
    const uint32_t ms = millis();
    sdLogger.logEvent(ms, "TEST mode ended");
    sdLogger.endSession(ms);
    gRobotMode          = ROBOT_DEBUG;
    gManualTestMode     = false;
    gAllSonarValidSince = 0;   // 重新开始入水确认计时

    Serial.println(F(">>> 调试模式：正在重新启用 WiFi..."));
    otaManager.begin(false);

    if (otaManager.isReady()) {
        otaManager.beginWebConsole([](uint8_t* data, size_t len) {
            String msg = String(reinterpret_cast<const char*>(data), len);
            msg.trim();
            if (msg.length() > 0) cmdHandler.processWebInput(msg);
        });
        _dbgTee.setThird(otaManager.getConsolePrint());
        g_dbg = &_dbgTee;
        Serial.println(F("WiFi 和浏览器控制台已恢复，FTP 可访问。"));
    }
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
    g_dbg->println(F("Startup depth auto-calibration complete."));
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

    // ── SD 卡（独立 SPI 总线）────────────────────────────────────────
    Serial.print(F("Initializing SD card... "));
    if (sdLogger.begin()) {
        Serial.println(F("OK"));
    } else {
        Serial.println(F("FAILED (logging disabled)"));
    }

    // ── WiFi + NTP + OTA + FTP（始终执行，用于时间校准）─────────────
    otaManager.begin(false);

    // ── 浏览器串口（WiFi 就绪后启动，访问 http://<IP>/console）─────
    if (otaManager.isReady()) {
        otaManager.beginWebConsole([](uint8_t* data, size_t len) {
            String msg = String(reinterpret_cast<const char*>(data), len);
            msg.trim();
            if (msg.length() > 0) cmdHandler.processWebInput(msg);
        });
        _dbgTee.setThird(otaManager.getConsolePrint());
        g_dbg = &_dbgTee;
    }

    // ── 深度传感器 ────────────────────────────────────────────────────
    g_dbg->print(F("Initializing MS5837 depth sensor"));
    bool depthReady = false;
    for (uint8_t attempt = 1; attempt <= DEPTH_INIT_RETRIES; ++attempt) {
        if (attempt > 1) {
            g_dbg->print(F(" | retry "));
            g_dbg->print(attempt);
        }

        if (depthMgr.begin()) {
            depthReady = true;
            break;
        }

        delay(DEPTH_INIT_RETRY_DELAY_MS);
    }

    if (depthReady) {
        g_dbg->println(F("... OK"));
    } else {
        g_dbg->print(F("... FAILED ("));
        g_dbg->print(depthMgr.getStatusText());
        g_dbg->println(F(")"));
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
    cmdHandler.setSDLogger(&sdLogger);
    cmdHandler.setEnterTestModeCallback(enterTestModeManual);  // 手动 mt → 不自动退出
    cmdHandler.setEnterDebugModeCallback(enterDebugMode);

    g_dbg->println(F("系统就绪，输入 h 查看命令列表。\n"));
}

void loop() {
    const uint32_t nowMs = millis();

    cmdHandler.update(nowMs);
    cmdHandler.processSerialInput();
    cmdHandler.processHC12Input();

    depthMgr.update();
    ultrasonicMgr.update();
    sensorHub.updateBattery();
    handleStartupDepthCalibration(nowMs);

    // 每轮只查一次传感器有效位，下游复用（避免 6 次 isValid 冗余调用）
    const bool usFrontValid = ultrasonicMgr.isValid(SENSOR_FRONT);
    const bool usLeftValid  = ultrasonicMgr.isValid(SENSOR_LEFT);
    const bool usRightValid = ultrasonicMgr.isValid(SENSOR_RIGHT);
    const bool depthValid   = depthMgr.isValid();
    const bool anyUsValid   = usFrontValid || usLeftValid || usRightValid;
    const bool allUsValid   = usFrontValid && usLeftValid && usRightValid;

    // 模式自动切换：三路同时有效 SUBMERGE_CONFIRM_MS → TEST；三路全部失效 SURFACE_TIMEOUT_MS → DEBUG
    if (allUsValid) {
        if (gAllSonarValidSince == 0) {
            gAllSonarValidSince = nowMs;
            if (gRobotMode == ROBOT_DEBUG) {
                g_dbg->print(F("[AUTO] 三路超声波已同时有效，"));
                g_dbg->print(SUBMERGE_CONFIRM_MS / 1000);
                g_dbg->println(F("s 后进入 TEST 模式..."));
            }
        }
        gLastSonarValidMs = nowMs;
        if (gRobotMode == ROBOT_DEBUG &&
            (nowMs - gAllSonarValidSince) >= SUBMERGE_CONFIRM_MS) {
            gManualTestMode = false;
            enterTestMode();
        }
    } else {
        if (gAllSonarValidSince != 0 && gRobotMode == ROBOT_DEBUG) {
            g_dbg->println(F("[AUTO] 超声波中断，入水确认已取消。"));
        }
        gAllSonarValidSince = 0;
        if (anyUsValid) {
            gLastSonarValidMs = nowMs;
        }
        if (!gManualTestMode &&
            gRobotMode == ROBOT_TEST &&
            gLastSonarValidMs > 0 &&
            (nowMs - gLastSonarValidMs) > SURFACE_TIMEOUT_MS) {
            enterDebugMode();
        }
    }

    autoNavigator.update(ultrasonicMgr, nowMs);
    depthController.update(
        depthValid,
        depthMgr.getDepthCm(),
        depthMgr.getDepthSpeedCmS(),
        depthMgr.getDepthAccelCmS2(),
        nowMs
    );
    forwardControl.update(nowMs);
    leftTurnControl.update(nowMs);
    rightTurnControl.update(nowMs);

    // SD 传感器日志：5 Hz（仅 TEST session 激活时写入）
    if (sdLogger.hasSession() && nowMs - gLastLogMs >= SD_LOG_INTERVAL_MS) {
        gLastLogMs = nowMs;
        sdLogger.logSensor(
            nowMs,
            depthValid ? depthMgr.getDepthCm()        : -1.0f,
            depthValid ? depthMgr.getDepthSpeedCmS()  : -1.0f,
            depthValid ? depthMgr.getDepthAccelCmS2() : -1.0f,
            usFrontValid ? ultrasonicMgr.getDistance(SENSOR_FRONT) / 10.0f : -1.0f,
            usLeftValid  ? ultrasonicMgr.getDistance(SENSOR_LEFT)  / 10.0f : -1.0f,
            usRightValid ? ultrasonicMgr.getDistance(SENSOR_RIGHT) / 10.0f : -1.0f,
            statusDisplay.getLastMotionStatus(),
            sensorHub.getBatteryVoltage()
        );
    }

    motionLink.applyMask(composeActuatorMask());
    motionLink.applyBuoyancy(depthController.getBuoyancyDirection(), depthController.getBuoyancyPwm());
    statusDisplay.processMinimaFeedback();
    sensorHub.displayAll();
    otaManager.handle();
}
