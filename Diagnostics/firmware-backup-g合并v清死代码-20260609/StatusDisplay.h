/**********************************************************************
 * StatusDisplay.h
 *
 * 这个文件声明 Minima 状态回传的解析和显示接口。
 *********************************************************************/

#ifndef ESP32_STATUS_DISPLAY_H
#define ESP32_STATUS_DISPLAY_H

#include <Arduino.h>
#include "Protocol.h"

class StatusDisplay {
public:
    StatusDisplay();

    // 轮询并解析 Minima 回传的状态帧。
    void processMinimaFeedback();

    // 详细输出控制。
    void enableVerbose();
    void disableVerbose();
    void toggleVerbose();
    bool isVerboseEnabled() const;
    uint8_t getLastMotionStatus() const;
    bool hasRecentHeartbeat(uint32_t nowMs, uint32_t timeoutMs = 3000) const;

private:
    bool _verboseMode;
    unsigned long _lastHeartbeat;
    uint8_t _lastMotionStatus;
    uint8_t _rxBuffer[FRAME_LENGTH];
    uint8_t _rxIndex;

    void processMotionStatus(uint8_t status);
    void processHeartbeat();
};

#endif // ESP32_STATUS_DISPLAY_H
