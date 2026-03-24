/**********************************************************************
 * AutoNavigator.cpp
 *
 * 这个文件实现基于超声波的自动寻路模块。
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
        _forwardControl->stop();
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

    bool frontValid = ultrasonicManager.isValid(SENSOR_FRONT);
    bool leftValid = ultrasonicManager.isValid(SENSOR_LEFT);
    bool rightValid = ultrasonicManager.isValid(SENSOR_RIGHT);

    float frontCm = frontValid ? ultrasonicManager.getDistance(SENSOR_FRONT) / 10.0f : 0.0f;
    float leftCm = leftValid ? ultrasonicManager.getDistance(SENSOR_LEFT) / 10.0f : 0.0f;
    float rightCm = rightValid ? ultrasonicManager.getDistance(SENSOR_RIGHT) / 10.0f : 0.0f;

    if (!frontValid) {
        _forwardControl->stop();
        return;
    }

    if (frontCm < FRONT_BLOCK_CM) {
        _forwardControl->stop();
        if (leftValid && (!rightValid || leftCm >= rightCm + SIDE_PREFERENCE_CM)) {
            _leftTurnControl->execute();
        } else {
            _rightTurnControl->execute();
        }
        _decisionLockUntilMs = nowMs + TURN_LOCK_MS;
        return;
    }

    if (leftValid && leftCm < SIDE_NEAR_CM && (!rightValid || rightCm > leftCm + SIDE_PREFERENCE_CM)) {
        _forwardControl->stop();
        _rightTurnControl->execute();
        _decisionLockUntilMs = nowMs + TURN_LOCK_MS;
        return;
    }

    if (rightValid && rightCm < SIDE_NEAR_CM && (!leftValid || leftCm > rightCm + SIDE_PREFERENCE_CM)) {
        _forwardControl->stop();
        _leftTurnControl->execute();
        _decisionLockUntilMs = nowMs + TURN_LOCK_MS;
        return;
    }

    _forwardControl->start();
}
