/**********************************************************************
 * MotionControl.cpp
 *
 * Low-level Minima actuator executor with software-PWM buoyancy output.
 *********************************************************************/

#include "MotionControl.h"

namespace {
constexpr uint32_t VALVE_MIN_INTERVAL_MS = 200;
constexpr uint32_t BUOYANCY_PWM_PERIOD_US = 20000;
}

MotionController::MotionController()
    : _mask(0),
      _requestedBuoyancyDirection(BUOYANCY_STOP),
      _requestedBuoyancyPwm(0),
      _appliedBuoyancyDirection(BUOYANCY_STOP),
      _appliedBuoyancyPwm(0),
      _lastValveChangeMs(0) {}

void MotionController::begin() {
    pinMode(PUMP_A_PIN, OUTPUT);
    pinMode(VALVE_B_PIN, OUTPUT);
    pinMode(VALVE_C_PIN, OUTPUT);
    pinMode(PUMP_D_PIN, OUTPUT);
    pinMode(VALVE_E_PIN, OUTPUT);
    pinMode(VALVE_F_PIN, OUTPUT);
    pinMode(PUMP_G_PIN, OUTPUT);
    pinMode(VALVE_H_PIN, OUTPUT);
    pinMode(VALVE_I_PIN, OUTPUT);

    emergencyStopAll();
}

void MotionController::applyMask(uint16_t mask) {
    _mask = mask;
    writeOutputs();
}

void MotionController::applyBuoyancy(uint8_t direction, uint8_t pwm) {
    if (direction == BUOYANCY_STOP || pwm == 0) {
        _requestedBuoyancyDirection = BUOYANCY_STOP;
        _requestedBuoyancyPwm = 0;
        return;
    }

    _requestedBuoyancyDirection = direction;
    _requestedBuoyancyPwm = pwm;
}

void MotionController::update() {
    writeOutputs();
}

void MotionController::emergencyStopAll() {
    _mask = 0;
    _requestedBuoyancyDirection = BUOYANCY_STOP;
    _requestedBuoyancyPwm = 0;
    _appliedBuoyancyDirection = BUOYANCY_STOP;
    _appliedBuoyancyPwm = 0;
    writeOutputs();
}

uint16_t MotionController::getMask() const {
    return _mask;
}

bool MotionController::isForwardActive() const {
    return (_mask & ACT_FORWARD_GROUP) != 0;
}

bool MotionController::isTurnActive() const {
    return (_mask & ACT_TURN_GROUP) != 0;
}

bool MotionController::isBuoyancyActive() const {
    return _appliedBuoyancyPwm > 0;
}

bool MotionController::isAnyActive() const {
    return _mask != 0 || _appliedBuoyancyPwm > 0;
}

uint8_t MotionController::getBuoyancyDirection() const {
    return _appliedBuoyancyDirection;
}

uint8_t MotionController::getBuoyancyPwm() const {
    return _appliedBuoyancyPwm;
}

void MotionController::printStatus() {
    Serial.print(F("Mask=0x"));
    Serial.print(_mask, HEX);
    Serial.print(F(" | "));

    if (isForwardActive()) {
        Serial.print(F("FWD "));
    }

    if (isTurnActive()) {
        Serial.print(F("TURN "));
    }

    if (isBuoyancyActive()) {
        Serial.print(F("BUOY("));
        Serial.print(_appliedBuoyancyDirection == BUOYANCY_DESCEND ? F("DESC") : F("ASC"));
        Serial.print(',');
        Serial.print(_appliedBuoyancyPwm);
        Serial.print(F(") "));
    }

    if (!isAnyActive()) {
        Serial.print(F("IDLE"));
    }

    Serial.println();
}

void MotionController::writeOutputs() {
    digitalWrite(PUMP_A_PIN,   (_mask & ACT_FORWARD_PUMP) != 0 ? HIGH : LOW);
    digitalWrite(VALVE_B_PIN,  (_mask & ACT_FORWARD_VALVE_B) != 0 ? HIGH : LOW);
    digitalWrite(VALVE_C_PIN,  (_mask & ACT_FORWARD_VALVE_C) != 0 ? HIGH : LOW);
    digitalWrite(PUMP_D_PIN,   (_mask & ACT_TURN_PUMP) != 0 ? HIGH : LOW);
    digitalWrite(VALVE_E_PIN,  (_mask & ACT_TURN_VALVE_E) != 0 ? HIGH : LOW);
    digitalWrite(VALVE_F_PIN,  (_mask & ACT_TURN_VALVE_F) != 0 ? HIGH : LOW);
    writeBuoyancyOutputs(millis());
}

void MotionController::writeBuoyancyOutputs(uint32_t nowMs) {
    if (_requestedBuoyancyDirection == BUOYANCY_STOP || _requestedBuoyancyPwm == 0) {
        _appliedBuoyancyPwm = 0;
        _appliedBuoyancyDirection = BUOYANCY_STOP;
        digitalWrite(PUMP_G_PIN, LOW);
        digitalWrite(VALVE_H_PIN, LOW);
        digitalWrite(VALVE_I_PIN, LOW);
        return;
    }

    if (_requestedBuoyancyDirection != _appliedBuoyancyDirection) {
        if (nowMs - _lastValveChangeMs >= VALVE_MIN_INTERVAL_MS) {
            _appliedBuoyancyDirection = _requestedBuoyancyDirection;
            _lastValveChangeMs = nowMs;
        } else {
            _appliedBuoyancyPwm = 0;
            digitalWrite(PUMP_G_PIN, LOW);
            return;
        }
    }

    _appliedBuoyancyPwm = _requestedBuoyancyPwm;

    const bool descend = _appliedBuoyancyDirection == BUOYANCY_DESCEND;
    digitalWrite(VALVE_H_PIN, descend ? HIGH : LOW);
    digitalWrite(VALVE_I_PIN, descend ? HIGH : LOW);

    const uint32_t dutyUs =
        (static_cast<uint32_t>(_appliedBuoyancyPwm) * BUOYANCY_PWM_PERIOD_US) / 255U;
    const bool pumpOn = dutyUs > 0 && (micros() % BUOYANCY_PWM_PERIOD_US) < dutyUs;
    digitalWrite(PUMP_G_PIN, pumpOn ? HIGH : LOW);
}
