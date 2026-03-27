/**********************************************************************
 * RightTurnControl.cpp
 *
 * 右转子系统时序控制。
 *
 * 右转推进：泵开，阀 E/F 全部关闭，持续 TURN_DURATION_MS。
 *   阀 E/F 全关 → 负循环（气室收缩，推动机身右转）
 *
 * 气压平衡阶段（转向完成后自动进入）：
 *   延迟 TURN_BALANCE_DELAY_MS 后进入平衡窗口，持续 TURN_BALANCE_TIME_MS。
 *   窗口内阀 E/F 每 TURN_BALANCE_ALT_MS 交替单独打开，
 *   让两气室轮流泄压，隔膜恢复中间位置。
 *********************************************************************/

#include "RightTurnControl.h"

namespace {
constexpr uint32_t TURN_DURATION_MS        = 1000; // 右转推进持续时间
constexpr uint32_t TURN_BALANCE_DELAY_MS   =   10; // 转向结束后延迟多久开始平衡
constexpr uint32_t TURN_BALANCE_TIME_MS    =  200; // 平衡窗口总时长
constexpr uint32_t TURN_BALANCE_ALT_MS     =  100; // 平衡窗口内每路阀门打开时长
}

RightTurnControl::RightTurnControl()
    : _running(false),
      _balancing(false),
      _balanceValveOpen(false),
      _balanceAltPhase(false),
      _startMs(0),
      _balanceStartMs(0) {}

void RightTurnControl::begin() {
    cancel();
}

void RightTurnControl::start() {
    _running          = true;
    _balancing        = false;
    _balanceValveOpen = false;
    _balanceAltPhase  = false;
    _startMs          = 0;  // 由第一次 update() 初始化，避免时间差导致立即跳到平衡阶段
}

void RightTurnControl::cancel() {
    _running          = false;
    _balancing        = false;
    _balanceValveOpen = false;
    _balanceAltPhase  = false;
    _startMs          = 0;
    _balanceStartMs   = 0;
}

void RightTurnControl::update(uint32_t nowMs) {
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
    }

    // 平衡阶段：在平衡窗口内每 TURN_BALANCE_ALT_MS 切换交替相位。
    if (_balancing) {
        const uint32_t elapsed = nowMs - _balanceStartMs;

        _balanceValveOpen =
            elapsed >= TURN_BALANCE_DELAY_MS &&
            elapsed < TURN_BALANCE_DELAY_MS + TURN_BALANCE_TIME_MS;

        if (_balanceValveOpen) {
            const uint32_t inWindow = elapsed - TURN_BALANCE_DELAY_MS;
            _balanceAltPhase = (inWindow / TURN_BALANCE_ALT_MS) % 2 != 0;
        }

        if (elapsed >= TURN_BALANCE_DELAY_MS + TURN_BALANCE_TIME_MS) {
            _balancing        = false;
            _balanceValveOpen = false;
            _balanceAltPhase  = false;
        }
    }
}

uint16_t RightTurnControl::getMask() const {
    uint16_t mask = 0;

    // 推进阶段：仅泵开，阀 c/d 全关（负循环）。
    if (_running) {
        mask |= ACT_TURN_PUMP;
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

bool RightTurnControl::isRunning() const {
    return _running;
}

bool RightTurnControl::isBalancing() const {
    return _balancing;
}

bool RightTurnControl::isBusy() const {
    return _running || _balancing;
}
