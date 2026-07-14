/**********************************************************************
 * MotionControl.cpp
 *
 * Minima 底层执行器驱动。
 *
 * applyMask()      — 接收 ESP32 下发的执行器掩码，立即写引脚。
 * applyBuoyancy()  — 接收浮沉方向和 PWM，存入请求，由 update() 执行。
 * update()         — 每帧调用，仅处理浮沉软件 PWM 输出（气泵 + 阀门）。
 *                    前进/转向引脚由 applyMask() 负责，无需重复写。
 *********************************************************************/

#include "MotionControl.h"

namespace {
constexpr uint32_t VALVE_MIN_INTERVAL_MS  = 200;   // 浮力阀换向防抖延迟
constexpr uint32_t BUOYANCY_PWM_PERIOD_US = 20000; // 浮力泵软件 PWM 周期
}

MotionController::MotionController()
    : _mask(0),
      _requestedBuoyancyDirection(BUOYANCY_STOP),
      _requestedBuoyancyPwm(0),
      _appliedBuoyancyDirection(BUOYANCY_STOP),
      _appliedBuoyancyPwm(0),
      _lastValveChangeMs(0) {}

void MotionController::begin() {
    pinMode(PUMP_A_PIN,  OUTPUT);
    pinMode(VALVE_A_PIN, OUTPUT);
    pinMode(VALVE_B_PIN, OUTPUT);
    pinMode(PUMP_D_PIN,  OUTPUT);
    pinMode(VALVE_C_PIN, OUTPUT);
    pinMode(VALVE_D_PIN, OUTPUT);
    pinMode(PUMP_G_PIN,  OUTPUT);
    pinMode(VALVE_E_PIN, OUTPUT);
    pinMode(VALVE_F_PIN, OUTPUT);

    emergencyStopAll();
}

void MotionController::applyMask(uint16_t mask) {
    _mask = mask;
    writeOutputs();
}

void MotionController::applyBuoyancy(uint8_t direction, uint8_t pwm) {
    if (direction == BUOYANCY_STOP || (pwm == 0 && direction != BUOYANCY_BALANCE)) {
        _requestedBuoyancyDirection = BUOYANCY_STOP;
        _requestedBuoyancyPwm       = 0;
        return;
    }
    _requestedBuoyancyDirection = direction;
    _requestedBuoyancyPwm       = pwm;
}

void MotionController::update() {
    // 前进/转向引脚由 applyMask() 在收到帧时立即写入，此处只处理浮沉 PWM。
    writeBuoyancyOutputs(millis());
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
    // 前进子系统：泵 + 阀 a/b
    digitalWrite(PUMP_A_PIN,  (_mask & ACT_FORWARD_PUMP)    != 0 ? HIGH : LOW);
    digitalWrite(VALVE_A_PIN, (_mask & ACT_FORWARD_VALVE_A) != 0 ? HIGH : LOW);
    digitalWrite(VALVE_B_PIN, (_mask & ACT_FORWARD_VALVE_B) != 0 ? HIGH : LOW);

    // 转向子系统：泵 + 阀 c/d
    digitalWrite(PUMP_D_PIN,  (_mask & ACT_TURN_PUMP)       != 0 ? HIGH : LOW);
    digitalWrite(VALVE_C_PIN, (_mask & ACT_TURN_VALVE_C)    != 0 ? HIGH : LOW);
    digitalWrite(VALVE_D_PIN, (_mask & ACT_TURN_VALVE_D)    != 0 ? HIGH : LOW);

    // 浮力子系统由 writeBuoyancyOutputs() 单独处理。
    writeBuoyancyOutputs(millis());
}

void MotionController::writeBuoyancyOutputs(uint32_t nowMs) {
    if (_requestedBuoyancyDirection == BUOYANCY_STOP || _requestedBuoyancyPwm == 0) {
        _appliedBuoyancyPwm = 0;
        _appliedBuoyancyDirection = BUOYANCY_STOP;
        digitalWrite(PUMP_G_PIN,  LOW);
        digitalWrite(VALVE_E_PIN, LOW);
        digitalWrite(VALVE_F_PIN, LOW);
        return;
    }

    // 气压平衡模式：E/F 同时通电，泵关闭。
    if (_requestedBuoyancyDirection == BUOYANCY_BALANCE) {
        _appliedBuoyancyDirection = BUOYANCY_BALANCE;
        _appliedBuoyancyPwm = 0;
        digitalWrite(PUMP_G_PIN,  LOW);
        digitalWrite(VALVE_E_PIN, HIGH);
        digitalWrite(VALVE_F_PIN, HIGH);
        return;
    }

    // 换向保护：方向切换时等待 VALVE_MIN_INTERVAL_MS，期间关泵防止冲击。
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

    // 浮力阀 e/f 差动控制（电磁阀做气压隔离）：
    //   上浮：E 通电，F 断电
    //   下沉：E 断电，F 通电
    //   停止：E F 全断电（由上层 STOP 分支处理，此处不会到达）
    const bool ascend = _appliedBuoyancyDirection == BUOYANCY_ASCEND;
    digitalWrite(VALVE_E_PIN, ascend ? HIGH : LOW);
    digitalWrite(VALVE_F_PIN, ascend ? LOW  : HIGH);

    // 软件 PWM：按占空比控制浮力泵。
    const uint32_t dutyUs =
        (static_cast<uint32_t>(_appliedBuoyancyPwm) * BUOYANCY_PWM_PERIOD_US) / 255U;
    const bool pumpOn = dutyUs > 0 && (micros() % BUOYANCY_PWM_PERIOD_US) < dutyUs;
    digitalWrite(PUMP_G_PIN, pumpOn ? HIGH : LOW);
}
