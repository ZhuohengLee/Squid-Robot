/**********************************************************************
 * MotionControl.cpp
 *
 * 杩欎釜鏂囦欢瀹炵幇 Minima 渚х殑浣庡眰鎵ц鍣ㄦ帶鍒躲€? * Minima 鍙墽琛?ESP32 鍙戞潵鐨勪綅鎺╃爜锛屼笉淇濆瓨浠讳綍鍔ㄤ綔鎸佺画鏃堕棿銆? *********************************************************************/

// 中文逐行说明：下面这一行保留原始代码 -> #include "MotionControl.h"
#include "MotionControl.h"

// 中文逐行说明：下面这一行保留原始代码 -> MotionController::MotionController()
MotionController::MotionController()
    // 中文逐行说明：下面这一行保留原始代码 -> : _mask(0) {}
    : _mask(0) {}

// 中文逐行说明：下面这一行保留原始代码 -> void MotionController::begin() {
void MotionController::begin() {
    // 中文逐行说明：下面这一行保留原始代码 -> pinMode(PUMP_A_PIN, OUTPUT);
    pinMode(PUMP_A_PIN, OUTPUT);
    // 中文逐行说明：下面这一行保留原始代码 -> pinMode(VALVE_B_PIN, OUTPUT);
    pinMode(VALVE_B_PIN, OUTPUT);
    // 中文逐行说明：下面这一行保留原始代码 -> pinMode(VALVE_C_PIN, OUTPUT);
    pinMode(VALVE_C_PIN, OUTPUT);
    // 中文逐行说明：下面这一行保留原始代码 -> pinMode(PUMP_D_PIN, OUTPUT);
    pinMode(PUMP_D_PIN, OUTPUT);
    // 中文逐行说明：下面这一行保留原始代码 -> pinMode(VALVE_E_PIN, OUTPUT);
    pinMode(VALVE_E_PIN, OUTPUT);
    // 中文逐行说明：下面这一行保留原始代码 -> pinMode(VALVE_F_PIN, OUTPUT);
    pinMode(VALVE_F_PIN, OUTPUT);
    // 中文逐行说明：下面这一行保留原始代码 -> pinMode(PUMP_G_PIN, OUTPUT);
    pinMode(PUMP_G_PIN, OUTPUT);
    // 中文逐行说明：下面这一行保留原始代码 -> pinMode(VALVE_H_PIN, OUTPUT);
    pinMode(VALVE_H_PIN, OUTPUT);
    // 中文逐行说明：下面这一行保留原始代码 -> pinMode(VALVE_I_PIN, OUTPUT);
    pinMode(VALVE_I_PIN, OUTPUT);

    // 中文逐行说明：下面这一行保留原始代码 -> emergencyStopAll();
    emergencyStopAll();
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void MotionController::applyMask(uint16_t mask) {
void MotionController::applyMask(uint16_t mask) {
    // 中文逐行说明：下面这一行保留原始代码 -> _mask = mask;
    _mask = mask;
    // 中文逐行说明：下面这一行保留原始代码 -> writeOutputs();
    writeOutputs();
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void MotionController::emergencyStopAll() {
void MotionController::emergencyStopAll() {
    // 中文逐行说明：下面这一行保留原始代码 -> _mask = 0;
    _mask = 0;
    // 中文逐行说明：下面这一行保留原始代码 -> writeOutputs();
    writeOutputs();
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> uint16_t MotionController::getMask() const {
uint16_t MotionController::getMask() const {
    // 中文逐行说明：下面这一行保留原始代码 -> return _mask;
    return _mask;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> bool MotionController::isForwardActive() const {
bool MotionController::isForwardActive() const {
    // 中文逐行说明：下面这一行保留原始代码 -> return (_mask & ACT_FORWARD_GROUP) != 0;
    return (_mask & ACT_FORWARD_GROUP) != 0;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> bool MotionController::isTurnActive() const {
bool MotionController::isTurnActive() const {
    // 中文逐行说明：下面这一行保留原始代码 -> return (_mask & ACT_TURN_GROUP) != 0;
    return (_mask & ACT_TURN_GROUP) != 0;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> bool MotionController::isBuoyancyActive() const {
bool MotionController::isBuoyancyActive() const {
    // 中文逐行说明：下面这一行保留原始代码 -> return (_mask & ACT_BUOYANCY_GROUP) != 0;
    return (_mask & ACT_BUOYANCY_GROUP) != 0;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> bool MotionController::isAnyActive() const {
bool MotionController::isAnyActive() const {
    // 中文逐行说明：下面这一行保留原始代码 -> return _mask != 0;
    return _mask != 0;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void MotionController::printStatus() {
void MotionController::printStatus() {
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.print(F("Mask=0x"));
    Serial.print(F("Mask=0x"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.print(_mask, HEX);
    Serial.print(_mask, HEX);
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.print(F(" | "));
    Serial.print(F(" | "));

    // 中文逐行说明：下面这一行保留原始代码 -> if (isForwardActive()) {
    if (isForwardActive()) {
        // 中文逐行说明：下面这一行保留原始代码 -> Serial.print(F("FWD "));
        Serial.print(F("FWD "));
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (isTurnActive()) {
    if (isTurnActive()) {
        // 中文逐行说明：下面这一行保留原始代码 -> Serial.print(F("TURN "));
        Serial.print(F("TURN "));
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (isBuoyancyActive()) {
    if (isBuoyancyActive()) {
        // 中文逐行说明：下面这一行保留原始代码 -> Serial.print(F("BUOY "));
        Serial.print(F("BUOY "));
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (!isAnyActive()) {
    if (!isAnyActive()) {
        // 中文逐行说明：下面这一行保留原始代码 -> Serial.print(F("IDLE"));
        Serial.print(F("IDLE"));
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println();
    Serial.println();
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void MotionController::writeOutputs() {
void MotionController::writeOutputs() {
    // 中文逐行说明：下面这一行保留原始代码 -> digitalWrite(PUMP_A_PIN,   (_mask & ACT_FORWARD_PUMP) != 0 ? HIGH : LOW);
    digitalWrite(PUMP_A_PIN,   (_mask & ACT_FORWARD_PUMP) != 0 ? HIGH : LOW);
    // 中文逐行说明：下面这一行保留原始代码 -> digitalWrite(VALVE_B_PIN,  (_mask & ACT_FORWARD_VALVE_B) != 0 ? HIGH : LOW);
    digitalWrite(VALVE_B_PIN,  (_mask & ACT_FORWARD_VALVE_B) != 0 ? HIGH : LOW);
    // 中文逐行说明：下面这一行保留原始代码 -> digitalWrite(VALVE_C_PIN,  (_mask & ACT_FORWARD_VALVE_C) != 0 ? HIGH : LOW);
    digitalWrite(VALVE_C_PIN,  (_mask & ACT_FORWARD_VALVE_C) != 0 ? HIGH : LOW);
    // 中文逐行说明：下面这一行保留原始代码 -> digitalWrite(PUMP_D_PIN,   (_mask & ACT_TURN_PUMP) != 0 ? HIGH : LOW);
    digitalWrite(PUMP_D_PIN,   (_mask & ACT_TURN_PUMP) != 0 ? HIGH : LOW);
    // 中文逐行说明：下面这一行保留原始代码 -> digitalWrite(VALVE_E_PIN,  (_mask & ACT_TURN_VALVE_E) != 0 ? HIGH : LOW);
    digitalWrite(VALVE_E_PIN,  (_mask & ACT_TURN_VALVE_E) != 0 ? HIGH : LOW);
    // 中文逐行说明：下面这一行保留原始代码 -> digitalWrite(VALVE_F_PIN,  (_mask & ACT_TURN_VALVE_F) != 0 ? HIGH : LOW);
    digitalWrite(VALVE_F_PIN,  (_mask & ACT_TURN_VALVE_F) != 0 ? HIGH : LOW);
    // 中文逐行说明：下面这一行保留原始代码 -> digitalWrite(PUMP_G_PIN,   (_mask & ACT_BUOYANCY_PUMP) != 0 ? HIGH : LOW);
    digitalWrite(PUMP_G_PIN,   (_mask & ACT_BUOYANCY_PUMP) != 0 ? HIGH : LOW);
    // 中文逐行说明：下面这一行保留原始代码 -> digitalWrite(VALVE_H_PIN,  (_mask & ACT_BUOYANCY_VALVE_H) != 0 ? HIGH : LOW);
    digitalWrite(VALVE_H_PIN,  (_mask & ACT_BUOYANCY_VALVE_H) != 0 ? HIGH : LOW);
    // 中文逐行说明：下面这一行保留原始代码 -> digitalWrite(VALVE_I_PIN,  (_mask & ACT_BUOYANCY_VALVE_I) != 0 ? HIGH : LOW);
    digitalWrite(VALVE_I_PIN,  (_mask & ACT_BUOYANCY_VALVE_I) != 0 ? HIGH : LOW);
// 中文逐行说明：下面这一行保留原始代码 -> }
}
