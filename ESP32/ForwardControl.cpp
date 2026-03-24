/**********************************************************************
 * ForwardControl.cpp
 *
 * 杩欎釜鏂囦欢瀹炵幇鍓嶈繘瀛愮郴缁熺殑鏃跺簭鎺у埗銆? *********************************************************************/

// 中文逐行说明：下面这一行保留原始代码 -> #include "ForwardControl.h"
#include "ForwardControl.h"

// 中文逐行说明：下面这一行保留原始代码 -> namespace {
namespace {
// 鍓嶈繘闃€闂ㄤ氦鏇垮紑鍏冲懆鏈熴€?constexpr uint32_t FORWARD_VALVE_INTERVAL_MS = 600;

// 鍋滄鍓嶈繘鍚庣殑骞宠　鏃堕棿绐楀彛銆?constexpr uint32_t FORWARD_BALANCE_TIME_MS = 500;
// 中文逐行说明：下面这一行保留原始代码 -> constexpr uint32_t FORWARD_BALANCE_DELAY_MS = 10;
constexpr uint32_t FORWARD_BALANCE_DELAY_MS = 10;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> ForwardControl::ForwardControl()
ForwardControl::ForwardControl()
    // 中文逐行说明：下面这一行保留原始代码 -> : _running(false),
    : _running(false),
      // 中文逐行说明：下面这一行保留原始代码 -> _balancing(false),
      _balancing(false),
      // 中文逐行说明：下面这一行保留原始代码 -> _valvesOpen(false),
      _valvesOpen(false),
      // 中文逐行说明：下面这一行保留原始代码 -> _balanceValveOpen(false),
      _balanceValveOpen(false),
      // 中文逐行说明：下面这一行保留原始代码 -> _phaseStartMs(0),
      _phaseStartMs(0),
      // 中文逐行说明：下面这一行保留原始代码 -> _balanceStartMs(0) {}
      _balanceStartMs(0) {}

// 中文逐行说明：下面这一行保留原始代码 -> void ForwardControl::begin() {
void ForwardControl::begin() {
    // 中文逐行说明：下面这一行保留原始代码 -> emergencyStop();
    emergencyStop();
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void ForwardControl::start() {
void ForwardControl::start() {
    // 中文逐行说明：下面这一行保留原始代码 -> _running = true;
    _running = true;
    // 中文逐行说明：下面这一行保留原始代码 -> _balancing = false;
    _balancing = false;
    // 中文逐行说明：下面这一行保留原始代码 -> _valvesOpen = false;
    _valvesOpen = false;
    // 中文逐行说明：下面这一行保留原始代码 -> _balanceValveOpen = false;
    _balanceValveOpen = false;
    // 中文逐行说明：下面这一行保留原始代码 -> _phaseStartMs = millis();
    _phaseStartMs = millis();
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void ForwardControl::stop() {
void ForwardControl::stop() {
    // 中文逐行说明：下面这一行保留原始代码 -> if (!_running) {
    if (!_running) {
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> _running = false;
    _running = false;
    // 中文逐行说明：下面这一行保留原始代码 -> _balancing = true;
    _balancing = true;
    // 中文逐行说明：下面这一行保留原始代码 -> _valvesOpen = false;
    _valvesOpen = false;
    // 中文逐行说明：下面这一行保留原始代码 -> _balanceValveOpen = false;
    _balanceValveOpen = false;
    // 中文逐行说明：下面这一行保留原始代码 -> _balanceStartMs = millis();
    _balanceStartMs = millis();
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void ForwardControl::emergencyStop() {
void ForwardControl::emergencyStop() {
    // 中文逐行说明：下面这一行保留原始代码 -> _running = false;
    _running = false;
    // 中文逐行说明：下面这一行保留原始代码 -> _balancing = false;
    _balancing = false;
    // 中文逐行说明：下面这一行保留原始代码 -> _valvesOpen = false;
    _valvesOpen = false;
    // 中文逐行说明：下面这一行保留原始代码 -> _balanceValveOpen = false;
    _balanceValveOpen = false;
    // 中文逐行说明：下面这一行保留原始代码 -> _phaseStartMs = 0;
    _phaseStartMs = 0;
    // 中文逐行说明：下面这一行保留原始代码 -> _balanceStartMs = 0;
    _balanceStartMs = 0;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void ForwardControl::update(uint32_t nowMs) {
void ForwardControl::update(uint32_t nowMs) {
    // 中文逐行说明：下面这一行保留原始代码 -> if (_running && nowMs - _phaseStartMs >= FORWARD_VALVE_INTERVAL_MS) {
    if (_running && nowMs - _phaseStartMs >= FORWARD_VALVE_INTERVAL_MS) {
        // 中文逐行说明：下面这一行保留原始代码 -> _valvesOpen = !_valvesOpen;
        _valvesOpen = !_valvesOpen;
        // 中文逐行说明：下面这一行保留原始代码 -> _phaseStartMs = nowMs;
        _phaseStartMs = nowMs;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (_balancing) {
    if (_balancing) {
        // 中文逐行说明：下面这一行保留原始代码 -> const uint32_t elapsed = nowMs - _balanceStartMs;
        const uint32_t elapsed = nowMs - _balanceStartMs;

        // 中文逐行说明：下面这一行保留原始代码 -> _balanceValveOpen =
        _balanceValveOpen =
            // 中文逐行说明：下面这一行保留原始代码 -> elapsed >= FORWARD_BALANCE_DELAY_MS &&
            elapsed >= FORWARD_BALANCE_DELAY_MS &&
            // 中文逐行说明：下面这一行保留原始代码 -> elapsed < FORWARD_BALANCE_DELAY_MS + FORWARD_BALANCE_TIME_MS;
            elapsed < FORWARD_BALANCE_DELAY_MS + FORWARD_BALANCE_TIME_MS;

        // 中文逐行说明：下面这一行保留原始代码 -> if (elapsed >= FORWARD_BALANCE_DELAY_MS + FORWARD_BALANCE_TIME_MS) {
        if (elapsed >= FORWARD_BALANCE_DELAY_MS + FORWARD_BALANCE_TIME_MS) {
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

// 中文逐行说明：下面这一行保留原始代码 -> uint16_t ForwardControl::getMask() const {
uint16_t ForwardControl::getMask() const {
    // 中文逐行说明：下面这一行保留原始代码 -> uint16_t mask = 0;
    uint16_t mask = 0;

    // 中文逐行说明：下面这一行保留原始代码 -> if (_running) {
    if (_running) {
        // 中文逐行说明：下面这一行保留原始代码 -> mask |= ACT_FORWARD_PUMP;
        mask |= ACT_FORWARD_PUMP;
        // 中文逐行说明：下面这一行保留原始代码 -> if (_valvesOpen) {
        if (_valvesOpen) {
            // 中文逐行说明：下面这一行保留原始代码 -> mask |= ACT_FORWARD_VALVE_B;
            mask |= ACT_FORWARD_VALVE_B;
            // 中文逐行说明：下面这一行保留原始代码 -> mask |= ACT_FORWARD_VALVE_C;
            mask |= ACT_FORWARD_VALVE_C;
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (_balancing && _balanceValveOpen) {
    if (_balancing && _balanceValveOpen) {
        // 中文逐行说明：下面这一行保留原始代码 -> mask |= ACT_FORWARD_VALVE_B;
        mask |= ACT_FORWARD_VALVE_B;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> return mask;
    return mask;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> bool ForwardControl::isRunning() const {
bool ForwardControl::isRunning() const {
    // 中文逐行说明：下面这一行保留原始代码 -> return _running;
    return _running;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> bool ForwardControl::isBalancing() const {
bool ForwardControl::isBalancing() const {
    // 中文逐行说明：下面这一行保留原始代码 -> return _balancing;
    return _balancing;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> bool ForwardControl::isBusy() const {
bool ForwardControl::isBusy() const {
    // 中文逐行说明：下面这一行保留原始代码 -> return _running || _balancing;
    return _running || _balancing;
// 中文逐行说明：下面这一行保留原始代码 -> }
}
