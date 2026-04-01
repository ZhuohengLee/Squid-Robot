/**********************************************************************
 * ForwardControl.cpp
 *
 * 前进子系统时序控制。
 *
 * 正常推进节拍：
 *   泵持续运行，阀 B/C 每 FORWARD_VALVE_INTERVAL_MS 同时切换一次。
 *   阀开  → 正循环（气室充气膨胀）
 *   阀关  → 负循环（气室收缩回弹）
 *
 * 气压平衡阶段（stop() 触发）：
 *   延迟 FORWARD_BALANCE_DELAY_MS 后进入平衡窗口，持续 FORWARD_BALANCE_TIME_MS。
 *   窗口内阀 B/C 每 FORWARD_BALANCE_ALT_MS 交替单独打开，
 *   让两气室轮流泄压，隔膜恢复中间位置。
 *********************************************************************/

#include "ForwardControl.h"

namespace {
constexpr uint32_t FORWARD_VALVE_INTERVAL_MS  =  500; // 正常推进阀门切换周期
constexpr uint32_t FORWARD_BALANCE_DELAY_MS   =   10; // stop() 后延迟多久开始平衡
constexpr uint32_t FORWARD_BALANCE_TIME_MS    = 5000; // 平衡窗口总时长
constexpr uint32_t FORWARD_BALANCE_ALT_MS     =  500; // 平衡窗口内每路阀门打开时长
}

ForwardControl::ForwardControl()
    : _running(false),
      _balancing(false),
      _valvesOpen(false),
      _balanceValveOpen(false),
      _balanceAltPhase(false),
      _phaseStartMs(0),
      _balanceStartMs(0) {}

void ForwardControl::begin() {
    emergencyStop();
}

void ForwardControl::start() {
    _running          = true;
    _balancing        = false;
    _valvesOpen       = true;  // 启动时阀门打开，第一个节拍动作是关阀建压
    _balanceValveOpen = false;
    _balanceAltPhase  = false;
    _phaseStartMs     = 0;     // 由第一次 update() 初始化，避免与 nowMs 时间差导致立即翻转
}

void ForwardControl::stop() {
    if (!_running) {
        return;
    }

    _running          = false;
    _balancing        = true;
    _valvesOpen       = false;
    _balanceValveOpen = false;
    _balanceAltPhase  = false;
    _balanceStartMs   = 0;  // 由第一次 update() 初始化，避免时间差导致平衡阶段立即结束
}

void ForwardControl::emergencyStop() {
    _running          = false;
    _balancing        = false;
    _valvesOpen       = false;
    _balanceValveOpen = false;
    _balanceAltPhase  = false;
    _phaseStartMs     = 0;
    _balanceStartMs   = 0;
}

void ForwardControl::update(uint32_t nowMs) {
    // 推进阶段：按周期切换阀门（正/负循环交替）。
    // _phaseStartMs == 0 表示本轮刚启动，延迟到这里初始化，保证与 nowMs 基准一致。
    if (_running) {
        if (_phaseStartMs == 0) {
            _phaseStartMs = nowMs;
        } else if (nowMs - _phaseStartMs >= FORWARD_VALVE_INTERVAL_MS) {
            _valvesOpen   = !_valvesOpen;
            _phaseStartMs = nowMs;
        }
    }

    // 平衡阶段：在平衡窗口内每 FORWARD_BALANCE_ALT_MS 切换交替相位。
    if (_balancing) {
        if (_balanceStartMs == 0) {
            _balanceStartMs = nowMs;
        }
        const uint32_t elapsed = nowMs - _balanceStartMs;

        _balanceValveOpen =
            elapsed >= FORWARD_BALANCE_DELAY_MS &&
            elapsed < FORWARD_BALANCE_DELAY_MS + FORWARD_BALANCE_TIME_MS;

        if (_balanceValveOpen) {
            const uint32_t inWindow = elapsed - FORWARD_BALANCE_DELAY_MS;
            _balanceAltPhase = (inWindow / FORWARD_BALANCE_ALT_MS) % 2 != 0;
        }

        if (elapsed >= FORWARD_BALANCE_DELAY_MS + FORWARD_BALANCE_TIME_MS) {
            _balancing        = false;
            _balanceValveOpen = false;
            _balanceAltPhase  = false;
        }
    }
}

uint16_t ForwardControl::getMask() const {
    uint16_t mask = 0;

    if (_running) {
        mask |= ACT_FORWARD_PUMP;
        if (_valvesOpen) {
            mask |= ACT_FORWARD_VALVE_A;
            mask |= ACT_FORWARD_VALVE_B;
        }
    }

    // 平衡阶段：阀 a/b 交替单独打开，让两气室轮流泄压归中。
    if (_balancing && _balanceValveOpen) {
        if (_balanceAltPhase) {
            mask |= ACT_FORWARD_VALVE_B;
        } else {
            mask |= ACT_FORWARD_VALVE_A;
        }
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
