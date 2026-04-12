/**********************************************************************
 * RightTurnControl.h
 *
 * 右转子系统时序控制模块。
 *
 * 右转推进：泵开，阀 E/F 全部关闭（负循环，气室收缩推动右转）。
 * 气压平衡：转向完成后阀 E/F 每 100ms 交替单独打开，持续 200ms。
 * 全局平衡：收到 forceBalance() 后，阀 E/F 每 500ms 交替打开，持续 5s。
 *
 * 只生成执行器位掩码，不直接操作串口。
 *********************************************************************/

#ifndef ESP32_RIGHT_TURN_CONTROL_H
#define ESP32_RIGHT_TURN_CONTROL_H

#include <Arduino.h>
#include "Protocol.h"

class RightTurnControl {
public:
    RightTurnControl();

    void begin();

    // 开始右转序列。
    void start();

    // 取消右转（不进入平衡阶段）。
    void cancel();

    // 全局平衡：急停后直接进入平衡（500ms/5s，由 s 命令触发）。
    void forceBalance();

    // 每帧调用：推进转向持续时间和平衡阶段交替节拍。
    void update(uint32_t nowMs);

    // 返回本子系统当前帧需要激活的执行器位掩码。
    uint16_t getMask() const;

    bool isRunning() const;
    bool isBalancing() const;
    bool isBusy() const;

private:
    bool     _running;
    bool     _balancing;
    bool     _balanceValveOpen; // 当前帧是否处于平衡窗口内
    bool     _balanceAltPhase;  // 平衡窗口内当前交替相位（false=阀E，true=阀F）
    uint32_t _startMs;
    uint32_t _balanceStartMs;
    uint32_t _balanceAltMs;     // 平衡阶段每路阀门打开时长（进入平衡时写入）
    uint32_t _balanceTotalMs;   // 平衡阶段总时长（进入平衡时写入）
};

#endif // ESP32_RIGHT_TURN_CONTROL_H
