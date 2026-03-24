/**********************************************************************
 * MotionLink.cpp
 *
 * 杩欎釜鏂囦欢瀹炵幇 ESP32 鍒?Minima 鐨勬墽琛屽櫒浣嶆帺鐮佸彂閫併€? *********************************************************************/

// 中文逐行说明：下面这一行保留原始代码 -> #include "MotionLink.h"
#include "MotionLink.h"

// 中文逐行说明：下面这一行保留原始代码 -> MotionLink::MotionLink()
MotionLink::MotionLink()
    // 中文逐行说明：下面这一行保留原始代码 -> : _lastMask(0) {}
    : _lastMask(0) {}

// 中文逐行说明：下面这一行保留原始代码 -> void MotionLink::begin() {
void MotionLink::begin() {
    // 中文逐行说明：下面这一行保留原始代码 -> UART_TO_MINIMA.begin(UART_BAUD_RATE, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
    UART_TO_MINIMA.begin(UART_BAUD_RATE, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void MotionLink::applyMask(uint16_t actuatorMask, bool forceSend) {
void MotionLink::applyMask(uint16_t actuatorMask, bool forceSend) {
    // 中文逐行说明：下面这一行保留原始代码 -> if (!forceSend && actuatorMask == _lastMask) {
    if (!forceSend && actuatorMask == _lastMask) {
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> sendCommand(
    sendCommand(
        // 中文逐行说明：下面这一行保留原始代码 -> CMD_SET_ACTUATORS,
        CMD_SET_ACTUATORS,
        // 中文逐行说明：下面这一行保留原始代码 -> static_cast<uint8_t>(actuatorMask & 0xFF),
        static_cast<uint8_t>(actuatorMask & 0xFF),
        // 中文逐行说明：下面这一行保留原始代码 -> static_cast<uint8_t>((actuatorMask >> 8) & 0xFF),
        static_cast<uint8_t>((actuatorMask >> 8) & 0xFF),
        // 中文逐行说明：下面这一行保留原始代码 -> 0
        0
    // 中文逐行说明：下面这一行保留原始代码 -> );
    );
    // 中文逐行说明：下面这一行保留原始代码 -> _lastMask = actuatorMask;
    _lastMask = actuatorMask;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void MotionLink::emergencyStop() {
void MotionLink::emergencyStop() {
    // 中文逐行说明：下面这一行保留原始代码 -> sendCommand(CMD_EMERGENCY_STOP);
    sendCommand(CMD_EMERGENCY_STOP);
    // 中文逐行说明：下面这一行保留原始代码 -> _lastMask = 0;
    _lastMask = 0;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> uint16_t MotionLink::getLastMask() const {
uint16_t MotionLink::getLastMask() const {
    // 中文逐行说明：下面这一行保留原始代码 -> return _lastMask;
    return _lastMask;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void MotionLink::sendCommand(uint8_t cmd, uint8_t data0, uint8_t data1, uint8_t data2) {
void MotionLink::sendCommand(uint8_t cmd, uint8_t data0, uint8_t data1, uint8_t data2) {
    // 中文逐行说明：下面这一行保留原始代码 -> uint8_t frame[FRAME_LENGTH];
    uint8_t frame[FRAME_LENGTH];
    // 中文逐行说明：下面这一行保留原始代码 -> frame[0] = FRAME_HEADER;
    frame[0] = FRAME_HEADER;
    // 中文逐行说明：下面这一行保留原始代码 -> frame[1] = FRAME_LENGTH;
    frame[1] = FRAME_LENGTH;
    // 中文逐行说明：下面这一行保留原始代码 -> frame[2] = cmd;
    frame[2] = cmd;
    // 中文逐行说明：下面这一行保留原始代码 -> frame[3] = data0;
    frame[3] = data0;
    // 中文逐行说明：下面这一行保留原始代码 -> frame[4] = data1;
    frame[4] = data1;
    // 中文逐行说明：下面这一行保留原始代码 -> frame[5] = data2;
    frame[5] = data2;
    // 中文逐行说明：下面这一行保留原始代码 -> frame[6] = calculateCRC8(&frame[2], 4);
    frame[6] = calculateCRC8(&frame[2], 4);
    // 中文逐行说明：下面这一行保留原始代码 -> frame[7] = FRAME_TAIL;
    frame[7] = FRAME_TAIL;
    // 中文逐行说明：下面这一行保留原始代码 -> UART_TO_MINIMA.write(frame, FRAME_LENGTH);
    UART_TO_MINIMA.write(frame, FRAME_LENGTH);
// 中文逐行说明：下面这一行保留原始代码 -> }
}
