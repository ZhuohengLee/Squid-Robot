/**********************************************************************
 * MotionLink.cpp
 *
 * Serial transport for Minima actuator and buoyancy commands.
 *********************************************************************/

#include "MotionLink.h"

MotionLink::MotionLink()
    : _lastMask(0),
      _lastBuoyancyDirection(BUOYANCY_STOP),
      _lastBuoyancyPwm(0) {}

void MotionLink::begin() {
    UART_TO_MINIMA.begin(UART_BAUD_RATE, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
}

void MotionLink::applyMask(uint16_t actuatorMask, bool forceSend) {
    if (!forceSend && actuatorMask == _lastMask) {
        return;
    }

    sendCommand(
        CMD_SET_ACTUATORS,
        static_cast<uint8_t>(actuatorMask & 0xFF),
        static_cast<uint8_t>((actuatorMask >> 8) & 0xFF),
        0
    );
    _lastMask = actuatorMask;
}

void MotionLink::applyBuoyancy(uint8_t direction, uint8_t pwm, bool forceSend) {
    if (!forceSend &&
        direction == _lastBuoyancyDirection &&
        pwm == _lastBuoyancyPwm) {
        return;
    }

    sendCommand(CMD_SET_BUOYANCY, direction, pwm, 0);
    _lastBuoyancyDirection = direction;
    _lastBuoyancyPwm = pwm;
}

void MotionLink::emergencyStop() {
    sendCommand(CMD_EMERGENCY_STOP);
    _lastMask = 0;
    _lastBuoyancyDirection = BUOYANCY_STOP;
    _lastBuoyancyPwm = 0;
}

void MotionLink::requestDepthZeroCalibration() {
    sendCommand(CMD_CALIBRATE_DEPTH_ZERO);
}

uint16_t MotionLink::getLastMask() const {
    return _lastMask;
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
