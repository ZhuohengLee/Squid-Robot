/**********************************************************************
 * Protocol.h
 *
 * 这个文件定义 Minima 侧使用的固定长度控制帧协议。
 *********************************************************************/

// 防止头文件被重复包含。
#ifndef MINIMA_PROTOCOL_H
// 定义头文件保护宏。
#define MINIMA_PROTOCOL_H

// 引入 Arduino 基础类型和 Stream 接口。
#include <Arduino.h>

// 定义 ESP32 与 Minima 链路所使用的串口波特率。
#define UART_BAUD_RATE        2000000

// 定义控制帧帧头字节。
#define FRAME_HEADER          0xAA
// 定义控制帧帧尾字节。
#define FRAME_TAIL            0x55
// 定义固定控制帧长度。
#define FRAME_LENGTH          8

// 定义旧版“前进切换”命令号。
#define CMD_FORWARD_TOGGLE    0x01
// 定义急停命令号。
#define CMD_EMERGENCY_STOP    0x02
// 定义右转命令号。
#define CMD_TURN_RIGHT        0x03
// 定义左转命令号。
#define CMD_TURN_LEFT         0x04
// 定义上浮命令号。
#define CMD_ASCEND            0x05
// 定义下沉命令号。
#define CMD_DESCEND           0x06
// 定义深度校准命令号，当前在 Minima 侧仅保留兼容。
#define CMD_CALIBRATE_DEPTH   0x07

// 定义开始前进命令号。
#define CMD_FORWARD_START     0x08
// 定义停止前进命令号。
#define CMD_FORWARD_STOP      0x09
// 定义停止转向命令号。
#define CMD_TURN_STOP         0x0A
// 定义停止浮沉命令号。
#define CMD_BUOYANCY_STOP     0x0B

// 定义运动状态回传命令号。
#define STATUS_MOTION         0x81
// 定义心跳状态回传命令号。
#define STATUS_HEARTBEAT      0x82
// 定义传感器状态回传命令号，当前保留兼容。
#define STATUS_SENSOR_DATA    0x83

// 定义协议帧的有效载荷结构。
struct ProtocolFrame {
    // 保存命令字节。
    uint8_t cmd;
    // 保存第 1 个数据字节。
    uint8_t data0;
    // 保存第 2 个数据字节。
    uint8_t data1;
    // 保存第 3 个数据字节。
    uint8_t data2;
};

// 声明 CRC8 计算函数。
uint8_t calculateCRC8(const uint8_t* data, uint8_t len);
// 声明固定长度协议帧发送函数。
void sendFrame(Stream& port, uint8_t cmd, uint8_t data0 = 0, uint8_t data1 = 0, uint8_t data2 = 0);

// 声明协议接收器类。
class ProtocolReceiver {
public:
    // 声明构造函数。
    ProtocolReceiver();
    // 声明从串口轮询一帧有效数据的函数。
    bool poll(Stream& port, ProtocolFrame& frame);

private:
    // 保存接收缓冲区。
    uint8_t rxBuffer[FRAME_LENGTH];
    // 保存当前接收索引。
    uint8_t rxIndex;
};

// 结束头文件保护。
#endif // MINIMA_PROTOCOL_H
