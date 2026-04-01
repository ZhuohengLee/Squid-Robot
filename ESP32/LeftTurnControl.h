/**********************************************************************
 * LeftTurnControl.h
 *
 * 左转子系统时序控制模块。
 *
 * 左转推进：泵开，阀 E/F 同时打开（正循环，气室膨胀推动左转）。
 * 气压平衡：转向完成后阀 E/F 每 100ms 交替单独打开，两气室泄压归中。
 *
 * 只生成执行器位掩码，不直接操作串口。
 *********************************************************************/

#ifndef ESP32_LEFT_TURN_CONTROL_H
#define ESP32_LEFT_TURN_CONTROL_H

#include <Arduino.h>
#include "Protocol.h"

class LeftTurnControl {
public:
    LeftTurnControl();

    void begin();

    // 开始左转序列。
    void start();

    // 取消左转（不进入平衡阶段）。
    void cancel();

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
};

#endif // ESP32_LEFT_TURN_CONTROL_H
