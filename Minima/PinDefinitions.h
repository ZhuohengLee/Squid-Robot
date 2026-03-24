/**********************************************************************
 * PinDefinitions.h
 *
 * 这个文件定义 Minima 侧执行器引脚和动作定时常量。
 *********************************************************************/

// 防止头文件被重复包含。
#ifndef MINIMA_PIN_DEFINITIONS_H
// 定义头文件保护宏。
#define MINIMA_PIN_DEFINITIONS_H

// 定义前进用主气泵引脚。
#define PUMP_A_PIN      2
// 定义前进用阀门 B 引脚。
#define VALVE_B_PIN     3
// 定义前进用阀门 C 引脚。
#define VALVE_C_PIN     4

// 定义转向用气泵引脚。
#define PUMP_D_PIN      5
// 定义转向用阀门 E 引脚。
#define VALVE_E_PIN     6
// 定义转向用阀门 F 引脚。
#define VALVE_F_PIN     7

// 定义浮沉用气泵引脚。
#define PUMP_G_PIN      8
// 定义浮沉用阀门 H 引脚。
#define VALVE_H_PIN     9
// 定义浮沉用阀门 I 引脚。
#define VALVE_I_PIN     10

// 定义前进阀门切换周期，单位毫秒。
#define FORWARD_VALVE_INTERVAL      600
// 定义前进停止后平衡阶段持续时间，单位毫秒。
#define FORWARD_BALANCE_TIME        500
// 定义前进停止后延迟多久开始平衡，单位毫秒。
#define FORWARD_BALANCE_DELAY       10

// 定义单次转向动作持续时间，单位毫秒。
#define TURN_DURATION               1000
// 定义转向完成后的平衡时间，单位毫秒。
#define TURN_BALANCE_TIME           200
// 定义转向平衡开始前的延迟，单位毫秒。
#define TURN_BALANCE_DELAY          10
// 定义转向方向切换时的平衡时间，单位毫秒。
#define TURN_SWITCH_BALANCE_TIME    200

// 定义上浮动作持续时间，单位毫秒。
#define ASCEND_DURATION             1500
// 定义下沉动作持续时间，单位毫秒。
#define DESCEND_DURATION            1500

// 定义“无转向”状态值。
#define TURN_NONE                   0
// 定义“右转”状态值。
#define TURN_RIGHT                  1
// 定义“左转”状态值。
#define TURN_LEFT                   2

// 结束头文件保护。
#endif // MINIMA_PIN_DEFINITIONS_H
