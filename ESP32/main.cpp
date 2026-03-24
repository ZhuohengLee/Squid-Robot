/**********************************************************************
 * main.cpp
 *
 * 这个文件是 ESP32 主控程序入口。
 * ESP32 负责读取深度和超声波，解析上位机命令，并把运动命令发送给 Minima。
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

// CH9434A 负责把 SPI 总线扩展成 4 路 UART。
CH9434A ch9434(SPI_CS, CH9434A_INT);

// 深度和超声波都挂在 CH9434A 上读取。
DepthSensorManager depthMgr(&ch9434, DEPTH_UART_CHANNEL);
UltrasonicManager ultrasonicMgr(&ch9434);
SensorHub sensorHub;

// 运动控制链路拆成独立模块，便于后续维护。
MotionLink motionLink;
ForwardControl forwardControl;
LeftTurnControl leftTurnControl;
RightTurnControl rightTurnControl;
DepthController depthController;
AutoNavigator autoNavigator;

// 串口命令和状态显示模块。
CommandHandler cmdHandler;
StatusDisplay statusDisplay;

// 启动时打印当前系统拓扑，方便串口检查接线和模块分工。
static void printWelcome() {
    Serial.println(F("\nESP32-S3 Main Controller"));
    Serial.println(F("---------------------------------------------"));
    Serial.println(F("Sensor topology:"));
    Serial.println(F("  CH9434A UART0 -> Depth sensor"));
    Serial.println(F("  CH9434A UART1 -> Front ultrasonic"));
    Serial.println(F("  CH9434A UART2 -> Left ultrasonic"));
    Serial.println(F("  CH9434A UART3 -> Right ultrasonic"));
    Serial.println(F("Motion topology:"));
    Serial.println(F("  ESP32         -> Mode logic / Kalman / Auto nav"));
    Serial.println(F("  UART1         -> Arduino Minima (motion output)"));
    Serial.println(F("---------------------------------------------"));
}

void setup() {
    // 打开 USB 调试串口。
    Serial.begin(115200);
    delay(1500);
    printWelcome();

    // 初始化 SPI 总线，给 CH9434A 使用。
    Serial.println(F("Initializing SPI..."));
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_CS);

    // 初始化 CH9434A。
    Serial.print(F("Initializing CH9434A... "));
    if (!ch9434.begin(10000000)) {
        Serial.println(F("FAILED"));
        while (1) {
            delay(1000);
        }
    }
    Serial.println(F("OK"));

    // 初始化深度传感器串口。
    Serial.print(F("Initializing depth sensor UART... "));
    if (depthMgr.begin()) {
        Serial.println(F("OK"));
    } else {
        Serial.println(F("FAILED"));
    }

    // 初始化三路超声波串口。
    Serial.print(F("Initializing ultrasonic UARTs... "));
    if (ultrasonicMgr.begin()) {
        Serial.println(F("OK"));
    } else {
        Serial.println(F("FAILED"));
    }

    // 传感器管理对象统一注入到 SensorHub 中。
    sensorHub.setDepthSensorManager(&depthMgr);
    sensorHub.setUltrasonicManager(&ultrasonicMgr);

    // 初始化运动链路和各个动作模块。
    motionLink.begin();
    forwardControl.begin(&motionLink);
    leftTurnControl.begin(&motionLink);
    rightTurnControl.begin(&motionLink);
    depthController.begin(&motionLink);
    autoNavigator.begin(&forwardControl, &leftTurnControl, &rightTurnControl);

    // 初始化命令处理器并注入全部依赖。
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
    // 先处理人工指令，再更新传感器和自动控制。
    cmdHandler.processSerialInput();

    // 轮询深度和超声波传感器。
    depthMgr.update();
    ultrasonicMgr.update();

    // 用原始深度更新卡尔曼滤波和定深控制。
    depthController.update(depthMgr.isValid(), depthMgr.getDepthCm(), millis());

    // 自动模式下依据超声波执行简单避障寻路。
    autoNavigator.update(ultrasonicMgr, millis());

    // 处理 Minima 回传的运动状态帧。
    statusDisplay.processMinimaFeedback();
}
