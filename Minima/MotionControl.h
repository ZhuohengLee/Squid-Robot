/**********************************************************************
 * MotionControl.h
 *
 * 这个文件声明 Minima 侧的运动执行状态机接口。
 *********************************************************************/

// 防止头文件被重复包含。
#ifndef MOTION_CONTROL_H
// 定义头文件保护宏。
#define MOTION_CONTROL_H

// 引入 Arduino 基础类型。
#include <Arduino.h>
// 引入引脚和时序常量定义。
#include "PinDefinitions.h"

// 声明运动控制类。
class MotionController {
public:
    // 声明构造函数。
    MotionController();

    // 声明初始化执行器的函数。
    void begin();
    // 声明更新全部状态机的函数。
    void update();

    // 声明“开始前进”命令入口。
    void startForwardCommand();
    // 声明“停止前进”命令入口。
    void stopForwardCommand();
    // 声明兼容旧协议的前进切换入口。
    void toggleForward();
    // 声明强制停止前进子系统的函数。
    void forceStopForward();
    // 声明读取前进子系统是否激活的函数。
    bool isForwardActive();
    // 声明读取前进平衡阶段是否激活的函数。
    bool isForwardBalancing();
    // 声明读取前进阀门当前是否打开的函数。
    bool isForwardValvesOpen();

    // 声明“开始右转”命令入口。
    void startTurnRight();
    // 声明“开始左转”命令入口。
    void startTurnLeft();
    // 声明“停止转向”命令入口。
    void stopTurnCommand();
    // 声明强制停止转向子系统的函数。
    void forceStopTurn();
    // 声明读取转向子系统是否激活的函数。
    bool isTurnActive();
    // 声明读取转向平衡阶段是否激活的函数。
    bool isTurnBalancing();
    // 声明读取当前是否为左转方向的函数。
    bool isTurnLeft();
    // 声明读取当前转向阶段编号的函数。
    uint8_t getTurnStage();

    // 声明“开始上浮”命令入口。
    void startAscend();
    // 声明“开始下沉”命令入口。
    void startDescend();
    // 声明停止浮沉子系统的函数。
    void stopBuoyancy();
    // 声明读取上浮动作是否激活的函数。
    bool isAscendActive();
    // 声明读取下沉动作是否激活的函数。
    bool isDescendActive();

    // 声明全局急停函数。
    void emergencyStopAll();
    // 声明读取任一子系统是否活跃的函数。
    bool isAnyActive();
    // 声明打印当前运动状态的函数。
    void printStatus();

private:
    // 保存前进子系统是否正在工作。
    bool forward_active;
    // 保存前进阀门当前是否为打开状态。
    bool forward_valves_open;
    // 保存前进子系统是否处于平衡阶段。
    bool forward_balancing;
    // 保存前进相位起始时间。
    unsigned long forward_phase_start;
    // 保存前进平衡起始时间。
    unsigned long forward_balance_start;

    // 声明真正执行前进启动的内部函数。
    void startForward();
    // 声明真正执行前进停止并平衡的内部函数。
    void stopForwardWithBalance();
    // 声明更新前进节拍的内部函数。
    void updateForward();
    // 声明更新前进平衡过程的内部函数。
    void updateForwardBalance();

    // 保存转向子系统是否正在工作。
    bool turn_active;
    // 保存当前转向方向是否为左转。
    bool turn_is_left;
    // 保存转向子系统是否处于平衡阶段。
    bool turn_balancing;
    // 保存转向子系统是否处于方向切换平衡阶段。
    bool turn_switching;
    // 保存当前转向阶段号。
    uint8_t turn_stage;
    // 保存等待执行的转向方向。
    uint8_t pending_turn_direction;
    // 保存转向启动时间。
    unsigned long turn_start_time;
    // 保存转向平衡起始时间。
    unsigned long turn_balance_start;

    // 声明真正执行右转启动的内部函数。
    void executeTurnRight();
    // 声明真正执行左转启动的内部函数。
    void executeTurnLeft();
    // 声明真正执行转向停止并平衡的内部函数。
    void stopTurnWithBalance();
    // 声明执行转向方向切换平衡的内部函数。
    void startTurnSwitchBalance(uint8_t new_direction);
    // 声明更新转向主阶段的内部函数。
    void updateTurn();
    // 声明更新转向平衡阶段的内部函数。
    void updateTurnBalance();

    // 保存上浮动作是否激活。
    bool ascend_active;
    // 保存下沉动作是否激活。
    bool descend_active;
    // 保存上浮动作起始时间。
    unsigned long ascend_start_time;
    // 保存下沉动作起始时间。
    unsigned long descend_start_time;

    // 声明更新浮沉子系统的内部函数。
    void updateBuoyancy();
};

// 结束头文件保护。
#endif // MOTION_CONTROL_H
