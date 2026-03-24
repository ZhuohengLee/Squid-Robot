/**********************************************************************
 * AutoNavigator.cpp
 *
 * 杩欎釜鏂囦欢瀹炵幇鍩轰簬瓒呭０娉㈡暟鎹殑鑷姩瀵昏矾妯″潡銆? * 鑷姩妯″紡涓嬶紝鍓嶈繘鍜岃浆鍚戠敱 ESP32 鍗忚皟锛孧inima 鍙墽琛岃緭鍑烘帺鐮併€? *********************************************************************/

// 中文逐行说明：下面这一行保留原始代码 -> #include "AutoNavigator.h"
#include "AutoNavigator.h"

// 中文逐行说明：下面这一行保留原始代码 -> namespace {
namespace {
// 中文逐行说明：下面这一行保留原始代码 -> constexpr float FRONT_BLOCK_CM = 35.0f;
constexpr float FRONT_BLOCK_CM = 35.0f;
// 中文逐行说明：下面这一行保留原始代码 -> constexpr float SIDE_NEAR_CM = 20.0f;
constexpr float SIDE_NEAR_CM = 20.0f;
// 中文逐行说明：下面这一行保留原始代码 -> constexpr float SIDE_PREFERENCE_CM = 8.0f;
constexpr float SIDE_PREFERENCE_CM = 8.0f;
// 中文逐行说明：下面这一行保留原始代码 -> constexpr uint32_t TURN_LOCK_MS = 1400;
constexpr uint32_t TURN_LOCK_MS = 1400;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> AutoNavigator::AutoNavigator()
AutoNavigator::AutoNavigator()
    // 中文逐行说明：下面这一行保留原始代码 -> : _forwardControl(nullptr),
    : _forwardControl(nullptr),
      // 中文逐行说明：下面这一行保留原始代码 -> _leftTurnControl(nullptr),
      _leftTurnControl(nullptr),
      // 中文逐行说明：下面这一行保留原始代码 -> _rightTurnControl(nullptr),
      _rightTurnControl(nullptr),
      // 中文逐行说明：下面这一行保留原始代码 -> _enabled(false),
      _enabled(false),
      // 中文逐行说明：下面这一行保留原始代码 -> _decisionLockUntilMs(0) {}
      _decisionLockUntilMs(0) {}

// 中文逐行说明：下面这一行保留原始代码 -> void AutoNavigator::begin(ForwardControl* forwardControl, LeftTurnControl* leftTurnControl, RightTurnControl* rightTurnControl) {
void AutoNavigator::begin(ForwardControl* forwardControl, LeftTurnControl* leftTurnControl, RightTurnControl* rightTurnControl) {
    // 中文逐行说明：下面这一行保留原始代码 -> _forwardControl = forwardControl;
    _forwardControl = forwardControl;
    // 中文逐行说明：下面这一行保留原始代码 -> _leftTurnControl = leftTurnControl;
    _leftTurnControl = leftTurnControl;
    // 中文逐行说明：下面这一行保留原始代码 -> _rightTurnControl = rightTurnControl;
    _rightTurnControl = rightTurnControl;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void AutoNavigator::setEnabled(bool enabled) {
void AutoNavigator::setEnabled(bool enabled) {
    // 中文逐行说明：下面这一行保留原始代码 -> _enabled = enabled;
    _enabled = enabled;
    // 中文逐行说明：下面这一行保留原始代码 -> if (!enabled && _forwardControl) {
    if (!enabled && _forwardControl) {
        // 中文逐行说明：下面这一行保留原始代码 -> _forwardControl->emergencyStop();
        _forwardControl->emergencyStop();
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
    // 中文逐行说明：下面这一行保留原始代码 -> _decisionLockUntilMs = 0;
    _decisionLockUntilMs = 0;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> bool AutoNavigator::isEnabled() const {
bool AutoNavigator::isEnabled() const {
    // 中文逐行说明：下面这一行保留原始代码 -> return _enabled;
    return _enabled;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void AutoNavigator::update(const UltrasonicManager& ultrasonicManager, uint32_t nowMs) {
void AutoNavigator::update(const UltrasonicManager& ultrasonicManager, uint32_t nowMs) {
    // 中文逐行说明：下面这一行保留原始代码 -> if (!_enabled || !_forwardControl || !_leftTurnControl || !_rightTurnControl) {
    if (!_enabled || !_forwardControl || !_leftTurnControl || !_rightTurnControl) {
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (nowMs < _decisionLockUntilMs) {
    if (nowMs < _decisionLockUntilMs) {
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> const bool frontValid = ultrasonicManager.isValid(SENSOR_FRONT);
    const bool frontValid = ultrasonicManager.isValid(SENSOR_FRONT);
    // 中文逐行说明：下面这一行保留原始代码 -> const bool leftValid = ultrasonicManager.isValid(SENSOR_LEFT);
    const bool leftValid = ultrasonicManager.isValid(SENSOR_LEFT);
    // 中文逐行说明：下面这一行保留原始代码 -> const bool rightValid = ultrasonicManager.isValid(SENSOR_RIGHT);
    const bool rightValid = ultrasonicManager.isValid(SENSOR_RIGHT);

    // 中文逐行说明：下面这一行保留原始代码 -> const float frontCm = frontValid ? ultrasonicManager.getDistance(SENSOR_FRONT) / 10.0f : 0.0f;
    const float frontCm = frontValid ? ultrasonicManager.getDistance(SENSOR_FRONT) / 10.0f : 0.0f;
    // 中文逐行说明：下面这一行保留原始代码 -> const float leftCm = leftValid ? ultrasonicManager.getDistance(SENSOR_LEFT) / 10.0f : 0.0f;
    const float leftCm = leftValid ? ultrasonicManager.getDistance(SENSOR_LEFT) / 10.0f : 0.0f;
    // 中文逐行说明：下面这一行保留原始代码 -> const float rightCm = rightValid ? ultrasonicManager.getDistance(SENSOR_RIGHT) / 10.0f : 0.0f;
    const float rightCm = rightValid ? ultrasonicManager.getDistance(SENSOR_RIGHT) / 10.0f : 0.0f;

    // 中文逐行说明：下面这一行保留原始代码 -> if (!frontValid) {
    if (!frontValid) {
        // 中文逐行说明：下面这一行保留原始代码 -> _forwardControl->stop();
        _forwardControl->stop();
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (frontCm < FRONT_BLOCK_CM) {
    if (frontCm < FRONT_BLOCK_CM) {
        // 中文逐行说明：下面这一行保留原始代码 -> _forwardControl->stop();
        _forwardControl->stop();

        // 中文逐行说明：下面这一行保留原始代码 -> if (leftValid && (!rightValid || leftCm >= rightCm + SIDE_PREFERENCE_CM)) {
        if (leftValid && (!rightValid || leftCm >= rightCm + SIDE_PREFERENCE_CM)) {
            // 中文逐行说明：下面这一行保留原始代码 -> _rightTurnControl->cancel();
            _rightTurnControl->cancel();
            // 中文逐行说明：下面这一行保留原始代码 -> _leftTurnControl->start();
            _leftTurnControl->start();
        // 中文逐行说明：下面这一行保留原始代码 -> } else {
        } else {
            // 中文逐行说明：下面这一行保留原始代码 -> _leftTurnControl->cancel();
            _leftTurnControl->cancel();
            // 中文逐行说明：下面这一行保留原始代码 -> _rightTurnControl->start();
            _rightTurnControl->start();
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }

        // 中文逐行说明：下面这一行保留原始代码 -> _decisionLockUntilMs = nowMs + TURN_LOCK_MS;
        _decisionLockUntilMs = nowMs + TURN_LOCK_MS;
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (leftValid && leftCm < SIDE_NEAR_CM && (!rightValid || rightCm > leftCm + SIDE_PREFERENCE_CM)) {
    if (leftValid && leftCm < SIDE_NEAR_CM && (!rightValid || rightCm > leftCm + SIDE_PREFERENCE_CM)) {
        // 中文逐行说明：下面这一行保留原始代码 -> _forwardControl->stop();
        _forwardControl->stop();
        // 中文逐行说明：下面这一行保留原始代码 -> _leftTurnControl->cancel();
        _leftTurnControl->cancel();
        // 中文逐行说明：下面这一行保留原始代码 -> _rightTurnControl->start();
        _rightTurnControl->start();
        // 中文逐行说明：下面这一行保留原始代码 -> _decisionLockUntilMs = nowMs + TURN_LOCK_MS;
        _decisionLockUntilMs = nowMs + TURN_LOCK_MS;
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (rightValid && rightCm < SIDE_NEAR_CM && (!leftValid || leftCm > rightCm + SIDE_PREFERENCE_CM)) {
    if (rightValid && rightCm < SIDE_NEAR_CM && (!leftValid || leftCm > rightCm + SIDE_PREFERENCE_CM)) {
        // 中文逐行说明：下面这一行保留原始代码 -> _forwardControl->stop();
        _forwardControl->stop();
        // 中文逐行说明：下面这一行保留原始代码 -> _rightTurnControl->cancel();
        _rightTurnControl->cancel();
        // 中文逐行说明：下面这一行保留原始代码 -> _leftTurnControl->start();
        _leftTurnControl->start();
        // 中文逐行说明：下面这一行保留原始代码 -> _decisionLockUntilMs = nowMs + TURN_LOCK_MS;
        _decisionLockUntilMs = nowMs + TURN_LOCK_MS;
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> _forwardControl->start();
    _forwardControl->start();
// 中文逐行说明：下面这一行保留原始代码 -> }
}
