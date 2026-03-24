/**********************************************************************
 * ESP32.ino
 *
 * 杩欐槸 ESP32 涓绘帶鑽夊浘鍏ュ彛銆? * ESP32 璐熻矗锛? * 1. 璇诲彇娣卞害鍜岃秴澹版尝浼犳劅鍣紱
 * 2. 瀵逛紶鎰熷櫒璇绘暟鍋氬崱灏旀浖婊ゆ尝锛? * 3. 绠＄悊鎵嬪姩/鑷姩妯″紡锛? * 4. 璁＄畻鍓嶈繘銆佽浆鍚戙€佹诞娌変笁涓瓙绯荤粺鐨勬墽琛屽櫒鎺╃爜锛? * 5. 鎶婃渶缁堟帺鐮佸彂閫佺粰 Minima 鎵ц銆? *********************************************************************/

// 中文逐行说明：下面这一行保留原始代码 -> #include <Arduino.h>
#include <Arduino.h>
// 中文逐行说明：下面这一行保留原始代码 -> #include <SPI.h>
#include <SPI.h>
// 中文逐行说明：下面这一行保留原始代码 -> #include "AutoNavigator.h"
#include "AutoNavigator.h"
// 中文逐行说明：下面这一行保留原始代码 -> #include "CH9434A.h"
#include "CH9434A.h"
// 中文逐行说明：下面这一行保留原始代码 -> #include "CommandHandler.h"
#include "CommandHandler.h"
// 中文逐行说明：下面这一行保留原始代码 -> #include "DepthController.h"
#include "DepthController.h"
// 中文逐行说明：下面这一行保留原始代码 -> #include "DepthSensorManager.h"
#include "DepthSensorManager.h"
// 中文逐行说明：下面这一行保留原始代码 -> #include "ForwardControl.h"
#include "ForwardControl.h"
// 中文逐行说明：下面这一行保留原始代码 -> #include "LeftTurnControl.h"
#include "LeftTurnControl.h"
// 中文逐行说明：下面这一行保留原始代码 -> #include "MotionLink.h"
#include "MotionLink.h"
// 中文逐行说明：下面这一行保留原始代码 -> #include "Protocol.h"
#include "Protocol.h"
// 中文逐行说明：下面这一行保留原始代码 -> #include "RightTurnControl.h"
#include "RightTurnControl.h"
// 中文逐行说明：下面这一行保留原始代码 -> #include "SensorHub.h"
#include "SensorHub.h"
// 中文逐行说明：下面这一行保留原始代码 -> #include "StatusDisplay.h"
#include "StatusDisplay.h"
// 中文逐行说明：下面这一行保留原始代码 -> #include "UltrasonicManager.h"
#include "UltrasonicManager.h"

// 中文逐行说明：下面这一行保留原始代码 -> CH9434A ch9434(SPI_CS, CH9434A_INT);
CH9434A ch9434(SPI_CS, CH9434A_INT);
// 中文逐行说明：下面这一行保留原始代码 -> DepthSensorManager depthMgr(&ch9434, DEPTH_UART_CHANNEL);
DepthSensorManager depthMgr(&ch9434, DEPTH_UART_CHANNEL);
// 中文逐行说明：下面这一行保留原始代码 -> UltrasonicManager ultrasonicMgr(&ch9434);
UltrasonicManager ultrasonicMgr(&ch9434);
// 中文逐行说明：下面这一行保留原始代码 -> SensorHub sensorHub;
SensorHub sensorHub;

// 中文逐行说明：下面这一行保留原始代码 -> MotionLink motionLink;
MotionLink motionLink;
// 中文逐行说明：下面这一行保留原始代码 -> ForwardControl forwardControl;
ForwardControl forwardControl;
// 中文逐行说明：下面这一行保留原始代码 -> LeftTurnControl leftTurnControl;
LeftTurnControl leftTurnControl;
// 中文逐行说明：下面这一行保留原始代码 -> RightTurnControl rightTurnControl;
RightTurnControl rightTurnControl;
// 中文逐行说明：下面这一行保留原始代码 -> DepthController depthController;
DepthController depthController;
// 中文逐行说明：下面这一行保留原始代码 -> AutoNavigator autoNavigator;
AutoNavigator autoNavigator;

// 中文逐行说明：下面这一行保留原始代码 -> CommandHandler cmdHandler;
CommandHandler cmdHandler;
// 中文逐行说明：下面这一行保留原始代码 -> StatusDisplay statusDisplay;
StatusDisplay statusDisplay;

// 鎵撳嵃褰撳墠绯荤粺鎷撴墤锛屾柟渚夸覆鍙ｆ鏌ヨ繍琛屾ā寮忋€?static void printWelcome() {
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("\nESP32-S3 Main Controller"));
    Serial.println(F("\nESP32-S3 Main Controller"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("---------------------------------------------"));
    Serial.println(F("---------------------------------------------"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("Sensor topology:"));
    Serial.println(F("Sensor topology:"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("  CH9434A UART0 -> Depth sensor"));
    Serial.println(F("  CH9434A UART0 -> Depth sensor"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("  CH9434A UART1 -> Front ultrasonic"));
    Serial.println(F("  CH9434A UART1 -> Front ultrasonic"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("  CH9434A UART2 -> Left ultrasonic"));
    Serial.println(F("  CH9434A UART2 -> Left ultrasonic"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("  CH9434A UART3 -> Right ultrasonic"));
    Serial.println(F("  CH9434A UART3 -> Right ultrasonic"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("Control topology:"));
    Serial.println(F("Control topology:"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("  ESP32         -> Timing / Kalman / Auto nav"));
    Serial.println(F("  ESP32         -> Timing / Kalman / Auto nav"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("  Minima        -> Pure actuator executor"));
    Serial.println(F("  Minima        -> Pure actuator executor"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("---------------------------------------------"));
    Serial.println(F("---------------------------------------------"));
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 鍚堟垚褰撳墠涓変釜瀛愮郴缁熺殑杈撳嚭鎺╃爜銆?static uint16_t composeActuatorMask() {
    // 中文逐行说明：下面这一行保留原始代码 -> uint16_t mask = 0;
    uint16_t mask = 0;

    // 中文逐行说明：下面这一行保留原始代码 -> mask |= forwardControl.getMask();
    mask |= forwardControl.getMask();

    // 杞悜鏄簰鏂ュ瓙绯荤粺锛屽乏鍙充笉鑳藉悓鏃剁敓鏁堬紱鏈€鍚庡彧鍙栦竴涓柟鍚戙€?    if (leftTurnControl.isBusy()) {
        // 中文逐行说明：下面这一行保留原始代码 -> mask |= leftTurnControl.getMask();
        mask |= leftTurnControl.getMask();
    // 中文逐行说明：下面这一行保留原始代码 -> } else if (rightTurnControl.isBusy()) {
    } else if (rightTurnControl.isBusy()) {
        // 中文逐行说明：下面这一行保留原始代码 -> mask |= rightTurnControl.getMask();
        mask |= rightTurnControl.getMask();
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> mask |= depthController.getMask();
    mask |= depthController.getMask();
    // 中文逐行说明：下面这一行保留原始代码 -> return mask;
    return mask;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void setup() {
void setup() {
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.begin(115200);
    Serial.begin(115200);
    // 中文逐行说明：下面这一行保留原始代码 -> delay(1500);
    delay(1500);
    // 中文逐行说明：下面这一行保留原始代码 -> printWelcome();
    printWelcome();

    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("Initializing SPI..."));
    Serial.println(F("Initializing SPI..."));
    // 中文逐行说明：下面这一行保留原始代码 -> SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_CS);
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_CS);

    // 中文逐行说明：下面这一行保留原始代码 -> Serial.print(F("Initializing CH9434A... "));
    Serial.print(F("Initializing CH9434A... "));
    // 中文逐行说明：下面这一行保留原始代码 -> if (!ch9434.begin(10000000)) {
    if (!ch9434.begin(10000000)) {
        // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("FAILED"));
        Serial.println(F("FAILED"));
        // 中文逐行说明：下面这一行保留原始代码 -> while (1) {
        while (1) {
            // 中文逐行说明：下面这一行保留原始代码 -> delay(1000);
            delay(1000);
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("OK"));
    Serial.println(F("OK"));

    // 中文逐行说明：下面这一行保留原始代码 -> Serial.print(F("Initializing depth sensor UART... "));
    Serial.print(F("Initializing depth sensor UART... "));
    // 中文逐行说明：下面这一行保留原始代码 -> if (depthMgr.begin()) {
    if (depthMgr.begin()) {
        // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("OK"));
        Serial.println(F("OK"));
    // 中文逐行说明：下面这一行保留原始代码 -> } else {
    } else {
        // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("FAILED"));
        Serial.println(F("FAILED"));
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> Serial.print(F("Initializing ultrasonic UARTs... "));
    Serial.print(F("Initializing ultrasonic UARTs... "));
    // 中文逐行说明：下面这一行保留原始代码 -> if (ultrasonicMgr.begin()) {
    if (ultrasonicMgr.begin()) {
        // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("OK"));
        Serial.println(F("OK"));
    // 中文逐行说明：下面这一行保留原始代码 -> } else {
    } else {
        // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("FAILED"));
        Serial.println(F("FAILED"));
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> sensorHub.setDepthSensorManager(&depthMgr);
    sensorHub.setDepthSensorManager(&depthMgr);
    // 中文逐行说明：下面这一行保留原始代码 -> sensorHub.setUltrasonicManager(&ultrasonicMgr);
    sensorHub.setUltrasonicManager(&ultrasonicMgr);

    // 中文逐行说明：下面这一行保留原始代码 -> motionLink.begin();
    motionLink.begin();
    // 中文逐行说明：下面这一行保留原始代码 -> forwardControl.begin();
    forwardControl.begin();
    // 中文逐行说明：下面这一行保留原始代码 -> leftTurnControl.begin();
    leftTurnControl.begin();
    // 中文逐行说明：下面这一行保留原始代码 -> rightTurnControl.begin();
    rightTurnControl.begin();
    // 中文逐行说明：下面这一行保留原始代码 -> depthController.begin();
    depthController.begin();
    // 中文逐行说明：下面这一行保留原始代码 -> autoNavigator.begin(&forwardControl, &leftTurnControl, &rightTurnControl);
    autoNavigator.begin(&forwardControl, &leftTurnControl, &rightTurnControl);

    // 中文逐行说明：下面这一行保留原始代码 -> cmdHandler.begin();
    cmdHandler.begin();
    // 中文逐行说明：下面这一行保留原始代码 -> cmdHandler.setSensorHub(&sensorHub);
    cmdHandler.setSensorHub(&sensorHub);
    // 中文逐行说明：下面这一行保留原始代码 -> cmdHandler.setStatusDisplay(&statusDisplay);
    cmdHandler.setStatusDisplay(&statusDisplay);
    // 中文逐行说明：下面这一行保留原始代码 -> cmdHandler.setMotionLink(&motionLink);
    cmdHandler.setMotionLink(&motionLink);
    // 中文逐行说明：下面这一行保留原始代码 -> cmdHandler.setForwardControl(&forwardControl);
    cmdHandler.setForwardControl(&forwardControl);
    // 中文逐行说明：下面这一行保留原始代码 -> cmdHandler.setLeftTurnControl(&leftTurnControl);
    cmdHandler.setLeftTurnControl(&leftTurnControl);
    // 中文逐行说明：下面这一行保留原始代码 -> cmdHandler.setRightTurnControl(&rightTurnControl);
    cmdHandler.setRightTurnControl(&rightTurnControl);
    // 中文逐行说明：下面这一行保留原始代码 -> cmdHandler.setDepthController(&depthController);
    cmdHandler.setDepthController(&depthController);
    // 中文逐行说明：下面这一行保留原始代码 -> cmdHandler.setAutoNavigator(&autoNavigator);
    cmdHandler.setAutoNavigator(&autoNavigator);

    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("System ready. Type h for commands.\n"));
    Serial.println(F("System ready. Type h for commands.\n"));
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void loop() {
void loop() {
    // 中文逐行说明：下面这一行保留原始代码 -> const uint32_t nowMs = millis();
    const uint32_t nowMs = millis();

    // 中文逐行说明：下面这一行保留原始代码 -> cmdHandler.processSerialInput();
    cmdHandler.processSerialInput();

    // 中文逐行说明：下面这一行保留原始代码 -> depthMgr.update();
    depthMgr.update();
    // 中文逐行说明：下面这一行保留原始代码 -> ultrasonicMgr.update();
    ultrasonicMgr.update();

    // 中文逐行说明：下面这一行保留原始代码 -> autoNavigator.update(ultrasonicMgr, nowMs);
    autoNavigator.update(ultrasonicMgr, nowMs);
    // 中文逐行说明：下面这一行保留原始代码 -> depthController.update(depthMgr.isValid(), depthMgr.getDepthCm(), depthMgr.getDepthSpeedCmS(), nowMs);
    depthController.update(depthMgr.isValid(), depthMgr.getDepthCm(), depthMgr.getDepthSpeedCmS(), nowMs);
    // 中文逐行说明：下面这一行保留原始代码 -> forwardControl.update(nowMs);
    forwardControl.update(nowMs);
    // 中文逐行说明：下面这一行保留原始代码 -> leftTurnControl.update(nowMs);
    leftTurnControl.update(nowMs);
    // 中文逐行说明：下面这一行保留原始代码 -> rightTurnControl.update(nowMs);
    rightTurnControl.update(nowMs);

    // 中文逐行说明：下面这一行保留原始代码 -> motionLink.applyMask(composeActuatorMask());
    motionLink.applyMask(composeActuatorMask());
    // 中文逐行说明：下面这一行保留原始代码 -> statusDisplay.processMinimaFeedback();
    statusDisplay.processMinimaFeedback();
// 中文逐行说明：下面这一行保留原始代码 -> }
}
