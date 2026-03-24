/**********************************************************************
 * Protocol.h
 *
 * 杩欎釜鏂囦欢瀹氫箟 Minima 渚т娇鐢ㄧ殑鍥哄畾闀垮害鎺у埗甯у崗璁€? * 褰撳墠鍗忚閲岋紝ESP32 鍙笅鍙戝畬鏁存墽琛屽櫒鎺╃爜锛孧inima 鍙礋璐ｆ墽琛屻€? *********************************************************************/

// 中文逐行说明：下面这一行保留原始代码 -> #ifndef MINIMA_PROTOCOL_H
#ifndef MINIMA_PROTOCOL_H
// 中文逐行说明：下面这一行保留原始代码 -> #define MINIMA_PROTOCOL_H
#define MINIMA_PROTOCOL_H

// 中文逐行说明：下面这一行保留原始代码 -> #include <Arduino.h>
#include <Arduino.h>

// 中文逐行说明：下面这一行保留原始代码 -> #define UART_BAUD_RATE        2000000
#define UART_BAUD_RATE        2000000

// 中文逐行说明：下面这一行保留原始代码 -> #define FRAME_HEADER          0xAA
#define FRAME_HEADER          0xAA
// 中文逐行说明：下面这一行保留原始代码 -> #define FRAME_TAIL            0x55
#define FRAME_TAIL            0x55
// 中文逐行说明：下面这一行保留原始代码 -> #define FRAME_LENGTH          8
#define FRAME_LENGTH          8

// 中文逐行说明：下面这一行保留原始代码 -> #define CMD_SET_ACTUATORS     0x01
#define CMD_SET_ACTUATORS     0x01
// 中文逐行说明：下面这一行保留原始代码 -> #define CMD_EMERGENCY_STOP    0x02
#define CMD_EMERGENCY_STOP    0x02

// 中文逐行说明：下面这一行保留原始代码 -> #define STATUS_MOTION         0x81
#define STATUS_MOTION         0x81
// 中文逐行说明：下面这一行保留原始代码 -> #define STATUS_HEARTBEAT      0x82
#define STATUS_HEARTBEAT      0x82

// 中文逐行说明：下面这一行保留原始代码 -> constexpr uint16_t ACT_FORWARD_PUMP     = 0x0001;
constexpr uint16_t ACT_FORWARD_PUMP     = 0x0001;
// 中文逐行说明：下面这一行保留原始代码 -> constexpr uint16_t ACT_FORWARD_VALVE_B  = 0x0002;
constexpr uint16_t ACT_FORWARD_VALVE_B  = 0x0002;
// 中文逐行说明：下面这一行保留原始代码 -> constexpr uint16_t ACT_FORWARD_VALVE_C  = 0x0004;
constexpr uint16_t ACT_FORWARD_VALVE_C  = 0x0004;
// 中文逐行说明：下面这一行保留原始代码 -> constexpr uint16_t ACT_TURN_PUMP        = 0x0008;
constexpr uint16_t ACT_TURN_PUMP        = 0x0008;
// 中文逐行说明：下面这一行保留原始代码 -> constexpr uint16_t ACT_TURN_VALVE_E     = 0x0010;
constexpr uint16_t ACT_TURN_VALVE_E     = 0x0010;
// 中文逐行说明：下面这一行保留原始代码 -> constexpr uint16_t ACT_TURN_VALVE_F     = 0x0020;
constexpr uint16_t ACT_TURN_VALVE_F     = 0x0020;
// 中文逐行说明：下面这一行保留原始代码 -> constexpr uint16_t ACT_BUOYANCY_PUMP    = 0x0040;
constexpr uint16_t ACT_BUOYANCY_PUMP    = 0x0040;
// 中文逐行说明：下面这一行保留原始代码 -> constexpr uint16_t ACT_BUOYANCY_VALVE_H = 0x0080;
constexpr uint16_t ACT_BUOYANCY_VALVE_H = 0x0080;
// 中文逐行说明：下面这一行保留原始代码 -> constexpr uint16_t ACT_BUOYANCY_VALVE_I = 0x0100;
constexpr uint16_t ACT_BUOYANCY_VALVE_I = 0x0100;

// 中文逐行说明：下面这一行保留原始代码 -> constexpr uint16_t ACT_FORWARD_GROUP =
constexpr uint16_t ACT_FORWARD_GROUP =
    // 中文逐行说明：下面这一行保留原始代码 -> ACT_FORWARD_PUMP | ACT_FORWARD_VALVE_B | ACT_FORWARD_VALVE_C;
    ACT_FORWARD_PUMP | ACT_FORWARD_VALVE_B | ACT_FORWARD_VALVE_C;
// 中文逐行说明：下面这一行保留原始代码 -> constexpr uint16_t ACT_TURN_GROUP =
constexpr uint16_t ACT_TURN_GROUP =
    // 中文逐行说明：下面这一行保留原始代码 -> ACT_TURN_PUMP | ACT_TURN_VALVE_E | ACT_TURN_VALVE_F;
    ACT_TURN_PUMP | ACT_TURN_VALVE_E | ACT_TURN_VALVE_F;
// 中文逐行说明：下面这一行保留原始代码 -> constexpr uint16_t ACT_BUOYANCY_GROUP =
constexpr uint16_t ACT_BUOYANCY_GROUP =
    // 中文逐行说明：下面这一行保留原始代码 -> ACT_BUOYANCY_PUMP | ACT_BUOYANCY_VALVE_H | ACT_BUOYANCY_VALVE_I;
    ACT_BUOYANCY_PUMP | ACT_BUOYANCY_VALVE_H | ACT_BUOYANCY_VALVE_I;

// 中文逐行说明：下面这一行保留原始代码 -> struct ProtocolFrame {
struct ProtocolFrame {
    // 中文逐行说明：下面这一行保留原始代码 -> uint8_t cmd;
    uint8_t cmd;
    // 中文逐行说明：下面这一行保留原始代码 -> uint8_t data0;
    uint8_t data0;
    // 中文逐行说明：下面这一行保留原始代码 -> uint8_t data1;
    uint8_t data1;
    // 中文逐行说明：下面这一行保留原始代码 -> uint8_t data2;
    uint8_t data2;
// 中文逐行说明：下面这一行保留原始代码 -> };
};

// 中文逐行说明：下面这一行保留原始代码 -> uint8_t calculateCRC8(const uint8_t* data, uint8_t len);
uint8_t calculateCRC8(const uint8_t* data, uint8_t len);
// 中文逐行说明：下面这一行保留原始代码 -> void sendFrame(Stream& port, uint8_t cmd, uint8_t data0 = 0, uint8_t data1 = 0, uint8_t data2 = 0);
void sendFrame(Stream& port, uint8_t cmd, uint8_t data0 = 0, uint8_t data1 = 0, uint8_t data2 = 0);

// 中文逐行说明：下面这一行保留原始代码 -> class ProtocolReceiver {
class ProtocolReceiver {
// 中文逐行说明：下面这一行保留原始代码 -> public:
public:
    // 中文逐行说明：下面这一行保留原始代码 -> ProtocolReceiver();
    ProtocolReceiver();
    // 中文逐行说明：下面这一行保留原始代码 -> bool poll(Stream& port, ProtocolFrame& frame);
    bool poll(Stream& port, ProtocolFrame& frame);

// 中文逐行说明：下面这一行保留原始代码 -> private:
private:
    // 中文逐行说明：下面这一行保留原始代码 -> uint8_t rxBuffer[FRAME_LENGTH];
    uint8_t rxBuffer[FRAME_LENGTH];
    // 中文逐行说明：下面这一行保留原始代码 -> uint8_t rxIndex;
    uint8_t rxIndex;
// 中文逐行说明：下面这一行保留原始代码 -> };
};

// 中文逐行说明：下面这一行保留原始代码 -> #endif // MINIMA_PROTOCOL_H
#endif // MINIMA_PROTOCOL_H
