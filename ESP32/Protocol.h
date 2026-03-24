/**********************************************************************
 * Protocol.h
 *
 * 这个文件定义 ESP32 侧使用的通信协议、引脚映射和公共常量。
 *********************************************************************/

// 防止头文件被重复包含。
#ifndef ESP32_PROTOCOL_H
// 定义头文件保护宏。
#define ESP32_PROTOCOL_H

// 引入 Arduino 基础类型和串口定义。
#include <Arduino.h>

// 指定 ESP32 用于连接 Minima 的硬件串口。
#define UART_TO_MINIMA        Serial1
// 指定 ESP32 与 Minima 之间的串口波特率。
#define UART_BAUD_RATE        2000000
// 指定 ESP32 发送到 Minima 的 TX 引脚。
#define UART_TX_PIN           17
// 指定 ESP32 接收自 Minima 的 RX 引脚。
#define UART_RX_PIN           18

// 指定 SPI 总线的 MOSI 引脚。
#define SPI_MOSI              11
// 指定 SPI 总线的 MISO 引脚。
#define SPI_MISO              13
// 指定 SPI 总线的时钟引脚。
#define SPI_SCK               12
// 指定 CH9434A 的片选引脚。
#define SPI_CS                10
// 指定 CH9434A 的中断输入引脚。
#define CH9434A_INT           14

// 把深度传感器分配到 CH9434A 的 UART0。
#define DEPTH_UART_CHANNEL    0
// 把前向超声波分配到 CH9434A 的 UART1。
#define ULTRASONIC_FRONT_UART 1
// 把左侧超声波分配到 CH9434A 的 UART2。
#define ULTRASONIC_LEFT_UART  2
// 把右侧超声波分配到 CH9434A 的 UART3。
#define ULTRASONIC_RIGHT_UART 3

// 定义控制帧的帧头字节。
#define FRAME_HEADER          0xAA
// 定义控制帧的帧尾字节。
#define FRAME_TAIL            0x55
// 定义固定控制帧总长度。
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
// 定义深度零点校准命令号。
#define CMD_CALIBRATE_DEPTH   0x07
// 定义显式开始前进命令号。
#define CMD_FORWARD_START     0x08
// 定义显式停止前进命令号。
#define CMD_FORWARD_STOP      0x09
// 定义显式停止转向命令号。
#define CMD_TURN_STOP         0x0A
// 定义显式停止浮沉命令号。
#define CMD_BUOYANCY_STOP     0x0B

// 定义 Minima 回传的运动状态命令号。
#define STATUS_MOTION         0x81
// 定义 Minima 回传的心跳状态命令号。
#define STATUS_HEARTBEAT      0x82
// 定义传感器状态命令号，当前架构中保留兼容。
#define STATUS_SENSOR_DATA    0x83

// 定义系统中超声波传感器的数量。
#define NUM_ULTRASONIC        3
// 定义超声波传感器的串口波特率。
#define ULTRASONIC_BAUDRATE   115200
// 定义超声波传感器读帧超时时间，单位毫秒。
#define ULTRASONIC_TIMEOUT    30
// 定义深度传感器的串口波特率。
#define DEPTH_UART_BAUDRATE   9600

// 定义前向超声波的逻辑编号。
#define SENSOR_FRONT          0
// 定义左侧超声波的逻辑编号。
#define SENSOR_LEFT           1
// 定义右侧超声波的逻辑编号。
#define SENSOR_RIGHT          2

// 定义 CRC8 计算函数，供协议打包和验包共用。
inline uint8_t calculateCRC8(uint8_t* data, uint8_t len) {
    // 初始化 CRC 寄存器。
    uint8_t crc = 0x00;
    // 逐字节处理输入数据。
    for (uint8_t i = 0; i < len; i++) {
        // 先把当前字节异或进 CRC。
        crc ^= data[i];
        // 对当前字节执行 8 次多项式迭代。
        for (uint8_t j = 0; j < 8; j++) {
            // 如果最高位为 1，就执行带多项式的移位。
            if (crc & 0x80) {
                // 左移后与多项式 0x07 异或。
                crc = (crc << 1) ^ 0x07;
            } else {
                // 如果最高位为 0，就只左移。
                crc <<= 1;
            }
        }
    }
    // 返回最终 CRC 结果。
    return crc;
}

// 结束头文件保护。
#endif // ESP32_PROTOCOL_H
