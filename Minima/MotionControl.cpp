/**********************************************************************
 * MotionControl.cpp
 *
 * 这个文件实现 Minima 侧的低层执行器控制。
 * Minima 只执行 ESP32 发来的位掩码，不保存任何动作持续时间。
 *********************************************************************/

#include "MotionControl.h"

MotionController::MotionController()
    : _mask(0) {}

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

void MotionController::emergencyStopAll() {
    _mask = 0;
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
    return (_mask & ACT_BUOYANCY_GROUP) != 0;
}

bool MotionController::isAnyActive() const {
    return _mask != 0;
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
        Serial.print(F("BUOY "));
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
    digitalWrite(PUMP_G_PIN,   (_mask & ACT_BUOYANCY_PUMP) != 0 ? HIGH : LOW);
    digitalWrite(VALVE_H_PIN,  (_mask & ACT_BUOYANCY_VALVE_H) != 0 ? HIGH : LOW);
    digitalWrite(VALVE_I_PIN,  (_mask & ACT_BUOYANCY_VALVE_I) != 0 ? HIGH : LOW);
}
