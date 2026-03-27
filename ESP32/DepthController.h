/**********************************************************************
 * DepthController.h
 *
 * Depth hold controller using filtered depth state and continuous buoyancy
 * commands.
 *********************************************************************/

#ifndef ESP32_DEPTH_CONTROLLER_H
#define ESP32_DEPTH_CONTROLLER_H

#include <Arduino.h>
#include "Protocol.h"

class DepthController {
public:
    DepthController();

    void begin();
    void update(bool depthValid,
                float filteredDepthCm,
                float depthSpeedCmS,
                float depthAccelCmS2,
                uint32_t nowMs);

    void setTargetDepth(float targetDepthCm);
    void holdCurrentDepth();
    void clearTarget();
    bool isHoldingTarget() const;

    void manualAscend();
    void manualDescend();
    void manualStop();

    void resetAfterCalibration();

    float getFilteredDepthCm() const;
    float getSpeedCmS() const;
    float getAccelerationCmS2() const;
    float getTargetDepthCm() const;
    float getControlOutput() const;

    uint8_t getBuoyancyDirection() const;
    uint8_t getBuoyancyPwm() const;

private:
    void adaptivePID(float err, float derr, float spd, float dt);
    void speedLimiter(float& u, float err);
    void applyBuoyancyOutput(float u);
    void stopBuoyancyOutput();

    bool _holdingTarget;

    float _targetDepthCm;
    float _depthFilt;
    float _speedCmS;
    float _accelCmS2;
    float _integ;
    float _errPrev;
    float _derivPrev;
    float _controlOutput;

    float _kpBase;
    float _kiBase;
    float _kdBase;
    float _kp;
    float _ki;
    float _kd;

    uint8_t _manualDirection;
    uint8_t _buoyancyDirection;
    uint8_t _buoyancyPwm;
    uint32_t _lastControlUpdateMs;
    uint32_t _manualStartMs;
};

#endif // ESP32_DEPTH_CONTROLLER_H
