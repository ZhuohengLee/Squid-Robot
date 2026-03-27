/**********************************************************************
 * AutoNavigator.cpp
 *
 * 这个文件实现基于超声波数据的自动寻路模块。
 * 自动模式下，前进和转向由 ESP32 协调，Minima 只执行输出掩码。
 *********************************************************************/

#include "AutoNavigator.h"

namespace {
constexpr float FRONT_BLOCK_CM = 35.0f;
constexpr float SIDE_NEAR_CM = 20.0f;
constexpr float SIDE_PREFERENCE_CM = 8.0f;
constexpr uint32_t TURN_LOCK_MS = 1400;
}

AutoNavigator::AutoNavigator()
    : _forwardControl(nullptr),
      _leftTurnControl(nullptr),
      _rightTurnControl(nullptr),
      _enabled(false),
      _decisionLockUntilMs(0) {}

void AutoNavigator::begin(ForwardControl* forwardControl, LeftTurnControl* leftTurnControl, RightTurnControl* rightTurnControl) {
    _forwardControl = forwardControl;
    _leftTurnControl = leftTurnControl;
    _rightTurnControl = rightTurnControl;
}

void AutoNavigator::setEnabled(bool enabled) {
    _enabled = enabled;
    if (!enabled && _forwardControl) {
        _forwardControl->emergencyStop();
    }
    _decisionLockUntilMs = 0;
}

bool AutoNavigator::isEnabled() const {
    return _enabled;
}

void AutoNavigator::update(const UltrasonicManager& ultrasonicManager, uint32_t nowMs) {
    if (!_enabled || !_forwardControl || !_leftTurnControl || !_rightTurnControl) {
        return;
    }

    if (nowMs < _decisionLockUntilMs) {
        return;
    }

    const bool frontValid = ultrasonicManager.isValid(SENSOR_FRONT);
    const bool leftValid = ultrasonicManager.isValid(SENSOR_LEFT);
    const bool rightValid = ultrasonicManager.isValid(SENSOR_RIGHT);

    const float frontCm = frontValid ? ultrasonicManager.getDistance(SENSOR_FRONT) / 10.0f : 0.0f;
    const float leftCm = leftValid ? ultrasonicManager.getDistance(SENSOR_LEFT) / 10.0f : 0.0f;
    const float rightCm = rightValid ? ultrasonicManager.getDistance(SENSOR_RIGHT) / 10.0f : 0.0f;

    if (!frontValid) {
        _forwardControl->stop();
        return;
    }

    if (frontCm < FRONT_BLOCK_CM) {
        _forwardControl->stop();

        if (leftValid && (!rightValid || leftCm >= rightCm + SIDE_PREFERENCE_CM)) {
            _rightTurnControl->cancel();
            _leftTurnControl->start();
        } else {
            _leftTurnControl->cancel();
            _rightTurnControl->start();
        }

        _decisionLockUntilMs = nowMs + TURN_LOCK_MS;
        return;
    }

    if (leftValid && leftCm < SIDE_NEAR_CM && (!rightValid || rightCm > leftCm + SIDE_PREFERENCE_CM)) {
        _forwardControl->stop();
        _leftTurnControl->cancel();
        _rightTurnControl->start();
        _decisionLockUntilMs = nowMs + TURN_LOCK_MS;
        return;
    }

    if (rightValid && rightCm < SIDE_NEAR_CM && (!leftValid || leftCm > rightCm + SIDE_PREFERENCE_CM)) {
        _forwardControl->stop();
        _rightTurnControl->cancel();
        _leftTurnControl->start();
        _decisionLockUntilMs = nowMs + TURN_LOCK_MS;
        return;
    }

    _forwardControl->start();
}
