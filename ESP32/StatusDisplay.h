/**********************************************************************
 * StatusDisplay.h
 *
 * 这个文件声明 Minima 回传状态的解析与显示接口。
 *********************************************************************/

// 防止头文件被重复包含。
#ifndef ESP32_STATUS_DISPLAY_H
// 定义头文件保护宏。
#define ESP32_STATUS_DISPLAY_H

// 引入 Arduino 基础类型。
#include <Arduino.h>
// 引入协议常量和串口定义。
#include "Protocol.h"

// 声明状态显示类。
class StatusDisplay {
public:
    // 声明构造函数。
    StatusDisplay();

    // 声明处理 Minima 串口反馈的函数。
    void processMinimaFeedback();
    // 声明打开详细输出模式的函数。
    void enableVerbose();
    // 声明关闭详细输出模式的函数。
    void disableVerbose();
    // 声明切换详细输出模式的函数。
    void toggleVerbose();
    // 声明读取当前详细输出开关状态的函数。
    bool isVerboseEnabled() const;

private:
    // 保存当前是否处于详细输出模式。
    bool _verboseMode;
    // 保存最近一次收到心跳的时间戳。
    unsigned long _lastHeartbeat;
    // 保存接收帧缓冲区。
    uint8_t _rxBuffer[FRAME_LENGTH];
    // 保存当前接收索引。
    uint8_t _rxIndex;

    // 声明解析运动状态字节的函数。
    void processMotionStatus(uint8_t status);
    // 声明处理心跳帧的函数。
    void processHeartbeat();
};

// 结束头文件保护。
#endif // ESP32_STATUS_DISPLAY_H
