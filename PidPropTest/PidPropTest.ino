/**********************************************************************
 * PidPropTest.ino — 浮沉 PID(模拟) + 推进 同时工作 干扰验证测试
 *
 * 目的：桌面/水面台架(未组装、无深度&超声波传感器)上，验证
 *       "推进系统与浮沉 PID 同时工作"时的气压耦合/倒灌。
 *       配套假设见 docs/浮沉-推进气压耦合实验.md。
 *
 * 没有真实传感器，深度是【模拟】的：
 *   用一个【虚拟深度闭环】替代传感器——浮沉指令(下沉+/上浮-)按 PWM 推动
 *   "虚拟深度"，真实 DepthController PID 据此闭环；目标深度每隔几秒切换，
 *   逼 PID 持续做上浮/下沉。
 *   ⚠ 遥测"模拟深度"只是【PID 以为的深度】，不是真实气体行为。
 *     真正的倒灌请【肉眼盯浮沉气囊(针筒)】。
 *
 * 串口命令：
 *   s  开始模拟(推进 + 浮沉PID 一起跑)
 *   e  结束模拟(全部急停)
 *   1/2/3  PID 驱动强度三档(浮沉泵 PWM 上限 120/180/255)
 *
 * 复用真实控制类：ForwardControl、DepthController、MotionLink。
 *   执行器在 Minima(需已烧"普通同步"固件 + 通电)。上电默认【待机】。
 *********************************************************************/

#include "Protocol.h"
#include "MotionLink.h"
#include "ForwardControl.h"
#include "DepthController.h"

MotionLink      motionLink;
ForwardControl  forwardControl;
DepthController depthController;

bool          g_running = false;                        // 模拟是否在跑(默认待机)

uint8_t       g_gear = 2;                                // 三档 PWM 上限
const uint8_t GEAR_PWM_CAP[4] = { 0, 120, 180, 255 };

// ── 虚拟深度闭环(替代没接的深度传感器) ──
float    g_simDepth  = 30.0f, g_prevDepth = 30.0f;
float    g_simSpeed  = 0.0f,  g_prevSpeed = 0.0f, g_simAccel = 0.0f;
const float    SIM_GAIN = 0.03f;               // cm/(pwm·s)，虚拟模型增益(粗略)
const float    TGT_A = 20.0f, TGT_B = 40.0f;
const uint32_t TGT_PERIOD_MS = 8000;           // 目标每 8s 切换，逼 PID 持续动作
bool     g_targetIsA = true;
uint32_t g_lastTgtMs = 0;

uint8_t  g_lastDir = BUOYANCY_STOP;            // 上一步下发的浮沉指令(喂虚拟模型)
uint8_t  g_lastPwm = 0;

const uint32_t CTRL_PERIOD_MS = 50;            // 20Hz 控制步
uint32_t g_lastCtrlMs = 0;
const uint32_t TELEMETRY_INTERVAL_MS = 200;    // 5Hz 遥测
uint32_t g_lastTeleMs = 0;

const __FlashStringHelper* dirName(uint8_t d) {
    switch (d) {
        case BUOYANCY_ASCEND:  return F("ASC ");
        case BUOYANCY_DESCEND: return F("DESC");
        case BUOYANCY_BALANCE: return F("BAL ");
        default:               return F("STOP");
    }
}

void startSim() {
    // 复位虚拟状态
    g_simDepth = 30.0f; g_prevDepth = 30.0f;
    g_simSpeed = 0.0f;  g_prevSpeed = 0.0f; g_simAccel = 0.0f;
    g_targetIsA = true;
    g_lastDir = BUOYANCY_STOP; g_lastPwm = 0;
    const uint32_t now = millis();
    g_lastTgtMs = now; g_lastCtrlMs = now; g_lastTeleMs = now;

    depthController.resetAfterCalibration();
    depthController.setTargetDepth(TGT_A);
    forwardControl.start();                    // 推进开始持续往复
    g_running = true;
    Serial.println(F(">>> 开始模拟：推进 + 浮沉PID 同时工作"));
}

void stopSim() {
    forwardControl.emergencyStop();
    depthController.manualStop();
    motionLink.emergencyStop();                // 通知 Minima 全部停
    g_lastDir = BUOYANCY_STOP; g_lastPwm = 0;
    g_running = false;
    Serial.println(F(">>> 结束模拟：全部急停"));
}

void printTelemetry() {
    Serial.print(F("[档"));
    Serial.print(g_gear);
    Serial.print(F("] 模拟深度="));
    Serial.print(g_simDepth, 1);
    Serial.print(F("cm 目标="));
    Serial.print(depthController.getTargetDepthCm(), 1);
    Serial.print(F(" u="));
    Serial.print(depthController.getControlOutput(), 0);
    Serial.print(F(" 浮沉="));
    Serial.print(dirName(g_lastDir));
    Serial.print(F(" pwm="));
    Serial.print(g_lastPwm);
    Serial.print('/');
    Serial.print(GEAR_PWM_CAP[g_gear]);
    Serial.print(F(" 推进="));
    Serial.print(forwardControl.isRunning()  ? F("ON")
               : forwardControl.isBalancing() ? F("BAL") : F("OFF"));
    Serial.println();
}

void handleSerial() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c >= '1' && c <= '3') {
            g_gear = (uint8_t)(c - '0');
            Serial.print(F(">>> 档位 "));
            Serial.print(g_gear);
            Serial.print(F(" (浮沉PWM上限="));
            Serial.print(GEAR_PWM_CAP[g_gear]);
            Serial.println(F(")"));
        } else if (c == 's' || c == 'S') {
            if (g_running) stopSim(); else startSim();   // s 键：开始/结束 切换
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(300);
    Serial.println(F("\n=== 浮沉PID(模拟) + 推进 干扰测试 ==="));
    Serial.println(F("命令: s=开始/结束(切换)  1/2/3=PID驱动强度档位"));
    Serial.println(F(">> 倒灌请肉眼盯浮沉气囊；'模拟深度'只是PID以为的值。"));

    motionLink.begin();
    depthController.begin();
    forwardControl.begin();                    // 仅初始化引脚，不启动
    stopSim();                                 // 上电默认待机，确保执行器全停

    Serial.println(F("--- 待机中，输入 s 开始 ---"));
}

void loop() {
    const uint32_t now = millis();

    handleSerial();

    if (!g_running) {
        // 待机：每 1s 提示一次
        if (now - g_lastTeleMs >= 1000) {
            g_lastTeleMs = now;
            Serial.println(F("[待机] 输入 s 开始模拟"));
        }
        return;
    }

    // 目标深度周期切换，逼 PID 持续上浮/下沉
    if (now - g_lastTgtMs >= TGT_PERIOD_MS) {
        g_lastTgtMs = now;
        g_targetIsA = !g_targetIsA;
        depthController.setTargetDepth(g_targetIsA ? TGT_A : TGT_B);
    }

    // 推进往复时序
    forwardControl.update(now);
    motionLink.applyMask(forwardControl.getMask());

    // 20Hz 控制步：虚拟模型 → 真实 PID → 下发浮沉
    if (now - g_lastCtrlMs >= CTRL_PERIOD_MS) {
        const float dt = (now - g_lastCtrlMs) * 0.001f;
        g_lastCtrlMs = now;

        float effect = (g_lastDir == BUOYANCY_DESCEND) ?  (float)g_lastPwm
                     : (g_lastDir == BUOYANCY_ASCEND)  ? -(float)g_lastPwm : 0.0f;
        g_simDepth += SIM_GAIN * effect * dt;
        if (g_simDepth < 0.0f)   g_simDepth = 0.0f;
        if (g_simDepth > 100.0f) g_simDepth = 100.0f;

        g_simSpeed = (g_simDepth - g_prevDepth) / dt;
        g_simAccel = (g_simSpeed - g_prevSpeed) / dt;
        g_prevDepth = g_simDepth;
        g_prevSpeed = g_simSpeed;

        depthController.update(true, g_simDepth, g_simSpeed, g_simAccel, now);

        g_lastDir = depthController.getBuoyancyDirection();
        g_lastPwm = depthController.getBuoyancyPwm();
        if (g_lastPwm > GEAR_PWM_CAP[g_gear]) g_lastPwm = GEAR_PWM_CAP[g_gear];
        motionLink.applyBuoyancy(g_lastDir, g_lastPwm);
    }

    if (now - g_lastTeleMs >= TELEMETRY_INTERVAL_MS) {
        g_lastTeleMs = now;
        printTelemetry();
    }
}
