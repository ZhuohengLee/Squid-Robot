/**********************************************************************
 * ForwardControl.h
 *
 * 前进子系统时序控制模块。
 *
 * 正常推进：泵开，阀 B/C 每 500ms 同时开/关交替（正负循环驱动气室）。
 * 气压平衡：收到 stop() 后，阀 B/C 每 200ms 交替单独打开，持续 800ms。
 * 全局平衡：收到 forceBalance() 后，阀 B/C 每 500ms 交替打开，持续 5s。
 * 急停：     直接清空所有状态，不进入平衡阶段。
 *
 * 只生成执行器位掩码，不直接操作串口。
 *********************************************************************/

#ifndef ESP32_FORWARD_CONTROL_H
#define ESP32_FORWARD_CONTROL_H

#include <Arduino.h>
#include "Protocol.h"

class ForwardControl {
public:
    ForwardControl();

    void begin();

    // 开始持续前进（阀门初始为打开状态）。
    void start();

    // 有序停止：停泵后进入气压平衡阶段（由 w 第二次触发）。
    void stop();

    // 全局平衡：急停后直接进入平衡（500ms/5s，由 s 命令触发）。
    void forceBalance();

    // 急停：直接清空所有状态，不平衡。
    void emergencyStop();

    // 每帧调用：推进阀门切换节拍和平衡阶段交替节拍。
    void update(uint32_t nowMs);

    // 返回本子系统当前帧需要激活的执行器位掩码。
    uint16_t getMask() const;

    bool isRunning() const;
    bool isBalancing() const;
    bool isBusy() const;

private:
    bool     _running;
    bool     _balancing;
    bool     _valvesOpen;       // 正常推进阶段的阀门状态
    bool     _balanceValveOpen; // 当前帧是否处于平衡窗口内
    bool     _balanceAltPhase;  // 平衡窗口内当前交替相位（false=阀B，true=阀C）
    uint32_t _phaseStartMs;
    uint32_t _balanceStartMs;
    uint32_t _balanceAltMs;     // 平衡阶段每路阀门打开时长（进入平衡时写入）
    uint32_t _balanceTotalMs;   // 平衡阶段总时长（进入平衡时写入）
};

#endif // ESP32_FORWARD_CONTROL_H
