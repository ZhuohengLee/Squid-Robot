/**********************************************************************
 * MotionLink.cpp
 *
 * 这个文件实现 ESP32 到 Minima 的运动命令发送接口。
 *********************************************************************/

#include "MotionLink.h"

void MotionLink::begin() {
    UART_TO_MINIMA.begin(UART_BAUD_RATE, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
}

void MotionLink::sendCommand(uint8_t cmd, uint8_t data0, uint8_t data1, uint8_t data2) {
    uint8_t frame[FRAME_LENGTH];
    frame[0] = FRAME_HEADER;
    frame[1] = FRAME_LENGTH;
    frame[2] = cmd;
    frame[3] = data0;
    frame[4] = data1;
    frame[5] = data2;
    frame[6] = calculateCRC8(&frame[2], 4);
    frame[7] = FRAME_TAIL;
    UART_TO_MINIMA.write(frame, FRAME_LENGTH);
}

void MotionLink::startForward() {
    sendCommand(CMD_FORWARD_START);
}

void MotionLink::stopForward() {
    sendCommand(CMD_FORWARD_STOP);
}

void MotionLink::turnLeft() {
    sendCommand(CMD_TURN_LEFT);
}

void MotionLink::turnRight() {
    sendCommand(CMD_TURN_RIGHT);
}

void MotionLink::stopTurn() {
    sendCommand(CMD_TURN_STOP);
}

void MotionLink::ascend() {
    sendCommand(CMD_ASCEND);
}

void MotionLink::descend() {
    sendCommand(CMD_DESCEND);
}

void MotionLink::stopBuoyancy() {
    sendCommand(CMD_BUOYANCY_STOP);
}

void MotionLink::emergencyStop() {
    sendCommand(CMD_EMERGENCY_STOP);
}
