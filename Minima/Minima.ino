/**********************************************************************
 * Minima.ino
 *
 * 这是 Arduino Minima 执行器草图入口。
 * Minima 只做三件事：
 * 1. 接收 ESP32 下发的执行器掩码；
 * 2. 把掩码直接写到真实引脚；
 * 3. 回传当前前进/转向/浮沉状态。
 *********************************************************************/

#include <Arduino.h>
#include "MotionControl.h"
#include "Protocol.h"

#define UART_FROM_ESP32 Serial1

MotionController motion;
ProtocolReceiver protocolReceiver;

unsigned long tStatus = 0;
const unsigned long STATUS_INTERVAL = 500;

// 发送状态帧到 ESP32。
static void sendStatusToESP32(uint8_t cmd, uint8_t data0 = 0, uint8_t data1 = 0, uint8_t data2 = 0) {
    sendFrame(UART_FROM_ESP32, cmd, data0, data1, data2);
}

// 压缩当前执行状态为一个状态字节。
static uint8_t getMotionStatus() {
    uint8_t status = 0;

    if (motion.isForwardActive()) {
        status |= 0x01;
    }

    if (motion.isTurnActive()) {
        status |= 0x02;
    }

    if (motion.isBuoyancyActive()) {
        status |= 0x04;
    }

    return status;
}

// 执行来自 ESP32 的协议命令。
static void executeCommand(uint8_t cmd, uint8_t data0, uint8_t data1, uint8_t data2) {
    (void)data2;

    switch (cmd) {
        case CMD_SET_ACTUATORS: {
            const uint16_t mask =
                static_cast<uint16_t>(data0) |
                (static_cast<uint16_t>(data1) << 8);
            motion.applyMask(mask);
            break;
        }

        case CMD_EMERGENCY_STOP:
            motion.emergencyStopAll();
            break;

        default:
            Serial.print(F("Unknown command: 0x"));
            Serial.println(cmd, HEX);
            break;
    }
}

// 轮询 ESP32 串口并执行命令。
static void processESP32Command() {
    ProtocolFrame frame;
    while (protocolReceiver.poll(UART_FROM_ESP32, frame)) {
        executeCommand(frame.cmd, frame.data0, frame.data1, frame.data2);
    }
}

void setup() {
    Serial.begin(115200);
    delay(500);

    UART_FROM_ESP32.begin(UART_BAUD_RATE);

    Serial.println(F("\n============================================"));
    Serial.println(F("  Arduino Minima Actuator Executor"));
    Serial.println(F("============================================"));

    motion.begin();
    Serial.println(F("Actuator executor ready.\n"));
}

void loop() {
    processESP32Command();

    if (millis() - tStatus >= STATUS_INTERVAL) {
        tStatus = millis();
        sendStatusToESP32(STATUS_MOTION, getMotionStatus(), 0, 0);
        motion.printStatus();
    }
}
