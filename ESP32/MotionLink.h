/**********************************************************************
 * MotionLink.h
 *
 * 这个文件声明 ESP32 到 Minima 的低层执行器链路。
 * ESP32 每次发送完整执行器掩码，Minima 只负责按掩码驱动引脚。
 *********************************************************************/

#ifndef ESP32_MOTION_LINK_H
#define ESP32_MOTION_LINK_H

#include <Arduino.h>
#include "Protocol.h"

class MotionLink {
public:
    MotionLink();

    // 初始化和 Minima 通信的硬件串口。
    void begin();

    // 下发完整执行器掩码；默认只有状态变化时才发送。
    void applyMask(uint16_t actuatorMask, bool forceSend = false);

    // 发送全局急停并清空本地记录的掩码。
    void emergencyStop();

    // 读取最近一次下发的掩码，便于调试。
    uint16_t getLastMask() const;

private:
    // 保存最近一次已经发送给 Minima 的掩码。
    uint16_t _lastMask;

    // 通用发帧函数。
    void sendCommand(uint8_t cmd, uint8_t data0 = 0, uint8_t data1 = 0, uint8_t data2 = 0);
};

#endif // ESP32_MOTION_LINK_H
