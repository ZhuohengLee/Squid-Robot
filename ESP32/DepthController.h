/**********************************************************************
 * DepthController.h
 *
 * 这个文件声明深度控制模块。
 * 模块输入滤波后的深度和速度，输出浮沉子系统执行器掩码。
 *********************************************************************/

#ifndef ESP32_DEPTH_CONTROLLER_H
#define ESP32_DEPTH_CONTROLLER_H

#include <Arduino.h>
#include "Protocol.h"

class DepthController {
public:
    DepthController();

    // 初始化内部状态。
    void begin();

    // 用滤波后的深度和速度更新控制器。
    void update(bool depthValid, float filteredDepthCm, float depthSpeedCmS, uint32_t nowMs);

    // 目标深度控制接口。
    void setTargetDepth(float targetDepthCm);
    void holdCurrentDepth();
    void clearTarget();
    bool isHoldingTarget() const;

    // 手动浮沉控制接口。
    void manualAscend();
    void manualDescend();
    void manualStop();

    // 校准后重置控制器内部状态。
    void resetAfterCalibration();

    // 获取当前滤波后的状态和目标值。
    float getFilteredDepthCm() const;
    float getSpeedCmS() const;
    float getTargetDepthCm() const;

    // 获取当前浮沉子系统输出掩码。
    uint16_t getMask() const;

private:
    void adaptivePID(float err, float spd);
    void speedLimiter(float& u, float err);
    void startAscendPulse(uint32_t nowMs);
    void startDescendPulse(uint32_t nowMs);

    bool _holdingTarget;
    bool _ascendActive;
    bool _descendActive;

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

    uint32_t _lastBuoyancyCmdMs;
    uint32_t _ascendStartMs;
    uint32_t _descendStartMs;
    uint32_t _lastControlUpdateMs;
};

#endif // ESP32_DEPTH_CONTROLLER_H
