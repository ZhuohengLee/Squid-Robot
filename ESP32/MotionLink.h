/**********************************************************************
 * MotionLink.h
 *
 * 杩欎釜鏂囦欢澹版槑 ESP32 鍒?Minima 鐨勪綆灞傛墽琛屽櫒閾捐矾銆? * ESP32 姣忔鍙戦€佸畬鏁存墽琛屽櫒鎺╃爜锛孧inima 鍙礋璐ｆ寜鎺╃爜椹卞姩寮曡剼銆? *********************************************************************/

// 中文逐行说明：下面这一行保留原始代码 -> #ifndef ESP32_MOTION_LINK_H
#ifndef ESP32_MOTION_LINK_H
// 中文逐行说明：下面这一行保留原始代码 -> #define ESP32_MOTION_LINK_H
#define ESP32_MOTION_LINK_H

// 中文逐行说明：下面这一行保留原始代码 -> #include <Arduino.h>
#include <Arduino.h>
// 中文逐行说明：下面这一行保留原始代码 -> #include "Protocol.h"
#include "Protocol.h"

// 中文逐行说明：下面这一行保留原始代码 -> class MotionLink {
class MotionLink {
// 中文逐行说明：下面这一行保留原始代码 -> public:
public:
    // 中文逐行说明：下面这一行保留原始代码 -> MotionLink();
    MotionLink();

    // 鍒濆鍖栧拰 Minima 閫氫俊鐨勭‖浠朵覆鍙ｃ€?    void begin();

    // 涓嬪彂瀹屾暣鎵ц鍣ㄦ帺鐮侊紱榛樿鍙湁鐘舵€佸彉鍖栨椂鎵嶅彂閫併€?    void applyMask(uint16_t actuatorMask, bool forceSend = false);

    // 鍙戦€佸叏灞€鎬ュ仠骞舵竻绌烘湰鍦拌褰曠殑鎺╃爜銆?    void emergencyStop();

    // 璇诲彇鏈€杩戜竴娆′笅鍙戠殑鎺╃爜锛屼究浜庤皟璇曘€?    uint16_t getLastMask() const;

// 中文逐行说明：下面这一行保留原始代码 -> private:
private:
    // 淇濆瓨鏈€杩戜竴娆″凡缁忓彂閫佺粰 Minima 鐨勬帺鐮併€?    uint16_t _lastMask;

    // 閫氱敤鍙戝抚鍑芥暟銆?    void sendCommand(uint8_t cmd, uint8_t data0 = 0, uint8_t data1 = 0, uint8_t data2 = 0);
// 中文逐行说明：下面这一行保留原始代码 -> };
};

// 中文逐行说明：下面这一行保留原始代码 -> #endif // ESP32_MOTION_LINK_H
#endif // ESP32_MOTION_LINK_H
