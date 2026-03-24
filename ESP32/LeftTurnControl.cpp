/**********************************************************************
 * LeftTurnControl.cpp
 *
 * 杩欎釜鏂囦欢瀹炵幇宸﹁浆瀛愮郴缁熸椂搴忔帶鍒躲€? *********************************************************************/

// 中文逐行说明：下面这一行保留原始代码 -> #include "LeftTurnControl.h"
#include "LeftTurnControl.h"

// 中文逐行说明：下面这一行保留原始代码 -> namespace {
namespace {
// 中文逐行说明：下面这一行保留原始代码 -> constexpr uint32_t TURN_DURATION_MS = 1000;
constexpr uint32_t TURN_DURATION_MS = 1000;
// 中文逐行说明：下面这一行保留原始代码 -> constexpr uint32_t TURN_BALANCE_TIME_MS = 200;
constexpr uint32_t TURN_BALANCE_TIME_MS = 200;
// 中文逐行说明：下面这一行保留原始代码 -> constexpr uint32_t TURN_BALANCE_DELAY_MS = 10;
constexpr uint32_t TURN_BALANCE_DELAY_MS = 10;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> LeftTurnControl::LeftTurnControl()
LeftTurnControl::LeftTurnControl()
    // 中文逐行说明：下面这一行保留原始代码 -> : _running(false),
    : _running(false),
      // 中文逐行说明：下面这一行保留原始代码 -> _balancing(false),
      _balancing(false),
      // 中文逐行说明：下面这一行保留原始代码 -> _balanceValveOpen(false),
      _balanceValveOpen(false),
      // 中文逐行说明：下面这一行保留原始代码 -> _startMs(0),
      _startMs(0),
      // 中文逐行说明：下面这一行保留原始代码 -> _balanceStartMs(0) {}
      _balanceStartMs(0) {}

// 中文逐行说明：下面这一行保留原始代码 -> void LeftTurnControl::begin() {
void LeftTurnControl::begin() {
    // 中文逐行说明：下面这一行保留原始代码 -> cancel();
    cancel();
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void LeftTurnControl::start() {
void LeftTurnControl::start() {
    // 中文逐行说明：下面这一行保留原始代码 -> _running = true;
    _running = true;
    // 中文逐行说明：下面这一行保留原始代码 -> _balancing = false;
    _balancing = false;
    // 中文逐行说明：下面这一行保留原始代码 -> _balanceValveOpen = false;
    _balanceValveOpen = false;
    // 中文逐行说明：下面这一行保留原始代码 -> _startMs = millis();
    _startMs = millis();
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void LeftTurnControl::cancel() {
void LeftTurnControl::cancel() {
    // 中文逐行说明：下面这一行保留原始代码 -> _running = false;
    _running = false;
    // 中文逐行说明：下面这一行保留原始代码 -> _balancing = false;
    _balancing = false;
    // 中文逐行说明：下面这一行保留原始代码 -> _balanceValveOpen = false;
    _balanceValveOpen = false;
    // 中文逐行说明：下面这一行保留原始代码 -> _startMs = 0;
    _startMs = 0;
    // 中文逐行说明：下面这一行保留原始代码 -> _balanceStartMs = 0;
    _balanceStartMs = 0;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void LeftTurnControl::update(uint32_t nowMs) {
void LeftTurnControl::update(uint32_t nowMs) {
    // 中文逐行说明：下面这一行保留原始代码 -> if (_running && nowMs - _startMs >= TURN_DURATION_MS) {
    if (_running && nowMs - _startMs >= TURN_DURATION_MS) {
        // 中文逐行说明：下面这一行保留原始代码 -> _running = false;
        _running = false;
        // 中文逐行说明：下面这一行保留原始代码 -> _balancing = true;
        _balancing = true;
        // 中文逐行说明：下面这一行保留原始代码 -> _balanceValveOpen = false;
        _balanceValveOpen = false;
        // 中文逐行说明：下面这一行保留原始代码 -> _balanceStartMs = nowMs;
        _balanceStartMs = nowMs;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (_balancing) {
    if (_balancing) {
        // 中文逐行说明：下面这一行保留原始代码 -> const uint32_t elapsed = nowMs - _balanceStartMs;
        const uint32_t elapsed = nowMs - _balanceStartMs;

        // 中文逐行说明：下面这一行保留原始代码 -> _balanceValveOpen =
        _balanceValveOpen =
            // 中文逐行说明：下面这一行保留原始代码 -> elapsed >= TURN_BALANCE_DELAY_MS &&
            elapsed >= TURN_BALANCE_DELAY_MS &&
            // 中文逐行说明：下面这一行保留原始代码 -> elapsed < TURN_BALANCE_DELAY_MS + TURN_BALANCE_TIME_MS;
            elapsed < TURN_BALANCE_DELAY_MS + TURN_BALANCE_TIME_MS;

        // 中文逐行说明：下面这一行保留原始代码 -> if (elapsed >= TURN_BALANCE_DELAY_MS + TURN_BALANCE_TIME_MS) {
        if (elapsed >= TURN_BALANCE_DELAY_MS + TURN_BALANCE_TIME_MS) {
            // 中文逐行说明：下面这一行保留原始代码 -> _balancing = false;
            _balancing = false;
            // 中文逐行说明：下面这一行保留原始代码 -> _balanceValveOpen = false;
            _balanceValveOpen = false;
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> uint16_t LeftTurnControl::getMask() const {
uint16_t LeftTurnControl::getMask() const {
    // 中文逐行说明：下面这一行保留原始代码 -> uint16_t mask = 0;
    uint16_t mask = 0;

    // 中文逐行说明：下面这一行保留原始代码 -> if (_running) {
    if (_running) {
        // 中文逐行说明：下面这一行保留原始代码 -> mask |= ACT_TURN_PUMP;
        mask |= ACT_TURN_PUMP;
        // 中文逐行说明：下面这一行保留原始代码 -> mask |= ACT_TURN_VALVE_E;
        mask |= ACT_TURN_VALVE_E;
        // 中文逐行说明：下面这一行保留原始代码 -> mask |= ACT_TURN_VALVE_F;
        mask |= ACT_TURN_VALVE_F;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (_balancing && _balanceValveOpen) {
    if (_balancing && _balanceValveOpen) {
        // 中文逐行说明：下面这一行保留原始代码 -> mask |= ACT_TURN_VALVE_E;
        mask |= ACT_TURN_VALVE_E;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> return mask;
    return mask;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> bool LeftTurnControl::isRunning() const {
bool LeftTurnControl::isRunning() const {
    // 中文逐行说明：下面这一行保留原始代码 -> return _running;
    return _running;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> bool LeftTurnControl::isBalancing() const {
bool LeftTurnControl::isBalancing() const {
    // 中文逐行说明：下面这一行保留原始代码 -> return _balancing;
    return _balancing;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> bool LeftTurnControl::isBusy() const {
bool LeftTurnControl::isBusy() const {
    // 中文逐行说明：下面这一行保留原始代码 -> return _running || _balancing;
    return _running || _balancing;
// 中文逐行说明：下面这一行保留原始代码 -> }
}
