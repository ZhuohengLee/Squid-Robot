/**********************************************************************
 * ESP32.ino
 *
 * 这是 ESP32 主控草图入口。
 * ESP32 负责：
 * 1. 读取深度和超声波传感器；
 * 2. 对传感器读数做卡尔曼滤波；
 * 3. 管理手动/自动模式；
 * 4. 计算前进、转向、浮沉三个子系统的执行器掩码；
 * 5. 把最终掩码发送给 Minima 执行。
 *********************************************************************/

#include <Arduino.h>
#include <SPI.h>
#include "AutoNavigator.h"
#include "CH9434A.h"
#include "CommandHandler.h"
#include "DepthController.h"
#include "DepthSensorManager.h"
#include "ForwardControl.h"
#include "LeftTurnControl.h"
#include "MotionLink.h"
#include "Protocol.h"
#include "RightTurnControl.h"
#include "SensorHub.h"
#include "StatusDisplay.h"
#include "UltrasonicManager.h"

CH9434A ch9434(SPI_CS, CH9434A_INT);
DepthSensorManager depthMgr(&ch9434, DEPTH_UART_CHANNEL);
UltrasonicManager ultrasonicMgr(&ch9434);
SensorHub sensorHub;

MotionLink motionLink;
ForwardControl forwardControl;
LeftTurnControl leftTurnControl;
RightTurnControl rightTurnControl;
DepthController depthController;
AutoNavigator autoNavigator;

CommandHandler cmdHandler;
StatusDisplay statusDisplay;

// 打印当前系统拓扑，方便串口检查运行模式。
static void printWelcome() {
    Serial.println(F("\nESP32-S3 Main Controller"));
    Serial.println(F("---------------------------------------------"));
    Serial.println(F("Sensor topology:"));
    Serial.println(F("  CH9434A UART0 -> Depth sensor"));
    Serial.println(F("  CH9434A UART1 -> Front ultrasonic"));
    Serial.println(F("  CH9434A UART2 -> Left ultrasonic"));
    Serial.println(F("  CH9434A UART3 -> Right ultrasonic"));
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

    mask |= depthController.getMask();
    return mask;
}

void setup() {
    Serial.begin(115200);
    delay(1500);
    printWelcome();

    Serial.println(F("Initializing SPI..."));
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_CS);

    Serial.print(F("Initializing CH9434A... "));
    if (!ch9434.begin(10000000)) {
        Serial.println(F("FAILED"));
        while (1) {
            delay(1000);
        }
    }
    Serial.println(F("OK"));

    Serial.print(F("Initializing depth sensor UART... "));
    if (depthMgr.begin()) {
        Serial.println(F("OK"));
    } else {
        Serial.println(F("FAILED"));
    }

    Serial.print(F("Initializing ultrasonic UARTs... "));
    if (ultrasonicMgr.begin()) {
        Serial.println(F("OK"));
    } else {
        Serial.println(F("FAILED"));
    }

    sensorHub.setDepthSensorManager(&depthMgr);
    sensorHub.setUltrasonicManager(&ultrasonicMgr);

    motionLink.begin();
    forwardControl.begin();
    leftTurnControl.begin();
    rightTurnControl.begin();
    depthController.begin();
    autoNavigator.begin(&forwardControl, &leftTurnControl, &rightTurnControl);

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

    depthMgr.update();
    ultrasonicMgr.update();

    autoNavigator.update(ultrasonicMgr, nowMs);
    depthController.update(depthMgr.isValid(), depthMgr.getDepthCm(), depthMgr.getDepthSpeedCmS(), nowMs);
    forwardControl.update(nowMs);
    leftTurnControl.update(nowMs);
    rightTurnControl.update(nowMs);

    motionLink.applyMask(composeActuatorMask());
    statusDisplay.processMinimaFeedback();
}
