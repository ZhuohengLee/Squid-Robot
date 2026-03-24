/**********************************************************************
 * DepthController.h
 *
 * 杩欎釜鏂囦欢澹版槑娣卞害鎺у埗妯″潡銆? * 妯″潡杈撳叆婊ゆ尝鍚庣殑娣卞害鍜岄€熷害锛岃緭鍑烘诞娌夊瓙绯荤粺鎵ц鍣ㄦ帺鐮併€? *********************************************************************/

// 中文逐行说明：下面这一行保留原始代码 -> #ifndef ESP32_DEPTH_CONTROLLER_H
#ifndef ESP32_DEPTH_CONTROLLER_H
// 中文逐行说明：下面这一行保留原始代码 -> #define ESP32_DEPTH_CONTROLLER_H
#define ESP32_DEPTH_CONTROLLER_H

// 中文逐行说明：下面这一行保留原始代码 -> #include <Arduino.h>
#include <Arduino.h>
// 中文逐行说明：下面这一行保留原始代码 -> #include "Protocol.h"
#include "Protocol.h"

// 中文逐行说明：下面这一行保留原始代码 -> class DepthController {
class DepthController {
// 中文逐行说明：下面这一行保留原始代码 -> public:
public:
    // 中文逐行说明：下面这一行保留原始代码 -> DepthController();
    DepthController();

    // 鍒濆鍖栧唴閮ㄧ姸鎬併€?    void begin();

    // 鐢ㄦ护娉㈠悗鐨勬繁搴﹀拰閫熷害鏇存柊鎺у埗鍣ㄣ€?    void update(bool depthValid, float filteredDepthCm, float depthSpeedCmS, uint32_t nowMs);

    // 鐩爣娣卞害鎺у埗鎺ュ彛銆?    void setTargetDepth(float targetDepthCm);
    // 中文逐行说明：下面这一行保留原始代码 -> void holdCurrentDepth();
    void holdCurrentDepth();
    // 中文逐行说明：下面这一行保留原始代码 -> void clearTarget();
    void clearTarget();
    // 中文逐行说明：下面这一行保留原始代码 -> bool isHoldingTarget() const;
    bool isHoldingTarget() const;

    // 鎵嬪姩娴矇鎺у埗鎺ュ彛銆?    void manualAscend();
    // 中文逐行说明：下面这一行保留原始代码 -> void manualDescend();
    void manualDescend();
    // 中文逐行说明：下面这一行保留原始代码 -> void manualStop();
    void manualStop();

    // 鏍″噯鍚庨噸缃帶鍒跺櫒鍐呴儴鐘舵€併€?    void resetAfterCalibration();

    // 鑾峰彇褰撳墠婊ゆ尝鍚庣殑鐘舵€佸拰鐩爣鍊笺€?    float getFilteredDepthCm() const;
    // 中文逐行说明：下面这一行保留原始代码 -> float getSpeedCmS() const;
    float getSpeedCmS() const;
    // 中文逐行说明：下面这一行保留原始代码 -> float getTargetDepthCm() const;
    float getTargetDepthCm() const;

    // 鑾峰彇褰撳墠娴矇瀛愮郴缁熻緭鍑烘帺鐮併€?    uint16_t getMask() const;

// 中文逐行说明：下面这一行保留原始代码 -> private:
private:
    // 中文逐行说明：下面这一行保留原始代码 -> void adaptivePID(float err, float spd);
    void adaptivePID(float err, float spd);
    // 中文逐行说明：下面这一行保留原始代码 -> void speedLimiter(float& u, float err);
    void speedLimiter(float& u, float err);
    // 中文逐行说明：下面这一行保留原始代码 -> void startAscendPulse(uint32_t nowMs);
    void startAscendPulse(uint32_t nowMs);
    // 中文逐行说明：下面这一行保留原始代码 -> void startDescendPulse(uint32_t nowMs);
    void startDescendPulse(uint32_t nowMs);

    // 中文逐行说明：下面这一行保留原始代码 -> bool _holdingTarget;
    bool _holdingTarget;
    // 中文逐行说明：下面这一行保留原始代码 -> bool _ascendActive;
    bool _ascendActive;
    // 中文逐行说明：下面这一行保留原始代码 -> bool _descendActive;
    bool _descendActive;

    // 中文逐行说明：下面这一行保留原始代码 -> float _targetDepthCm;
    float _targetDepthCm;
    // 中文逐行说明：下面这一行保留原始代码 -> float _depthFilt;
    float _depthFilt;
    // 中文逐行说明：下面这一行保留原始代码 -> float _speedCmS;
    float _speedCmS;
    // 中文逐行说明：下面这一行保留原始代码 -> float _integ;
    float _integ;

    // 中文逐行说明：下面这一行保留原始代码 -> float _kpBase;
    float _kpBase;
    // 中文逐行说明：下面这一行保留原始代码 -> float _kiBase;
    float _kiBase;
    // 中文逐行说明：下面这一行保留原始代码 -> float _kdBase;
    float _kdBase;
    // 中文逐行说明：下面这一行保留原始代码 -> float _kp;
    float _kp;
    // 中文逐行说明：下面这一行保留原始代码 -> float _ki;
    float _ki;
    // 中文逐行说明：下面这一行保留原始代码 -> float _kd;
    float _kd;

    // 中文逐行说明：下面这一行保留原始代码 -> uint32_t _lastBuoyancyCmdMs;
    uint32_t _lastBuoyancyCmdMs;
    // 中文逐行说明：下面这一行保留原始代码 -> uint32_t _ascendStartMs;
    uint32_t _ascendStartMs;
    // 中文逐行说明：下面这一行保留原始代码 -> uint32_t _descendStartMs;
    uint32_t _descendStartMs;
    // 中文逐行说明：下面这一行保留原始代码 -> uint32_t _lastControlUpdateMs;
    uint32_t _lastControlUpdateMs;
// 中文逐行说明：下面这一行保留原始代码 -> };
};

// 中文逐行说明：下面这一行保留原始代码 -> #endif // ESP32_DEPTH_CONTROLLER_H
#endif // ESP32_DEPTH_CONTROLLER_H
