/**********************************************************************
 * DepthController.cpp
 *
 * 这个文件实现定深和浮沉控制。
 * 深度控制只管理浮沉子系统，不和前进、转向子系统抢状态。
 *********************************************************************/

#include "DepthController.h"

namespace {
constexpr float DEADBAND_CM = 0.20f;
constexpr float MAX_DEPTH_CM = 100.0f;
constexpr float SPEED_LIMIT_CM_S = 0.8f;
constexpr float SYSTEM_TAU = 2.7f;
constexpr float CONTROL_TRIGGER = 15.0f;
constexpr uint32_t BUOYANCY_COOLDOWN_MS = 1700;
constexpr uint32_t ASCEND_DURATION_MS = 1500;
constexpr uint32_t DESCEND_DURATION_MS = 1500;
}

DepthController::DepthController()
    : _holdingTarget(false),
      _ascendActive(false),
      _descendActive(false),
      _targetDepthCm(0.0f),
      _depthFilt(0.0f),
      _speedCmS(0.0f),
      _integ(0.0f),
      _kpBase(120.0f),
      _kiBase(1.2f),
      _kdBase(150.0f),
      _kp(120.0f),
      _ki(1.2f),
      _kd(150.0f),
      _lastBuoyancyCmdMs(0),
      _ascendStartMs(0),
      _descendStartMs(0),
      _lastControlUpdateMs(0) {}

void DepthController::begin() {
    resetAfterCalibration();
}

void DepthController::update(bool depthValid, float filteredDepthCm, float depthSpeedCmS, uint32_t nowMs) {
    if (_ascendActive && nowMs - _ascendStartMs >= ASCEND_DURATION_MS) {
        _ascendActive = false;
    }

    if (_descendActive && nowMs - _descendStartMs >= DESCEND_DURATION_MS) {
        _descendActive = false;
    }

    if (!depthValid) {
        return;
    }

    _depthFilt = filteredDepthCm;
    _speedCmS = depthSpeedCmS;

    if (_lastControlUpdateMs == 0) {
        _lastControlUpdateMs = nowMs;
        return;
    }

    float dt = static_cast<float>(nowMs - _lastControlUpdateMs) * 0.001f;
    if (dt < 0.02f) {
        return;
    }
    if (dt > 0.20f) {
        dt = 0.20f;
    }
    _lastControlUpdateMs = nowMs;

    if (!_holdingTarget) {
        return;
    }

    if (_targetDepthCm < 0.0f || _depthFilt >= MAX_DEPTH_CM) {
        clearTarget();
        manualStop();
        return;
    }

    // 正在执行浮沉脉冲时，不重复下发新的浮沉动作。
    if (_ascendActive || _descendActive) {
        return;
    }

    const float err = _targetDepthCm - _depthFilt;
    adaptivePID(err, _speedCmS);

    const float pTerm = _kp * err;
    const float dTerm = _kd * (-_speedCmS);
    float u = pTerm + _ki * _integ + dTerm;

    speedLimiter(u, err);

    const float uFinal = constrain(u, -100.0f, 100.0f);
    const bool isSaturated =
        (uFinal >= 100.0f && err > 0.0f) ||
        (uFinal <= -100.0f && err < 0.0f);

    if (!isSaturated) {
        _integ += err * dt;
    }
    _integ = constrain(_integ, -100.0f, 100.0f);

    if (fabs(err) <= DEADBAND_CM || fabs(uFinal) < CONTROL_TRIGGER) {
        return;
    }

    if (nowMs - _lastBuoyancyCmdMs < BUOYANCY_COOLDOWN_MS) {
        return;
    }

    if (uFinal > 0.0f) {
        startDescendPulse(nowMs);
    } else {
        startAscendPulse(nowMs);
    }
}

void DepthController::setTargetDepth(float targetDepthCm) {
    _targetDepthCm = constrain(targetDepthCm, 0.0f, MAX_DEPTH_CM);
    _holdingTarget = true;
    _integ = 0.0f;
}

void DepthController::holdCurrentDepth() {
    _targetDepthCm = _depthFilt;
    _holdingTarget = true;
    _integ = 0.0f;
}

void DepthController::clearTarget() {
    _holdingTarget = false;
    _targetDepthCm = 0.0f;
    _integ = 0.0f;
}

bool DepthController::isHoldingTarget() const {
    return _holdingTarget;
}

void DepthController::manualAscend() {
    clearTarget();
    startAscendPulse(millis());
}

void DepthController::manualDescend() {
    clearTarget();
    startDescendPulse(millis());
}

void DepthController::manualStop() {
    _ascendActive = false;
    _descendActive = false;
    _ascendStartMs = 0;
    _descendStartMs = 0;
}

void DepthController::resetAfterCalibration() {
    _holdingTarget = false;
    _ascendActive = false;
    _descendActive = false;
    _targetDepthCm = 0.0f;
    _depthFilt = 0.0f;
    _speedCmS = 0.0f;
    _integ = 0.0f;
    _lastBuoyancyCmdMs = 0;
    _ascendStartMs = 0;
    _descendStartMs = 0;
    _lastControlUpdateMs = 0;
}

float DepthController::getFilteredDepthCm() const {
    return _depthFilt;
}

float DepthController::getSpeedCmS() const {
    return _speedCmS;
}

float DepthController::getTargetDepthCm() const {
    return _targetDepthCm;
}

uint16_t DepthController::getMask() const {
    if (_ascendActive) {
        return ACT_BUOYANCY_PUMP;
    }

    if (_descendActive) {
        return ACT_BUOYANCY_PUMP | ACT_BUOYANCY_VALVE_H | ACT_BUOYANCY_VALVE_I;
    }

    return 0;
}

void DepthController::adaptivePID(float err, float spd) {
    const float aErr = fabs(err);
    const float aSpd = fabs(spd);

    if (aErr > 15.0f) {
        _kp = _kpBase * 2.0f;
        _ki = _kiBase * 0.7f;
        _kd = _kdBase * 1.5f;
    } else if (aErr > 5.0f) {
        _kp = _kpBase * 1.2f;
        _ki = _kiBase * 1.0f;
        _kd = _kdBase * 1.2f;
    } else {
        _kp = _kpBase * 1.0f;
        _ki = _kiBase * 1.3f;
        _kd = _kdBase * 1.0f;
    }

    if (aSpd > 3.0f) {
        _kd *= 1.3f;
        _kp *= 0.8f;
    } else if (aSpd < 0.5f) {
        _kd *= 0.8f;
        _ki *= 1.1f;
    }

    if (spd > 0.0f && err > 0.0f) {
        _kp *= 1.7f;
        _kd *= 1.7f;
    }

    _kp = constrain(_kp, _kpBase * 0.3f, _kpBase * 2.5f);
    _ki = constrain(_ki, _kiBase * 0.2f, _kiBase * 2.0f);
    _kd = constrain(_kd, _kdBase * 0.3f, _kdBase * 3.0f);
}

void DepthController::speedLimiter(float& u, float err) {
    if (fabs(_speedCmS) > SPEED_LIMIT_CM_S) {
        if ((_speedCmS > 0.0f && u > 0.0f) || (_speedCmS < 0.0f && u < 0.0f)) {
            u *= 0.5f;
        }
    }

    float predict = max(fabs(_speedCmS) * SYSTEM_TAU, 0.5f * fabs(err));
    if (_speedCmS > 0.0f) {
        predict *= 2.0f;
    }
    predict = constrain(predict, 2.0f, 16.0f);

    if (fabs(err) < predict && fabs(_speedCmS) > 0.8f) {
        if (_speedCmS > 0.0f && err > 0.0f) {
            u = -fabs(u) * 2.0f;
        } else if (_speedCmS < 0.0f && err < 0.0f) {
            u = fabs(u) * 0.7f;
        } else if (_speedCmS > 0.0f && err < 0.0f) {
            u = -fabs(u) * 2.2f;
        } else if (_speedCmS < 0.0f && err > 0.0f) {
            u = fabs(u) * 1.2f;
        }
    }
}

void DepthController::startAscendPulse(uint32_t nowMs) {
    _descendActive = false;
    _ascendActive = true;
    _ascendStartMs = nowMs;
    _lastBuoyancyCmdMs = nowMs;
}

void DepthController::startDescendPulse(uint32_t nowMs) {
    _ascendActive = false;
    _descendActive = true;
    _descendStartMs = nowMs;
    _lastBuoyancyCmdMs = nowMs;
}
