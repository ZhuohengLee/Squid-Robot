/**********************************************************************
 * DepthController.cpp
 *
 * 杩欎釜鏂囦欢瀹炵幇瀹氭繁鍜屾诞娌夋帶鍒躲€? * 娣卞害鎺у埗鍙鐞嗘诞娌夊瓙绯荤粺锛屼笉鍜屽墠杩涖€佽浆鍚戝瓙绯荤粺鎶㈢姸鎬併€? *********************************************************************/

// 中文逐行说明：下面这一行保留原始代码 -> #include "DepthController.h"
#include "DepthController.h"

// 中文逐行说明：下面这一行保留原始代码 -> namespace {
namespace {
// 中文逐行说明：下面这一行保留原始代码 -> constexpr float DEADBAND_CM = 0.20f;
constexpr float DEADBAND_CM = 0.20f;
// 中文逐行说明：下面这一行保留原始代码 -> constexpr float MAX_DEPTH_CM = 100.0f;
constexpr float MAX_DEPTH_CM = 100.0f;
// 中文逐行说明：下面这一行保留原始代码 -> constexpr float SPEED_LIMIT_CM_S = 0.8f;
constexpr float SPEED_LIMIT_CM_S = 0.8f;
// 中文逐行说明：下面这一行保留原始代码 -> constexpr float SYSTEM_TAU = 2.7f;
constexpr float SYSTEM_TAU = 2.7f;
// 中文逐行说明：下面这一行保留原始代码 -> constexpr float CONTROL_TRIGGER = 15.0f;
constexpr float CONTROL_TRIGGER = 15.0f;
// 中文逐行说明：下面这一行保留原始代码 -> constexpr uint32_t BUOYANCY_COOLDOWN_MS = 1700;
constexpr uint32_t BUOYANCY_COOLDOWN_MS = 1700;
// 中文逐行说明：下面这一行保留原始代码 -> constexpr uint32_t ASCEND_DURATION_MS = 1500;
constexpr uint32_t ASCEND_DURATION_MS = 1500;
// 中文逐行说明：下面这一行保留原始代码 -> constexpr uint32_t DESCEND_DURATION_MS = 1500;
constexpr uint32_t DESCEND_DURATION_MS = 1500;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> DepthController::DepthController()
DepthController::DepthController()
    // 中文逐行说明：下面这一行保留原始代码 -> : _holdingTarget(false),
    : _holdingTarget(false),
      // 中文逐行说明：下面这一行保留原始代码 -> _ascendActive(false),
      _ascendActive(false),
      // 中文逐行说明：下面这一行保留原始代码 -> _descendActive(false),
      _descendActive(false),
      // 中文逐行说明：下面这一行保留原始代码 -> _targetDepthCm(0.0f),
      _targetDepthCm(0.0f),
      // 中文逐行说明：下面这一行保留原始代码 -> _depthFilt(0.0f),
      _depthFilt(0.0f),
      // 中文逐行说明：下面这一行保留原始代码 -> _speedCmS(0.0f),
      _speedCmS(0.0f),
      // 中文逐行说明：下面这一行保留原始代码 -> _integ(0.0f),
      _integ(0.0f),
      // 中文逐行说明：下面这一行保留原始代码 -> _kpBase(120.0f),
      _kpBase(120.0f),
      // 中文逐行说明：下面这一行保留原始代码 -> _kiBase(1.2f),
      _kiBase(1.2f),
      // 中文逐行说明：下面这一行保留原始代码 -> _kdBase(150.0f),
      _kdBase(150.0f),
      // 中文逐行说明：下面这一行保留原始代码 -> _kp(120.0f),
      _kp(120.0f),
      // 中文逐行说明：下面这一行保留原始代码 -> _ki(1.2f),
      _ki(1.2f),
      // 中文逐行说明：下面这一行保留原始代码 -> _kd(150.0f),
      _kd(150.0f),
      // 中文逐行说明：下面这一行保留原始代码 -> _lastBuoyancyCmdMs(0),
      _lastBuoyancyCmdMs(0),
      // 中文逐行说明：下面这一行保留原始代码 -> _ascendStartMs(0),
      _ascendStartMs(0),
      // 中文逐行说明：下面这一行保留原始代码 -> _descendStartMs(0),
      _descendStartMs(0),
      // 中文逐行说明：下面这一行保留原始代码 -> _lastControlUpdateMs(0) {}
      _lastControlUpdateMs(0) {}

// 中文逐行说明：下面这一行保留原始代码 -> void DepthController::begin() {
void DepthController::begin() {
    // 中文逐行说明：下面这一行保留原始代码 -> resetAfterCalibration();
    resetAfterCalibration();
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void DepthController::update(bool depthValid, float filteredDepthCm, float depthSpeedCmS, uint32_t nowMs) {
void DepthController::update(bool depthValid, float filteredDepthCm, float depthSpeedCmS, uint32_t nowMs) {
    // 中文逐行说明：下面这一行保留原始代码 -> if (_ascendActive && nowMs - _ascendStartMs >= ASCEND_DURATION_MS) {
    if (_ascendActive && nowMs - _ascendStartMs >= ASCEND_DURATION_MS) {
        // 中文逐行说明：下面这一行保留原始代码 -> _ascendActive = false;
        _ascendActive = false;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (_descendActive && nowMs - _descendStartMs >= DESCEND_DURATION_MS) {
    if (_descendActive && nowMs - _descendStartMs >= DESCEND_DURATION_MS) {
        // 中文逐行说明：下面这一行保留原始代码 -> _descendActive = false;
        _descendActive = false;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (!depthValid) {
    if (!depthValid) {
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> _depthFilt = filteredDepthCm;
    _depthFilt = filteredDepthCm;
    // 中文逐行说明：下面这一行保留原始代码 -> _speedCmS = depthSpeedCmS;
    _speedCmS = depthSpeedCmS;

    // 中文逐行说明：下面这一行保留原始代码 -> if (_lastControlUpdateMs == 0) {
    if (_lastControlUpdateMs == 0) {
        // 中文逐行说明：下面这一行保留原始代码 -> _lastControlUpdateMs = nowMs;
        _lastControlUpdateMs = nowMs;
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> float dt = static_cast<float>(nowMs - _lastControlUpdateMs) * 0.001f;
    float dt = static_cast<float>(nowMs - _lastControlUpdateMs) * 0.001f;
    // 中文逐行说明：下面这一行保留原始代码 -> if (dt < 0.02f) {
    if (dt < 0.02f) {
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
    // 中文逐行说明：下面这一行保留原始代码 -> if (dt > 0.20f) {
    if (dt > 0.20f) {
        // 中文逐行说明：下面这一行保留原始代码 -> dt = 0.20f;
        dt = 0.20f;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
    // 中文逐行说明：下面这一行保留原始代码 -> _lastControlUpdateMs = nowMs;
    _lastControlUpdateMs = nowMs;

    // 中文逐行说明：下面这一行保留原始代码 -> if (!_holdingTarget) {
    if (!_holdingTarget) {
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (_targetDepthCm < 0.0f || _depthFilt >= MAX_DEPTH_CM) {
    if (_targetDepthCm < 0.0f || _depthFilt >= MAX_DEPTH_CM) {
        // 中文逐行说明：下面这一行保留原始代码 -> clearTarget();
        clearTarget();
        // 中文逐行说明：下面这一行保留原始代码 -> manualStop();
        manualStop();
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 姝ｅ湪鎵ц娴矇鑴夊啿鏃讹紝涓嶉噸澶嶄笅鍙戞柊鐨勬诞娌夊姩浣溿€?    if (_ascendActive || _descendActive) {
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> const float err = _targetDepthCm - _depthFilt;
    const float err = _targetDepthCm - _depthFilt;
    // 中文逐行说明：下面这一行保留原始代码 -> adaptivePID(err, _speedCmS);
    adaptivePID(err, _speedCmS);

    // 中文逐行说明：下面这一行保留原始代码 -> const float pTerm = _kp * err;
    const float pTerm = _kp * err;
    // 中文逐行说明：下面这一行保留原始代码 -> const float dTerm = _kd * (-_speedCmS);
    const float dTerm = _kd * (-_speedCmS);
    // 中文逐行说明：下面这一行保留原始代码 -> float u = pTerm + _ki * _integ + dTerm;
    float u = pTerm + _ki * _integ + dTerm;

    // 中文逐行说明：下面这一行保留原始代码 -> speedLimiter(u, err);
    speedLimiter(u, err);

    // 中文逐行说明：下面这一行保留原始代码 -> const float uFinal = constrain(u, -100.0f, 100.0f);
    const float uFinal = constrain(u, -100.0f, 100.0f);
    // 中文逐行说明：下面这一行保留原始代码 -> const bool isSaturated =
    const bool isSaturated =
        // 中文逐行说明：下面这一行保留原始代码 -> (uFinal >= 100.0f && err > 0.0f) ||
        (uFinal >= 100.0f && err > 0.0f) ||
        // 中文逐行说明：下面这一行保留原始代码 -> (uFinal <= -100.0f && err < 0.0f);
        (uFinal <= -100.0f && err < 0.0f);

    // 中文逐行说明：下面这一行保留原始代码 -> if (!isSaturated) {
    if (!isSaturated) {
        // 中文逐行说明：下面这一行保留原始代码 -> _integ += err * dt;
        _integ += err * dt;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
    // 中文逐行说明：下面这一行保留原始代码 -> _integ = constrain(_integ, -100.0f, 100.0f);
    _integ = constrain(_integ, -100.0f, 100.0f);

    // 中文逐行说明：下面这一行保留原始代码 -> if (fabs(err) <= DEADBAND_CM || fabs(uFinal) < CONTROL_TRIGGER) {
    if (fabs(err) <= DEADBAND_CM || fabs(uFinal) < CONTROL_TRIGGER) {
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (nowMs - _lastBuoyancyCmdMs < BUOYANCY_COOLDOWN_MS) {
    if (nowMs - _lastBuoyancyCmdMs < BUOYANCY_COOLDOWN_MS) {
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (uFinal > 0.0f) {
    if (uFinal > 0.0f) {
        // 中文逐行说明：下面这一行保留原始代码 -> startDescendPulse(nowMs);
        startDescendPulse(nowMs);
    // 中文逐行说明：下面这一行保留原始代码 -> } else {
    } else {
        // 中文逐行说明：下面这一行保留原始代码 -> startAscendPulse(nowMs);
        startAscendPulse(nowMs);
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void DepthController::setTargetDepth(float targetDepthCm) {
void DepthController::setTargetDepth(float targetDepthCm) {
    // 中文逐行说明：下面这一行保留原始代码 -> _targetDepthCm = constrain(targetDepthCm, 0.0f, MAX_DEPTH_CM);
    _targetDepthCm = constrain(targetDepthCm, 0.0f, MAX_DEPTH_CM);
    // 中文逐行说明：下面这一行保留原始代码 -> _holdingTarget = true;
    _holdingTarget = true;
    // 中文逐行说明：下面这一行保留原始代码 -> _integ = 0.0f;
    _integ = 0.0f;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void DepthController::holdCurrentDepth() {
void DepthController::holdCurrentDepth() {
    // 中文逐行说明：下面这一行保留原始代码 -> _targetDepthCm = _depthFilt;
    _targetDepthCm = _depthFilt;
    // 中文逐行说明：下面这一行保留原始代码 -> _holdingTarget = true;
    _holdingTarget = true;
    // 中文逐行说明：下面这一行保留原始代码 -> _integ = 0.0f;
    _integ = 0.0f;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void DepthController::clearTarget() {
void DepthController::clearTarget() {
    // 中文逐行说明：下面这一行保留原始代码 -> _holdingTarget = false;
    _holdingTarget = false;
    // 中文逐行说明：下面这一行保留原始代码 -> _targetDepthCm = 0.0f;
    _targetDepthCm = 0.0f;
    // 中文逐行说明：下面这一行保留原始代码 -> _integ = 0.0f;
    _integ = 0.0f;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> bool DepthController::isHoldingTarget() const {
bool DepthController::isHoldingTarget() const {
    // 中文逐行说明：下面这一行保留原始代码 -> return _holdingTarget;
    return _holdingTarget;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void DepthController::manualAscend() {
void DepthController::manualAscend() {
    // 中文逐行说明：下面这一行保留原始代码 -> clearTarget();
    clearTarget();
    // 中文逐行说明：下面这一行保留原始代码 -> startAscendPulse(millis());
    startAscendPulse(millis());
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void DepthController::manualDescend() {
void DepthController::manualDescend() {
    // 中文逐行说明：下面这一行保留原始代码 -> clearTarget();
    clearTarget();
    // 中文逐行说明：下面这一行保留原始代码 -> startDescendPulse(millis());
    startDescendPulse(millis());
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void DepthController::manualStop() {
void DepthController::manualStop() {
    // 中文逐行说明：下面这一行保留原始代码 -> _ascendActive = false;
    _ascendActive = false;
    // 中文逐行说明：下面这一行保留原始代码 -> _descendActive = false;
    _descendActive = false;
    // 中文逐行说明：下面这一行保留原始代码 -> _ascendStartMs = 0;
    _ascendStartMs = 0;
    // 中文逐行说明：下面这一行保留原始代码 -> _descendStartMs = 0;
    _descendStartMs = 0;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void DepthController::resetAfterCalibration() {
void DepthController::resetAfterCalibration() {
    // 中文逐行说明：下面这一行保留原始代码 -> _holdingTarget = false;
    _holdingTarget = false;
    // 中文逐行说明：下面这一行保留原始代码 -> _ascendActive = false;
    _ascendActive = false;
    // 中文逐行说明：下面这一行保留原始代码 -> _descendActive = false;
    _descendActive = false;
    // 中文逐行说明：下面这一行保留原始代码 -> _targetDepthCm = 0.0f;
    _targetDepthCm = 0.0f;
    // 中文逐行说明：下面这一行保留原始代码 -> _depthFilt = 0.0f;
    _depthFilt = 0.0f;
    // 中文逐行说明：下面这一行保留原始代码 -> _speedCmS = 0.0f;
    _speedCmS = 0.0f;
    // 中文逐行说明：下面这一行保留原始代码 -> _integ = 0.0f;
    _integ = 0.0f;
    // 中文逐行说明：下面这一行保留原始代码 -> _lastBuoyancyCmdMs = 0;
    _lastBuoyancyCmdMs = 0;
    // 中文逐行说明：下面这一行保留原始代码 -> _ascendStartMs = 0;
    _ascendStartMs = 0;
    // 中文逐行说明：下面这一行保留原始代码 -> _descendStartMs = 0;
    _descendStartMs = 0;
    // 中文逐行说明：下面这一行保留原始代码 -> _lastControlUpdateMs = 0;
    _lastControlUpdateMs = 0;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> float DepthController::getFilteredDepthCm() const {
float DepthController::getFilteredDepthCm() const {
    // 中文逐行说明：下面这一行保留原始代码 -> return _depthFilt;
    return _depthFilt;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> float DepthController::getSpeedCmS() const {
float DepthController::getSpeedCmS() const {
    // 中文逐行说明：下面这一行保留原始代码 -> return _speedCmS;
    return _speedCmS;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> float DepthController::getTargetDepthCm() const {
float DepthController::getTargetDepthCm() const {
    // 中文逐行说明：下面这一行保留原始代码 -> return _targetDepthCm;
    return _targetDepthCm;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> uint16_t DepthController::getMask() const {
uint16_t DepthController::getMask() const {
    // 中文逐行说明：下面这一行保留原始代码 -> if (_ascendActive) {
    if (_ascendActive) {
        // 中文逐行说明：下面这一行保留原始代码 -> return ACT_BUOYANCY_PUMP;
        return ACT_BUOYANCY_PUMP;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (_descendActive) {
    if (_descendActive) {
        // 中文逐行说明：下面这一行保留原始代码 -> return ACT_BUOYANCY_PUMP | ACT_BUOYANCY_VALVE_H | ACT_BUOYANCY_VALVE_I;
        return ACT_BUOYANCY_PUMP | ACT_BUOYANCY_VALVE_H | ACT_BUOYANCY_VALVE_I;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> return 0;
    return 0;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void DepthController::adaptivePID(float err, float spd) {
void DepthController::adaptivePID(float err, float spd) {
    // 中文逐行说明：下面这一行保留原始代码 -> const float aErr = fabs(err);
    const float aErr = fabs(err);
    // 中文逐行说明：下面这一行保留原始代码 -> const float aSpd = fabs(spd);
    const float aSpd = fabs(spd);

    // 中文逐行说明：下面这一行保留原始代码 -> if (aErr > 15.0f) {
    if (aErr > 15.0f) {
        // 中文逐行说明：下面这一行保留原始代码 -> _kp = _kpBase * 2.0f;
        _kp = _kpBase * 2.0f;
        // 中文逐行说明：下面这一行保留原始代码 -> _ki = _kiBase * 0.7f;
        _ki = _kiBase * 0.7f;
        // 中文逐行说明：下面这一行保留原始代码 -> _kd = _kdBase * 1.5f;
        _kd = _kdBase * 1.5f;
    // 中文逐行说明：下面这一行保留原始代码 -> } else if (aErr > 5.0f) {
    } else if (aErr > 5.0f) {
        // 中文逐行说明：下面这一行保留原始代码 -> _kp = _kpBase * 1.2f;
        _kp = _kpBase * 1.2f;
        // 中文逐行说明：下面这一行保留原始代码 -> _ki = _kiBase * 1.0f;
        _ki = _kiBase * 1.0f;
        // 中文逐行说明：下面这一行保留原始代码 -> _kd = _kdBase * 1.2f;
        _kd = _kdBase * 1.2f;
    // 中文逐行说明：下面这一行保留原始代码 -> } else {
    } else {
        // 中文逐行说明：下面这一行保留原始代码 -> _kp = _kpBase * 1.0f;
        _kp = _kpBase * 1.0f;
        // 中文逐行说明：下面这一行保留原始代码 -> _ki = _kiBase * 1.3f;
        _ki = _kiBase * 1.3f;
        // 中文逐行说明：下面这一行保留原始代码 -> _kd = _kdBase * 1.0f;
        _kd = _kdBase * 1.0f;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (aSpd > 3.0f) {
    if (aSpd > 3.0f) {
        // 中文逐行说明：下面这一行保留原始代码 -> _kd *= 1.3f;
        _kd *= 1.3f;
        // 中文逐行说明：下面这一行保留原始代码 -> _kp *= 0.8f;
        _kp *= 0.8f;
    // 中文逐行说明：下面这一行保留原始代码 -> } else if (aSpd < 0.5f) {
    } else if (aSpd < 0.5f) {
        // 中文逐行说明：下面这一行保留原始代码 -> _kd *= 0.8f;
        _kd *= 0.8f;
        // 中文逐行说明：下面这一行保留原始代码 -> _ki *= 1.1f;
        _ki *= 1.1f;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (spd > 0.0f && err > 0.0f) {
    if (spd > 0.0f && err > 0.0f) {
        // 中文逐行说明：下面这一行保留原始代码 -> _kp *= 1.7f;
        _kp *= 1.7f;
        // 中文逐行说明：下面这一行保留原始代码 -> _kd *= 1.7f;
        _kd *= 1.7f;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> _kp = constrain(_kp, _kpBase * 0.3f, _kpBase * 2.5f);
    _kp = constrain(_kp, _kpBase * 0.3f, _kpBase * 2.5f);
    // 中文逐行说明：下面这一行保留原始代码 -> _ki = constrain(_ki, _kiBase * 0.2f, _kiBase * 2.0f);
    _ki = constrain(_ki, _kiBase * 0.2f, _kiBase * 2.0f);
    // 中文逐行说明：下面这一行保留原始代码 -> _kd = constrain(_kd, _kdBase * 0.3f, _kdBase * 3.0f);
    _kd = constrain(_kd, _kdBase * 0.3f, _kdBase * 3.0f);
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void DepthController::speedLimiter(float& u, float err) {
void DepthController::speedLimiter(float& u, float err) {
    // 中文逐行说明：下面这一行保留原始代码 -> if (fabs(_speedCmS) > SPEED_LIMIT_CM_S) {
    if (fabs(_speedCmS) > SPEED_LIMIT_CM_S) {
        // 中文逐行说明：下面这一行保留原始代码 -> if ((_speedCmS > 0.0f && u > 0.0f) || (_speedCmS < 0.0f && u < 0.0f)) {
        if ((_speedCmS > 0.0f && u > 0.0f) || (_speedCmS < 0.0f && u < 0.0f)) {
            // 中文逐行说明：下面这一行保留原始代码 -> u *= 0.5f;
            u *= 0.5f;
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> float predict = max(fabs(_speedCmS) * SYSTEM_TAU, 0.5f * fabs(err));
    float predict = max(fabs(_speedCmS) * SYSTEM_TAU, 0.5f * fabs(err));
    // 中文逐行说明：下面这一行保留原始代码 -> if (_speedCmS > 0.0f) {
    if (_speedCmS > 0.0f) {
        // 中文逐行说明：下面这一行保留原始代码 -> predict *= 2.0f;
        predict *= 2.0f;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
    // 中文逐行说明：下面这一行保留原始代码 -> predict = constrain(predict, 2.0f, 16.0f);
    predict = constrain(predict, 2.0f, 16.0f);

    // 中文逐行说明：下面这一行保留原始代码 -> if (fabs(err) < predict && fabs(_speedCmS) > 0.8f) {
    if (fabs(err) < predict && fabs(_speedCmS) > 0.8f) {
        // 中文逐行说明：下面这一行保留原始代码 -> if (_speedCmS > 0.0f && err > 0.0f) {
        if (_speedCmS > 0.0f && err > 0.0f) {
            // 中文逐行说明：下面这一行保留原始代码 -> u = -fabs(u) * 2.0f;
            u = -fabs(u) * 2.0f;
        // 中文逐行说明：下面这一行保留原始代码 -> } else if (_speedCmS < 0.0f && err < 0.0f) {
        } else if (_speedCmS < 0.0f && err < 0.0f) {
            // 中文逐行说明：下面这一行保留原始代码 -> u = fabs(u) * 0.7f;
            u = fabs(u) * 0.7f;
        // 中文逐行说明：下面这一行保留原始代码 -> } else if (_speedCmS > 0.0f && err < 0.0f) {
        } else if (_speedCmS > 0.0f && err < 0.0f) {
            // 中文逐行说明：下面这一行保留原始代码 -> u = -fabs(u) * 2.2f;
            u = -fabs(u) * 2.2f;
        // 中文逐行说明：下面这一行保留原始代码 -> } else if (_speedCmS < 0.0f && err > 0.0f) {
        } else if (_speedCmS < 0.0f && err > 0.0f) {
            // 中文逐行说明：下面这一行保留原始代码 -> u = fabs(u) * 1.2f;
            u = fabs(u) * 1.2f;
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void DepthController::startAscendPulse(uint32_t nowMs) {
void DepthController::startAscendPulse(uint32_t nowMs) {
    // 中文逐行说明：下面这一行保留原始代码 -> _descendActive = false;
    _descendActive = false;
    // 中文逐行说明：下面这一行保留原始代码 -> _ascendActive = true;
    _ascendActive = true;
    // 中文逐行说明：下面这一行保留原始代码 -> _ascendStartMs = nowMs;
    _ascendStartMs = nowMs;
    // 中文逐行说明：下面这一行保留原始代码 -> _lastBuoyancyCmdMs = nowMs;
    _lastBuoyancyCmdMs = nowMs;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void DepthController::startDescendPulse(uint32_t nowMs) {
void DepthController::startDescendPulse(uint32_t nowMs) {
    // 中文逐行说明：下面这一行保留原始代码 -> _ascendActive = false;
    _ascendActive = false;
    // 中文逐行说明：下面这一行保留原始代码 -> _descendActive = true;
    _descendActive = true;
    // 中文逐行说明：下面这一行保留原始代码 -> _descendStartMs = nowMs;
    _descendStartMs = nowMs;
    // 中文逐行说明：下面这一行保留原始代码 -> _lastBuoyancyCmdMs = nowMs;
    _lastBuoyancyCmdMs = nowMs;
// 中文逐行说明：下面这一行保留原始代码 -> }
}
