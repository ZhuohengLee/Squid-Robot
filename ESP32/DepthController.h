/**********************************************************************
 * DepthController.h
 *
 * 这个文件声明深度滤波和深度控制模块。
 *********************************************************************/

#ifndef ESP32_DEPTH_CONTROLLER_H
#define ESP32_DEPTH_CONTROLLER_H

#include <Arduino.h>
#include "KalmanFilter.h"
#include "MotionLink.h"

class DepthController {
public:
    DepthController();

    void begin(MotionLink* motionLink);
    void update(bool depthValid, float rawDepthCm, uint32_t nowMs);

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
    float getTargetDepthCm() const;

private:
    void adaptivePID(float err, float spd);
    void speedLimiter(float& u, float err);

    MotionLink* _motionLink;
    KalmanFilter _kalman;

    bool _holdingTarget;
    float _targetDepthCm;
    float _depthFilt;
    float _speedCmS;
    float _integ;

    float _kpBase;
    float _kiBase;
    float _kdBase;
    float _kp;
    float _ki;
    float _kd;

    uint32_t _lastUpdateMs;
    uint32_t _lastBuoyancyCmdMs;
};

#endif // ESP32_DEPTH_CONTROLLER_H
