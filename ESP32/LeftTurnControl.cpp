/**********************************************************************
 * LeftTurnControl.cpp
 *
 * 这个文件实现左转子系统时序控制。
 *********************************************************************/

#include "LeftTurnControl.h"

namespace {
constexpr uint32_t TURN_DURATION_MS = 1000;
constexpr uint32_t TURN_BALANCE_TIME_MS = 200;
constexpr uint32_t TURN_BALANCE_DELAY_MS = 10;
}

LeftTurnControl::LeftTurnControl()
    : _running(false),
      _balancing(false),
      _balanceValveOpen(false),
      _startMs(0),
      _balanceStartMs(0) {}

void LeftTurnControl::begin() {
    cancel();
}

void LeftTurnControl::start() {
    _running = true;
    _balancing = false;
    _balanceValveOpen = false;
    _startMs = millis();
}

void LeftTurnControl::cancel() {
    _running = false;
    _balancing = false;
    _balanceValveOpen = false;
    _startMs = 0;
    _balanceStartMs = 0;
}

void LeftTurnControl::update(uint32_t nowMs) {
    if (_running && nowMs - _startMs >= TURN_DURATION_MS) {
        _running = false;
        _balancing = true;
        _balanceValveOpen = false;
        _balanceStartMs = nowMs;
    }

    if (_balancing) {
        const uint32_t elapsed = nowMs - _balanceStartMs;

        _balanceValveOpen =
            elapsed >= TURN_BALANCE_DELAY_MS &&
            elapsed < TURN_BALANCE_DELAY_MS + TURN_BALANCE_TIME_MS;

        if (elapsed >= TURN_BALANCE_DELAY_MS + TURN_BALANCE_TIME_MS) {
            _balancing = false;
            _balanceValveOpen = false;
        }
    }
}

uint16_t LeftTurnControl::getMask() const {
    uint16_t mask = 0;

    if (_running) {
        mask |= ACT_TURN_PUMP;
        mask |= ACT_TURN_VALVE_E;
        mask |= ACT_TURN_VALVE_F;
    }

    if (_balancing && _balanceValveOpen) {
        mask |= ACT_TURN_VALVE_E;
    }

    return mask;
}

bool LeftTurnControl::isRunning() const {
    return _running;
}

bool LeftTurnControl::isBalancing() const {
    return _balancing;
}

bool LeftTurnControl::isBusy() const {
    return _running || _balancing;
}
