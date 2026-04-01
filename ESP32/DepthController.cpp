/**********************************************************************
 * DepthController.cpp
 *
 * Predictive depth control using filtered depth, speed, and acceleration.
 *********************************************************************/

#include "DepthController.h"

namespace {
constexpr float DEADBAND_CM = 0.20f;
constexpr float MAX_DEPTH_CM = 100.0f;
constexpr float SPEED_LIMIT_CM_S = 0.8f;
constexpr float SYSTEM_TAU = 2.7f;
constexpr float CONTROL_TRIGGER = 8.0f;
constexpr uint8_t PUMP_PWM_MIN = 80;
constexpr uint8_t PUMP_PWM_MAX = 255;
constexpr uint8_t MANUAL_PWM = 255;
constexpr uint32_t MANUAL_DURATION_MS = 1500;
}

DepthController::DepthController()
    : _holdingTarget(false),
      _targetDepthCm(0.0f),
      _depthFilt(0.0f),
      _speedCmS(0.0f),
      _accelCmS2(0.0f),
      _integ(0.0f),
      _errPrev(0.0f),
      _derivPrev(0.0f),
      _controlOutput(0.0f),
      _kpBase(120.0f),
      _kiBase(1.2f),
      _kdBase(150.0f),
      _kp(120.0f),
      _ki(1.2f),
      _kd(150.0f),
      _manualDirection(BUOYANCY_STOP),
      _buoyancyDirection(BUOYANCY_STOP),
      _buoyancyPwm(0),
      _lastControlUpdateMs(0),
      _manualStartMs(0) {}

void DepthController::begin() {
    resetAfterCalibration();
}

void DepthController::update(bool depthValid,
                             float filteredDepthCm,
                             float depthSpeedCmS,
                             float depthAccelCmS2,
                             uint32_t nowMs) {
    if (_manualDirection != BUOYANCY_STOP) {
        if (nowMs - _manualStartMs >= MANUAL_DURATION_MS) {
            manualStop();
            return;
        }
        _controlOutput = _manualDirection == BUOYANCY_DESCEND ? 100.0f : -100.0f;
        _buoyancyDirection = _manualDirection;
        _buoyancyPwm = MANUAL_PWM;
        return;
    }

    if (!depthValid) {
        stopBuoyancyOutput();
        return;
    }

    _depthFilt = filteredDepthCm;
    _speedCmS = depthSpeedCmS;
    _accelCmS2 = depthAccelCmS2;

    if (_lastControlUpdateMs == 0) {
        _lastControlUpdateMs = nowMs;
        _errPrev = _targetDepthCm - _depthFilt;
        stopBuoyancyOutput();
        return;
    }

    float dt = static_cast<float>(nowMs - _lastControlUpdateMs) * 0.001f;
    if (dt < 0.01f) {
        return;
    }
    if (dt > 0.20f) {
        dt = 0.20f;
    }
    _lastControlUpdateMs = nowMs;

    if (!_holdingTarget) {
        _errPrev = _targetDepthCm - _depthFilt;
        _derivPrev = 0.0f;
        stopBuoyancyOutput();
        return;
    }

    if (_targetDepthCm < 0.0f || _depthFilt >= MAX_DEPTH_CM) {
        clearTarget();
        manualStop();
        stopBuoyancyOutput();
        return;
    }

    const float err = _targetDepthCm - _depthFilt;
    const float derr = (err - _errPrev) / dt;
    adaptivePID(err, derr, _speedCmS, dt);

    float u = _kp * err + _ki * _integ + _kd * _derivPrev;
    speedLimiter(u, err);

    const float uFinal = constrain(u, -100.0f, 100.0f);
    const bool isSaturated =
        (uFinal >= 100.0f && err > 0.0f) ||
        (uFinal <= -100.0f && err < 0.0f);

    if (!isSaturated) {
        _integ += err * dt;
    }
    _integ = constrain(_integ, -100.0f, 100.0f);
    _errPrev = err;
    _controlOutput = uFinal;

    if (fabs(err) <= DEADBAND_CM || fabs(uFinal) < CONTROL_TRIGGER) {
        stopBuoyancyOutput();
        return;
    }

    applyBuoyancyOutput(uFinal);
}

void DepthController::setTargetDepth(float targetDepthCm) {
    _targetDepthCm = constrain(targetDepthCm, 0.0f, MAX_DEPTH_CM);
    _holdingTarget = true;
    _integ = 0.0f;
    _manualDirection = BUOYANCY_STOP;
}

void DepthController::holdCurrentDepth() {
    _targetDepthCm = _depthFilt;
    _holdingTarget = true;
    _integ = 0.0f;
    _manualDirection = BUOYANCY_STOP;
}

void DepthController::clearTarget() {
    _holdingTarget = false;
    _targetDepthCm = 0.0f;
    _integ = 0.0f;
    _controlOutput = 0.0f;
}

bool DepthController::isHoldingTarget() const {
    return _holdingTarget;
}

void DepthController::manualAscend() {
    clearTarget();
    _manualDirection = BUOYANCY_ASCEND;
    _manualStartMs = millis();
    _controlOutput = -100.0f;
    _buoyancyDirection = BUOYANCY_ASCEND;
    _buoyancyPwm = MANUAL_PWM;
}

void DepthController::manualDescend() {
    clearTarget();
    _manualDirection = BUOYANCY_DESCEND;
    _manualStartMs = millis();
    _controlOutput = 100.0f;
    _buoyancyDirection = BUOYANCY_DESCEND;
    _buoyancyPwm = MANUAL_PWM;
}

void DepthController::manualStop() {
    _manualDirection = BUOYANCY_STOP;
    stopBuoyancyOutput();
}

void DepthController::resetAfterCalibration() {
    _holdingTarget = false;
    _targetDepthCm = 0.0f;
    _depthFilt = 0.0f;
    _speedCmS = 0.0f;
    _accelCmS2 = 0.0f;
    _integ = 0.0f;
    _errPrev = 0.0f;
    _derivPrev = 0.0f;
    _controlOutput = 0.0f;
    _manualDirection = BUOYANCY_STOP;
    _buoyancyDirection = BUOYANCY_STOP;
    _buoyancyPwm = 0;
    _lastControlUpdateMs = 0;
    _manualStartMs = 0;
}

float DepthController::getFilteredDepthCm() const {
    return _depthFilt;
}

float DepthController::getSpeedCmS() const {
    return _speedCmS;
}

float DepthController::getAccelerationCmS2() const {
    return _accelCmS2;
}

float DepthController::getTargetDepthCm() const {
    return _targetDepthCm;
}

float DepthController::getControlOutput() const {
    return _controlOutput;
}

uint8_t DepthController::getBuoyancyDirection() const {
    return _buoyancyDirection;
}

uint8_t DepthController::getBuoyancyPwm() const {
    return _buoyancyPwm;
}

void DepthController::adaptivePID(float err, float derr, float spd, float dt) {
    const float aErr = fabs(err);
    const float aRate = fabs(derr);
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

    if (aRate > 2.0f) {
        _kd *= 1.3f;
    } else if (aRate < 0.5f) {
        _kd *= 0.8f;
        _ki *= 1.1f;
    }

    if (aSpd > 3.0f) {
        _kd *= 1.2f;
        _kp *= 0.8f;
    } else if (aSpd < 0.5f && aErr < 3.0f) {
        _ki *= 1.3f;
        _kp *= 0.9f;
    }

    if (spd > 0.0f && err > 0.0f) {
        _kp *= 1.7f;
        _kd *= 1.7f;
    }

    _kp = constrain(_kp, _kpBase * 0.3f, _kpBase * 2.5f);
    _ki = constrain(_ki, _kiBase * 0.2f, _kiBase * 2.0f);
    _kd = constrain(_kd, _kdBase * 0.3f, _kdBase * 3.0f);

    const float derivAlpha = constrain(dt * 4.0f, 0.05f, 0.35f);
    _derivPrev += (derr - _derivPrev) * derivAlpha;
}

void DepthController::speedLimiter(float& u, float err) {
    if (fabs(_speedCmS) > SPEED_LIMIT_CM_S) {
        if ((_speedCmS > 0.0f && u > 0.0f) || (_speedCmS < 0.0f && u < 0.0f)) {
            u *= 0.5f;
        }
    }

    const float kinematicPredict =
        _speedCmS * SYSTEM_TAU +
        0.5f * _accelCmS2 * SYSTEM_TAU * SYSTEM_TAU;

    float originalPredict = max(fabs(_speedCmS) * SYSTEM_TAU, 0.5f * fabs(err));
    if (_speedCmS > 0.0f) {
        originalPredict *= 2.0f;
    }

    float finalPredict = max(fabs(kinematicPredict), originalPredict);
    finalPredict = constrain(finalPredict, 2.0f, 20.0f);

    if (fabs(err) < finalPredict && fabs(_speedCmS) > 0.8f) {
        const float accelFactor = 1.0f + constrain(fabs(_accelCmS2) * 0.5f, 0.0f, 1.0f);

        if (_speedCmS > 0.0f && err > 0.0f) {
            u = -fabs(u) * 2.0f * accelFactor;
        } else if (_speedCmS < 0.0f && err < 0.0f) {
            u = fabs(u) * 1.2f;
        } else if (_speedCmS > 0.0f && err < 0.0f) {
            u = -fabs(u) * 2.5f * accelFactor;
        } else if (_speedCmS < 0.0f && err > 0.0f) {
            u = fabs(u) * 1.2f;
        }
    }
}

void DepthController::applyBuoyancyOutput(float u) {
    const float magnitude = constrain(fabs(u), CONTROL_TRIGGER, 100.0f);
    const float span = max(1.0f, 100.0f - CONTROL_TRIGGER);
    const float normalized = (magnitude - CONTROL_TRIGGER) / span;
    const float pwm = PUMP_PWM_MIN + normalized * (PUMP_PWM_MAX - PUMP_PWM_MIN);

    _buoyancyDirection = u > 0.0f ? BUOYANCY_DESCEND : BUOYANCY_ASCEND;
    _buoyancyPwm = static_cast<uint8_t>(
        constrain(static_cast<int>(pwm + 0.5f),
                  static_cast<int>(PUMP_PWM_MIN),
                  static_cast<int>(PUMP_PWM_MAX))
    );
}

void DepthController::stopBuoyancyOutput() {
    _controlOutput = 0.0f;
    _buoyancyDirection = BUOYANCY_STOP;
    _buoyancyPwm = 0;
}
