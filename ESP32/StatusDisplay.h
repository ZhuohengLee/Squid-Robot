/**********************************************************************
 * StatusDisplay.h
 *
 * 杩欎釜鏂囦欢澹版槑 Minima 鐘舵€佸洖浼犵殑瑙ｆ瀽鍜屾樉绀烘帴鍙ｃ€? *********************************************************************/

// 中文逐行说明：下面这一行保留原始代码 -> #ifndef ESP32_STATUS_DISPLAY_H
#ifndef ESP32_STATUS_DISPLAY_H
// 中文逐行说明：下面这一行保留原始代码 -> #define ESP32_STATUS_DISPLAY_H
#define ESP32_STATUS_DISPLAY_H

// 中文逐行说明：下面这一行保留原始代码 -> #include <Arduino.h>
#include <Arduino.h>
// 中文逐行说明：下面这一行保留原始代码 -> #include "Protocol.h"
#include "Protocol.h"

// 中文逐行说明：下面这一行保留原始代码 -> class StatusDisplay {
class StatusDisplay {
// 中文逐行说明：下面这一行保留原始代码 -> public:
public:
    // 中文逐行说明：下面这一行保留原始代码 -> StatusDisplay();
    StatusDisplay();

    // 杞骞惰В鏋?Minima 鍥炰紶鐨勭姸鎬佸抚銆?    void processMinimaFeedback();

    // 璇︾粏杈撳嚭鎺у埗銆?    void enableVerbose();
    // 中文逐行说明：下面这一行保留原始代码 -> void disableVerbose();
    void disableVerbose();
    // 中文逐行说明：下面这一行保留原始代码 -> void toggleVerbose();
    void toggleVerbose();
    // 中文逐行说明：下面这一行保留原始代码 -> bool isVerboseEnabled() const;
    bool isVerboseEnabled() const;

// 中文逐行说明：下面这一行保留原始代码 -> private:
private:
    // 中文逐行说明：下面这一行保留原始代码 -> bool _verboseMode;
    bool _verboseMode;
    // 中文逐行说明：下面这一行保留原始代码 -> unsigned long _lastHeartbeat;
    unsigned long _lastHeartbeat;
    // 中文逐行说明：下面这一行保留原始代码 -> uint8_t _rxBuffer[FRAME_LENGTH];
    uint8_t _rxBuffer[FRAME_LENGTH];
    // 中文逐行说明：下面这一行保留原始代码 -> uint8_t _rxIndex;
    uint8_t _rxIndex;

    // 中文逐行说明：下面这一行保留原始代码 -> void processMotionStatus(uint8_t status);
    void processMotionStatus(uint8_t status);
    // 中文逐行说明：下面这一行保留原始代码 -> void processHeartbeat();
    void processHeartbeat();
// 中文逐行说明：下面这一行保留原始代码 -> };
};

// 中文逐行说明：下面这一行保留原始代码 -> #endif // ESP32_STATUS_DISPLAY_H
#endif // ESP32_STATUS_DISPLAY_H
