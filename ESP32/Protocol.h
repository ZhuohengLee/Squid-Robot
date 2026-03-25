/**********************************************************************
 * Protocol.h
 *
 * 这个文件定义 ESP32 侧使用的通信协议、引脚映射和执行器位掩码。
 * 当前架构里，ESP32 负责运动状态机，Minima 只负责按位掩码执行输出。
 *********************************************************************/

#ifndef ESP32_PROTOCOL_H
#define ESP32_PROTOCOL_H

#include <Arduino.h>

// ESP32 和 Minima 的串口链路。
#define UART_TO_MINIMA        Serial1
#define UART_BAUD_RATE        2000000
#define UART_TX_PIN           17
#define UART_RX_PIN           18

// ESP32 和 CH9434A 的 SPI 接线。
#define SPI_MOSI              11
#define SPI_MISO              13
#define SPI_SCK               12
#define SPI_CS                10
#define CH9434A_INT           14

// CH9434A 的 UART 通道分配。
#define ULTRASONIC_FRONT_UART 1
#define ULTRASONIC_LEFT_UART  2
#define ULTRASONIC_RIGHT_UART 3

// 固定长度协议帧格式。
#define FRAME_HEADER          0xAA
#define FRAME_TAIL            0x55
#define FRAME_LENGTH          8

// 当前只保留两个控制命令：设置执行器输出和全局急停。
#define CMD_SET_ACTUATORS     0x01
#define CMD_EMERGENCY_STOP    0x02
#define CMD_CALIBRATE_DEPTH_ZERO 0x03

// Minima 回传的状态帧命令号。
#define STATUS_MOTION         0x81
#define STATUS_HEARTBEAT      0x82
#define STATUS_DEPTH          0x83

// 传感器数量和串口参数。
#define NUM_ULTRASONIC        3
#define ULTRASONIC_BAUDRATE   115200
#define ULTRASONIC_TIMEOUT    30

// 超声波逻辑编号。
#define SENSOR_FRONT          0
#define SENSOR_LEFT           1
#define SENSOR_RIGHT          2

// 前进子系统执行器位。
constexpr uint16_t ACT_FORWARD_PUMP    = 0x0001;
constexpr uint16_t ACT_FORWARD_VALVE_B = 0x0002;
constexpr uint16_t ACT_FORWARD_VALVE_C = 0x0004;

// 转向子系统执行器位。
constexpr uint16_t ACT_TURN_PUMP       = 0x0008;
constexpr uint16_t ACT_TURN_VALVE_E    = 0x0010;
constexpr uint16_t ACT_TURN_VALVE_F    = 0x0020;

// 浮沉子系统执行器位。
constexpr uint16_t ACT_BUOYANCY_PUMP   = 0x0040;
constexpr uint16_t ACT_BUOYANCY_VALVE_H= 0x0080;
constexpr uint16_t ACT_BUOYANCY_VALVE_I= 0x0100;

// 分组掩码，便于快速判断当前哪个子系统被激活。
constexpr uint16_t ACT_FORWARD_GROUP =
    ACT_FORWARD_PUMP | ACT_FORWARD_VALVE_B | ACT_FORWARD_VALVE_C;
constexpr uint16_t ACT_TURN_GROUP =
    ACT_TURN_PUMP | ACT_TURN_VALVE_E | ACT_TURN_VALVE_F;
constexpr uint16_t ACT_BUOYANCY_GROUP =
    ACT_BUOYANCY_PUMP | ACT_BUOYANCY_VALVE_H | ACT_BUOYANCY_VALVE_I;

// CRC8 校验函数，ESP32 和 Minima 两侧保持一致。
inline uint8_t calculateCRC8(uint8_t* data, uint8_t len) {
    uint8_t crc = 0x00;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

#endif // ESP32_PROTOCOL_H
