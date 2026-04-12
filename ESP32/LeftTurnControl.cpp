/**********************************************************************
 * LeftTurnControl.cpp
 *
 * 左转子系统时序控制。
 *
 * 左转推进：泵 + 阀 E/F 同时打开，持续 TURN_DURATION_MS。
 *   阀 E/F 同时开 → 正循环（气室膨胀，推动机身左转）
 *
 * 气压平衡阶段（转向完成后自动进入）：
 *   延迟 TURN_BALANCE_DELAY_MS 后进入平衡窗口，持续 TURN_BALANCE_TIME_MS。
 *   窗口内阀 E/F 每 TURN_BALANCE_ALT_MS 交替单独打开，
 *   让两气室轮流泄压，隔膜恢复中间位置。
 *********************************************************************/

#include "LeftTurnControl.h"

namespace {
constexpr uint32_t TURN_DURATION_MS        = 1000; // 左转推进持续时间
constexpr uint32_t TURN_BALANCE_DELAY_MS   =   10; // 转向结束后延迟多久开始平衡
constexpr uint32_t TURN_BALANCE_TIME_MS    =  200; // 正常平衡窗口总时长
constexpr uint32_t TURN_BALANCE_ALT_MS     =  100; // 正常平衡每路阀门打开时长
constexpr uint32_t GLOBAL_BALANCE_TIME_MS  = 5000; // 全局平衡（s 命令）总时长
constexpr uint32_t GLOBAL_BALANCE_ALT_MS   =  500; // 全局平衡每路阀门打开时长
}

LeftTurnControl::LeftTurnControl()
    : _running(false),
      _balancing(false),
      _balanceValveOpen(false),
      _balanceAltPhase(false),
      _startMs(0),
      _balanceStartMs(0),
      _balanceAltMs(TURN_BALANCE_ALT_MS),
      _balanceTotalMs(TURN_BALANCE_TIME_MS) {}

void LeftTurnControl::begin() {
    cancel();
}

void LeftTurnControl::start() {
    _running          = true;
    _balancing        = false;
    _balanceValveOpen = false;
    _balanceAltPhase  = false;
    _startMs          = 0;  // 由第一次 update() 初始化，避免时间差导致立即跳到平衡阶段
}

void LeftTurnControl::cancel() {
    _running          = false;
    _balancing        = false;
    _balanceValveOpen = false;
    _balanceAltPhase  = false;
    _startMs          = 0;
    _balanceStartMs   = 0;
}

void LeftTurnControl::forceBalance() {
    // 不管当前状态，直接进入全局平衡（500ms/5s）。
    _running          = false;
    _balancing        = true;
    _balanceValveOpen = false;
    _balanceAltPhase  = false;
    _startMs          = 0;
    _balanceStartMs   = 0;
    _balanceAltMs     = GLOBAL_BALANCE_ALT_MS;
    _balanceTotalMs   = GLOBAL_BALANCE_TIME_MS;
}

void LeftTurnControl::update(uint32_t nowMs) {
    // 推进阶段：计时结束后进入平衡阶段。
    // _startMs == 0 表示本轮刚启动，延迟到这里初始化，保证与 nowMs 基准一致。
    if (_running && _startMs == 0) {
        _startMs = nowMs;
    }
    if (_running && _startMs != 0 && nowMs - _startMs >= TURN_DURATION_MS) {
        _running          = false;
        _balancing        = true;
        _balanceValveOpen = false;
        _balanceAltPhase  = false;
        _balanceStartMs   = nowMs;
        _balanceAltMs     = TURN_BALANCE_ALT_MS;
        _balanceTotalMs   = TURN_BALANCE_TIME_MS;
    }

    // 平衡阶段：在平衡窗口内每 TURN_BALANCE_ALT_MS 切换交替相位。
    if (_balancing) {
        const uint32_t elapsed = nowMs - _balanceStartMs;

        _balanceValveOpen =
            elapsed >= TURN_BALANCE_DELAY_MS &&
            elapsed < TURN_BALANCE_DELAY_MS + _balanceTotalMs;

        if (_balanceValveOpen) {
            const uint32_t inWindow = elapsed - TURN_BALANCE_DELAY_MS;
            _balanceAltPhase = (inWindow / _balanceAltMs) % 2 != 0;
        }

        if (elapsed >= TURN_BALANCE_DELAY_MS + _balanceTotalMs) {
            _balancing        = false;
            _balanceValveOpen = false;
            _balanceAltPhase  = false;
        }
    }
}

uint16_t LeftTurnControl::getMask() const {
    uint16_t mask = 0;

    // 推进阶段：泵 + 阀 c/d 同时打开（正循环）。
    if (_running) {
        mask |= ACT_TURN_PUMP;
        mask |= ACT_TURN_VALVE_C;
        mask |= ACT_TURN_VALVE_D;
    }

    // 平衡阶段：阀 c/d 交替单独打开，让两气室轮流泄压归中。
    if (_balancing && _balanceValveOpen) {
        if (_balanceAltPhase) {
            mask |= ACT_TURN_VALVE_D;
        } else {
            mask |= ACT_TURN_VALVE_C;
        }
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
