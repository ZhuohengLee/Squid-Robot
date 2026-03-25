/**********************************************************************
 * StatusDisplay.h
 *
 * 这个文件声明 Minima 状态回传的解析和显示接口。
 *********************************************************************/

#ifndef ESP32_STATUS_DISPLAY_H
#define ESP32_STATUS_DISPLAY_H

#include <Arduino.h>
#include "DepthSensorManager.h"
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
    void setDepthSensorManager(DepthSensorManager* manager);

private:
    bool _verboseMode;
    unsigned long _lastHeartbeat;
    uint8_t _rxBuffer[FRAME_LENGTH];
    uint8_t _rxIndex;
    DepthSensorManager* _depthMgr;

    void processMotionStatus(uint8_t status);
    void processHeartbeat();
    void processDepthStatus(uint8_t data0, uint8_t data1, uint8_t data2);
};

#endif // ESP32_STATUS_DISPLAY_H
