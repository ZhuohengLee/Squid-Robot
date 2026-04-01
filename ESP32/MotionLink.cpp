/**********************************************************************
 * MotionLink.cpp
 *
 * Serial transport for Minima actuator and buoyancy commands.
 *********************************************************************/

#include "MotionLink.h"

namespace {
constexpr uint32_t COMMAND_REFRESH_MS = 250;
}

MotionLink::MotionLink()
    : _lastMask(0),
      _lastBuoyancyDirection(BUOYANCY_STOP),
      _lastBuoyancyPwm(0),
      _lastMaskSendMs(0),
      _lastBuoyancySendMs(0) {}

void MotionLink::begin() {
    UART_TO_MINIMA.begin(UART_BAUD_RATE, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
    _lastMaskSendMs = 0;
    _lastBuoyancySendMs = 0;
}

void MotionLink::applyMask(uint16_t actuatorMask, bool forceSend) {
    const uint32_t nowMs = millis();
    const bool active = actuatorMask != 0;
    const bool changed = actuatorMask != _lastMask;
    const bool refreshDue =
        _lastMaskSendMs == 0 || (nowMs - _lastMaskSendMs) >= COMMAND_REFRESH_MS;

    if (!forceSend && !changed && (!active || !refreshDue)) {
        return;
    }

    sendCommand(
        CMD_SET_ACTUATORS,
        static_cast<uint8_t>(actuatorMask & 0xFF),
        static_cast<uint8_t>((actuatorMask >> 8) & 0xFF),
        0
    );
    _lastMask = actuatorMask;
    _lastMaskSendMs = nowMs;
}

void MotionLink::applyBuoyancy(uint8_t direction, uint8_t pwm, bool forceSend) {
    const uint32_t nowMs = millis();
    const bool active =
        direction != BUOYANCY_STOP && pwm > 0;
    const bool changed =
        direction != _lastBuoyancyDirection || pwm != _lastBuoyancyPwm;
    const bool refreshDue =
        _lastBuoyancySendMs == 0 ||
        (nowMs - _lastBuoyancySendMs) >= COMMAND_REFRESH_MS;

    if (!forceSend && !changed && (!active || !refreshDue)) {
        return;
    }

    sendCommand(CMD_SET_BUOYANCY, direction, pwm, 0);
    _lastBuoyancyDirection = direction;
    _lastBuoyancyPwm = pwm;
    _lastBuoyancySendMs = nowMs;
}

void MotionLink::emergencyStop() {
    sendCommand(CMD_EMERGENCY_STOP);
    _lastMask = 0;
    _lastBuoyancyDirection = BUOYANCY_STOP;
    _lastBuoyancyPwm = 0;
    _lastMaskSendMs = millis();
    _lastBuoyancySendMs = _lastMaskSendMs;
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
