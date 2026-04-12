/**********************************************************************
 * Protocol.h
 *
 * 这个文件定义 Minima 侧使用的固定长度控制帧协议。
 * 当前协议里，ESP32 只下发完整执行器掩码，Minima 只负责执行。
 *********************************************************************/

#ifndef MINIMA_PROTOCOL_H
#define MINIMA_PROTOCOL_H

#include <Arduino.h>

#define UART_BAUD_RATE        2000000

#define FRAME_HEADER          0xAA
#define FRAME_TAIL            0x55
#define FRAME_LENGTH          8

#define CMD_SET_ACTUATORS     0x01
#define CMD_EMERGENCY_STOP    0x02
#define CMD_SET_BUOYANCY      0x04

#define STATUS_MOTION         0x81
#define STATUS_HEARTBEAT      0x82

// 前进子系统执行器位（泵 + 阀 a/b）。
constexpr uint16_t ACT_FORWARD_PUMP    = 0x0001;
constexpr uint16_t ACT_FORWARD_VALVE_A = 0x0002;
constexpr uint16_t ACT_FORWARD_VALVE_B = 0x0004;

// 转向子系统执行器位（泵 + 阀 c/d）。
constexpr uint16_t ACT_TURN_PUMP       = 0x0008;
constexpr uint16_t ACT_TURN_VALVE_C    = 0x0010;
constexpr uint16_t ACT_TURN_VALVE_D    = 0x0020;

// 浮力子系统执行器位（泵 + 阀 e/f）。
constexpr uint16_t ACT_BUOYANCY_PUMP   = 0x0040;
constexpr uint16_t ACT_BUOYANCY_VALVE_E= 0x0080;
constexpr uint16_t ACT_BUOYANCY_VALVE_F= 0x0100;

// 分组掩码，便于快速判断当前哪个子系统被激活。
constexpr uint16_t ACT_FORWARD_GROUP =
    ACT_FORWARD_PUMP | ACT_FORWARD_VALVE_A | ACT_FORWARD_VALVE_B;
constexpr uint16_t ACT_TURN_GROUP =
    ACT_TURN_PUMP | ACT_TURN_VALVE_C | ACT_TURN_VALVE_D;
constexpr uint16_t ACT_BUOYANCY_GROUP =
    ACT_BUOYANCY_PUMP | ACT_BUOYANCY_VALVE_E | ACT_BUOYANCY_VALVE_F;

constexpr uint8_t BUOYANCY_STOP    = 0;
constexpr uint8_t BUOYANCY_ASCEND  = 1;
constexpr uint8_t BUOYANCY_DESCEND = 2;
constexpr uint8_t BUOYANCY_BALANCE = 3; // 气压平衡：E/F 同时通电，泵关闭

struct ProtocolFrame {
    uint8_t cmd;
    uint8_t data0;
    uint8_t data1;
    uint8_t data2;
};

uint8_t calculateCRC8(const uint8_t* data, uint8_t len);
void sendFrame(Stream& port, uint8_t cmd, uint8_t data0 = 0, uint8_t data1 = 0, uint8_t data2 = 0);

class ProtocolReceiver {
public:
    ProtocolReceiver();
    bool poll(Stream& port, ProtocolFrame& frame);

private:
    uint8_t rxBuffer[FRAME_LENGTH];
    uint8_t rxIndex;
};

#endif // MINIMA_PROTOCOL_H
