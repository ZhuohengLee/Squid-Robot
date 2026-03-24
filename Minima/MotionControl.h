/**********************************************************************
 * MotionControl.h
 *
 * 杩欎釜鏂囦欢澹版槑 Minima 渚х殑浣庡眰鎵ц鍣ㄦ帶鍒剁被銆? * 瀹冧笉鍐嶈礋璐ｄ换浣曡繍鍔ㄦ椂闂撮€昏緫锛屽彧璐熻矗鎶婁綅鎺╃爜鍐欏埌鐪熷疄寮曡剼銆? *********************************************************************/

// 中文逐行说明：下面这一行保留原始代码 -> #ifndef MOTION_CONTROL_H
#ifndef MOTION_CONTROL_H
// 中文逐行说明：下面这一行保留原始代码 -> #define MOTION_CONTROL_H
#define MOTION_CONTROL_H

// 中文逐行说明：下面这一行保留原始代码 -> #include <Arduino.h>
#include <Arduino.h>
// 中文逐行说明：下面这一行保留原始代码 -> #include "PinDefinitions.h"
#include "PinDefinitions.h"
// 中文逐行说明：下面这一行保留原始代码 -> #include "Protocol.h"
#include "Protocol.h"

// 中文逐行说明：下面这一行保留原始代码 -> class MotionController {
class MotionController {
// 中文逐行说明：下面这一行保留原始代码 -> public:
public:
    // 中文逐行说明：下面这一行保留原始代码 -> MotionController();
    MotionController();

    // 鍒濆鍖栧叏閮ㄦ墽琛屽櫒寮曡剼銆?    void begin();

    // 搴旂敤鏉ヨ嚜 ESP32 鐨勫畬鏁存墽琛屽櫒鎺╃爜銆?    void applyMask(uint16_t mask);

    // 鍏ㄥ眬鎬ュ仠锛屾竻绌烘墍鏈夋墽琛屽櫒銆?    void emergencyStopAll();

    // 鑾峰彇褰撳墠杈撳嚭鎺╃爜鍜岀姸鎬併€?    uint16_t getMask() const;
    // 中文逐行说明：下面这一行保留原始代码 -> bool isForwardActive() const;
    bool isForwardActive() const;
    // 中文逐行说明：下面这一行保留原始代码 -> bool isTurnActive() const;
    bool isTurnActive() const;
    // 中文逐行说明：下面这一行保留原始代码 -> bool isBuoyancyActive() const;
    bool isBuoyancyActive() const;
    // 中文逐行说明：下面这一行保留原始代码 -> bool isAnyActive() const;
    bool isAnyActive() const;

    // 鎵撳嵃褰撳墠鎵ц鍣ㄧ姸鎬侊紝渚夸簬涓插彛璋冭瘯銆?    void printStatus();

// 中文逐行说明：下面这一行保留原始代码 -> private:
private:
    // 中文逐行说明：下面这一行保留原始代码 -> uint16_t _mask;
    uint16_t _mask;

    // 鎶婂唴閮ㄦ帺鐮佸悓姝ュ埌鐪熷疄 GPIO銆?    void writeOutputs();
// 中文逐行说明：下面这一行保留原始代码 -> };
};

// 中文逐行说明：下面这一行保留原始代码 -> #endif // MOTION_CONTROL_H
#endif // MOTION_CONTROL_H
