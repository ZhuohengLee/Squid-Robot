/**********************************************************************
 * MotionControl.h
 *
 * 这个文件声明 Minima 侧的低层执行器控制类。
 * 它不再负责任何运动时间逻辑，只负责把位掩码写到真实引脚。
 *********************************************************************/

#ifndef MOTION_CONTROL_H
#define MOTION_CONTROL_H

#include <Arduino.h>
#include "PinDefinitions.h"
#include "Protocol.h"

class MotionController {
public:
    MotionController();

    // 初始化全部执行器引脚。
    void begin();

    // 应用来自 ESP32 的完整执行器掩码。
    void applyMask(uint16_t mask);

    // 全局急停，清空所有执行器。
    void emergencyStopAll();

    // 获取当前输出掩码和状态。
    uint16_t getMask() const;
    bool isForwardActive() const;
    bool isTurnActive() const;
    bool isBuoyancyActive() const;
    bool isAnyActive() const;

    // 打印当前执行器状态，便于串口调试。
    void printStatus();

private:
    uint16_t _mask;

    // 把内部掩码同步到真实 GPIO。
    void writeOutputs();
};

#endif // MOTION_CONTROL_H
