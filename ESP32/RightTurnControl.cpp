/**********************************************************************
 * RightTurnControl.cpp
 *
 * 这个文件实现右转子系统时序控制。
 *********************************************************************/

#include "RightTurnControl.h"

namespace {
constexpr uint32_t TURN_DURATION_MS = 1000;
constexpr uint32_t TURN_BALANCE_TIME_MS = 200;
constexpr uint32_t TURN_BALANCE_DELAY_MS = 10;
}

RightTurnControl::RightTurnControl()
    : _running(false),
      _balancing(false),
      _balanceValveOpen(false),
      _startMs(0),
      _balanceStartMs(0) {}

void RightTurnControl::begin() {
    cancel();
}

void RightTurnControl::start() {
    _running = true;
    _balancing = false;
    _balanceValveOpen = false;
    _startMs = millis();
}

void RightTurnControl::cancel() {
    _running = false;
    _balancing = false;
    _balanceValveOpen = false;
    _startMs = 0;
    _balanceStartMs = 0;
}

void RightTurnControl::update(uint32_t nowMs) {
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

uint16_t RightTurnControl::getMask() const {
    uint16_t mask = 0;

    if (_running) {
        mask |= ACT_TURN_PUMP;
    }

    if (_balancing && _balanceValveOpen) {
        mask |= ACT_TURN_VALVE_E;
    }

    return mask;
}

bool RightTurnControl::isRunning() const {
    return _running;
}

bool RightTurnControl::isBalancing() const {
    return _balancing;
}

bool RightTurnControl::isBusy() const {
    return _running || _balancing;
}
