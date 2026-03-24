/**********************************************************************
 * Minima.ino
 *
 * 杩欐槸 Arduino Minima 鎵ц鍣ㄨ崏鍥惧叆鍙ｃ€? * Minima 鍙仛涓変欢浜嬶細
 * 1. 鎺ユ敹 ESP32 涓嬪彂鐨勬墽琛屽櫒鎺╃爜锛? * 2. 鎶婃帺鐮佺洿鎺ュ啓鍒扮湡瀹炲紩鑴氾紱
 * 3. 鍥炰紶褰撳墠鍓嶈繘/杞悜/娴矇鐘舵€併€? *********************************************************************/

// 中文逐行说明：下面这一行保留原始代码 -> #include <Arduino.h>
#include <Arduino.h>
// 中文逐行说明：下面这一行保留原始代码 -> #include "MotionControl.h"
#include "MotionControl.h"
// 中文逐行说明：下面这一行保留原始代码 -> #include "Protocol.h"
#include "Protocol.h"

// 中文逐行说明：下面这一行保留原始代码 -> #define UART_FROM_ESP32 Serial1
#define UART_FROM_ESP32 Serial1

// 中文逐行说明：下面这一行保留原始代码 -> MotionController motion;
MotionController motion;
// 中文逐行说明：下面这一行保留原始代码 -> ProtocolReceiver protocolReceiver;
ProtocolReceiver protocolReceiver;

// 中文逐行说明：下面这一行保留原始代码 -> unsigned long tStatus = 0;
unsigned long tStatus = 0;
// 中文逐行说明：下面这一行保留原始代码 -> const unsigned long STATUS_INTERVAL = 500;
const unsigned long STATUS_INTERVAL = 500;

// 鍙戦€佺姸鎬佸抚鍒?ESP32銆?static void sendStatusToESP32(uint8_t cmd, uint8_t data0 = 0, uint8_t data1 = 0, uint8_t data2 = 0) {
    // 中文逐行说明：下面这一行保留原始代码 -> sendFrame(UART_FROM_ESP32, cmd, data0, data1, data2);
    sendFrame(UART_FROM_ESP32, cmd, data0, data1, data2);
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 鍘嬬缉褰撳墠鎵ц鐘舵€佷负涓€涓姸鎬佸瓧鑺傘€?static uint8_t getMotionStatus() {
    // 中文逐行说明：下面这一行保留原始代码 -> uint8_t status = 0;
    uint8_t status = 0;

    // 中文逐行说明：下面这一行保留原始代码 -> if (motion.isForwardActive()) {
    if (motion.isForwardActive()) {
        // 中文逐行说明：下面这一行保留原始代码 -> status |= 0x01;
        status |= 0x01;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (motion.isTurnActive()) {
    if (motion.isTurnActive()) {
        // 中文逐行说明：下面这一行保留原始代码 -> status |= 0x02;
        status |= 0x02;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (motion.isBuoyancyActive()) {
    if (motion.isBuoyancyActive()) {
        // 中文逐行说明：下面这一行保留原始代码 -> status |= 0x04;
        status |= 0x04;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> return status;
    return status;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 鎵ц鏉ヨ嚜 ESP32 鐨勫崗璁懡浠ゃ€?static void executeCommand(uint8_t cmd, uint8_t data0, uint8_t data1, uint8_t data2) {
    // 中文逐行说明：下面这一行保留原始代码 -> (void)data2;
    (void)data2;

    // 中文逐行说明：下面这一行保留原始代码 -> switch (cmd) {
    switch (cmd) {
        // 中文逐行说明：下面这一行保留原始代码 -> case CMD_SET_ACTUATORS: {
        case CMD_SET_ACTUATORS: {
            // 中文逐行说明：下面这一行保留原始代码 -> const uint16_t mask =
            const uint16_t mask =
                // 中文逐行说明：下面这一行保留原始代码 -> static_cast<uint16_t>(data0) |
                static_cast<uint16_t>(data0) |
                // 中文逐行说明：下面这一行保留原始代码 -> (static_cast<uint16_t>(data1) << 8);
                (static_cast<uint16_t>(data1) << 8);
            // 中文逐行说明：下面这一行保留原始代码 -> motion.applyMask(mask);
            motion.applyMask(mask);
            // 中文逐行说明：下面这一行保留原始代码 -> break;
            break;
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }

        // 中文逐行说明：下面这一行保留原始代码 -> case CMD_EMERGENCY_STOP:
        case CMD_EMERGENCY_STOP:
            // 中文逐行说明：下面这一行保留原始代码 -> motion.emergencyStopAll();
            motion.emergencyStopAll();
            // 中文逐行说明：下面这一行保留原始代码 -> break;
            break;

        // 中文逐行说明：下面这一行保留原始代码 -> default:
        default:
            // 中文逐行说明：下面这一行保留原始代码 -> Serial.print(F("Unknown command: 0x"));
            Serial.print(F("Unknown command: 0x"));
            // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(cmd, HEX);
            Serial.println(cmd, HEX);
            // 中文逐行说明：下面这一行保留原始代码 -> break;
            break;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 杞 ESP32 涓插彛骞舵墽琛屽懡浠ゃ€?static void processESP32Command() {
    // 中文逐行说明：下面这一行保留原始代码 -> ProtocolFrame frame;
    ProtocolFrame frame;
    // 中文逐行说明：下面这一行保留原始代码 -> while (protocolReceiver.poll(UART_FROM_ESP32, frame)) {
    while (protocolReceiver.poll(UART_FROM_ESP32, frame)) {
        // 中文逐行说明：下面这一行保留原始代码 -> executeCommand(frame.cmd, frame.data0, frame.data1, frame.data2);
        executeCommand(frame.cmd, frame.data0, frame.data1, frame.data2);
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void setup() {
void setup() {
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.begin(115200);
    Serial.begin(115200);
    // 中文逐行说明：下面这一行保留原始代码 -> delay(500);
    delay(500);

    // 中文逐行说明：下面这一行保留原始代码 -> UART_FROM_ESP32.begin(UART_BAUD_RATE);
    UART_FROM_ESP32.begin(UART_BAUD_RATE);

    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("\n============================================"));
    Serial.println(F("\n============================================"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("  Arduino Minima Actuator Executor"));
    Serial.println(F("  Arduino Minima Actuator Executor"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("============================================"));
    Serial.println(F("============================================"));

    // 中文逐行说明：下面这一行保留原始代码 -> motion.begin();
    motion.begin();
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("Actuator executor ready.\n"));
    Serial.println(F("Actuator executor ready.\n"));
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void loop() {
void loop() {
    // 中文逐行说明：下面这一行保留原始代码 -> processESP32Command();
    processESP32Command();

    // 中文逐行说明：下面这一行保留原始代码 -> if (millis() - tStatus >= STATUS_INTERVAL) {
    if (millis() - tStatus >= STATUS_INTERVAL) {
        // 中文逐行说明：下面这一行保留原始代码 -> tStatus = millis();
        tStatus = millis();
        // 中文逐行说明：下面这一行保留原始代码 -> sendStatusToESP32(STATUS_MOTION, getMotionStatus(), 0, 0);
        sendStatusToESP32(STATUS_MOTION, getMotionStatus(), 0, 0);
        // 中文逐行说明：下面这一行保留原始代码 -> motion.printStatus();
        motion.printStatus();
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
// 中文逐行说明：下面这一行保留原始代码 -> }
}
