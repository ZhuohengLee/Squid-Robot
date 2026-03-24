/**********************************************************************
 * ForwardControl.h
 *
 * 这个文件声明前进子系统的时序控制模块。
 * 它只负责生成前进子系统对应的执行器位掩码，不直接发串口。
 *********************************************************************/

#ifndef ESP32_FORWARD_CONTROL_H
#define ESP32_FORWARD_CONTROL_H

#include <Arduino.h>
#include "Protocol.h"

class ForwardControl {
public:
    ForwardControl();

    // 初始化内部状态。
    void begin();

    // 开始持续前进。
    void start();

    // 停止前进，并进入短暂的平衡阶段。
    void stop();

    // 全局停机时直接清空所有状态。
    void emergencyStop();

    // 根据当前时钟推进前进节拍。
    void update(uint32_t nowMs);

    // 输出当前前进子系统需要激活的执行器位。
    uint16_t getMask() const;

    // 用于调试和状态显示。
    bool isRunning() const;
    bool isBalancing() const;
    bool isBusy() const;

private:
    bool _running;
    bool _balancing;
    bool _valvesOpen;
    bool _balanceValveOpen;
    uint32_t _phaseStartMs;
    uint32_t _balanceStartMs;
};

#endif // ESP32_FORWARD_CONTROL_H
