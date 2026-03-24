/**********************************************************************
 * ForwardControl.cpp
 *
 * 这个文件实现前进子系统的时序控制。
 *********************************************************************/

#include "ForwardControl.h"

namespace {
// 前进阀门交替开关周期。
constexpr uint32_t FORWARD_VALVE_INTERVAL_MS = 600;

// 停止前进后的平衡时间窗口。
constexpr uint32_t FORWARD_BALANCE_TIME_MS = 500;
constexpr uint32_t FORWARD_BALANCE_DELAY_MS = 10;
}

ForwardControl::ForwardControl()
    : _running(false),
      _balancing(false),
      _valvesOpen(false),
      _balanceValveOpen(false),
      _phaseStartMs(0),
      _balanceStartMs(0) {}

void ForwardControl::begin() {
    emergencyStop();
}

void ForwardControl::start() {
    _running = true;
    _balancing = false;
    _valvesOpen = false;
    _balanceValveOpen = false;
    _phaseStartMs = millis();
}

void ForwardControl::stop() {
    if (!_running) {
        return;
    }

    _running = false;
    _balancing = true;
    _valvesOpen = false;
    _balanceValveOpen = false;
    _balanceStartMs = millis();
}

void ForwardControl::emergencyStop() {
    _running = false;
    _balancing = false;
    _valvesOpen = false;
    _balanceValveOpen = false;
    _phaseStartMs = 0;
    _balanceStartMs = 0;
}

void ForwardControl::update(uint32_t nowMs) {
    if (_running && nowMs - _phaseStartMs >= FORWARD_VALVE_INTERVAL_MS) {
        _valvesOpen = !_valvesOpen;
        _phaseStartMs = nowMs;
    }

    if (_balancing) {
        const uint32_t elapsed = nowMs - _balanceStartMs;

        _balanceValveOpen =
            elapsed >= FORWARD_BALANCE_DELAY_MS &&
            elapsed < FORWARD_BALANCE_DELAY_MS + FORWARD_BALANCE_TIME_MS;

        if (elapsed >= FORWARD_BALANCE_DELAY_MS + FORWARD_BALANCE_TIME_MS) {
            _balancing = false;
            _balanceValveOpen = false;
        }
    }
}

uint16_t ForwardControl::getMask() const {
    uint16_t mask = 0;

    if (_running) {
        mask |= ACT_FORWARD_PUMP;
        if (_valvesOpen) {
            mask |= ACT_FORWARD_VALVE_B;
            mask |= ACT_FORWARD_VALVE_C;
        }
    }

    if (_balancing && _balanceValveOpen) {
        mask |= ACT_FORWARD_VALVE_B;
    }

    return mask;
}

bool ForwardControl::isRunning() const {
    return _running;
}

bool ForwardControl::isBalancing() const {
    return _balancing;
}

bool ForwardControl::isBusy() const {
    return _running || _balancing;
}
