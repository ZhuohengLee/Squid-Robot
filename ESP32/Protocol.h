/**********************************************************************
 * Protocol.h
 *
 * 杩欎釜鏂囦欢瀹氫箟 ESP32 渚т娇鐢ㄧ殑閫氫俊鍗忚銆佸紩鑴氭槧灏勫拰鎵ц鍣ㄤ綅鎺╃爜銆? * 褰撳墠鏋舵瀯閲岋紝ESP32 璐熻矗杩愬姩鐘舵€佹満锛孧inima 鍙礋璐ｆ寜浣嶆帺鐮佹墽琛岃緭鍑恒€? *********************************************************************/

// 中文逐行说明：下面这一行保留原始代码 -> #ifndef ESP32_PROTOCOL_H
#ifndef ESP32_PROTOCOL_H
// 中文逐行说明：下面这一行保留原始代码 -> #define ESP32_PROTOCOL_H
#define ESP32_PROTOCOL_H

// 中文逐行说明：下面这一行保留原始代码 -> #include <Arduino.h>
#include <Arduino.h>

// ESP32 鍜?Minima 鐨勪覆鍙ｉ摼璺€?#define UART_TO_MINIMA        Serial1
// 中文逐行说明：下面这一行保留原始代码 -> #define UART_BAUD_RATE        2000000
#define UART_BAUD_RATE        2000000
// 中文逐行说明：下面这一行保留原始代码 -> #define UART_TX_PIN           17
#define UART_TX_PIN           17
// 中文逐行说明：下面这一行保留原始代码 -> #define UART_RX_PIN           18
#define UART_RX_PIN           18

// ESP32 鍜?CH9434A 鐨?SPI 鎺ョ嚎銆?#define SPI_MOSI              11
// 中文逐行说明：下面这一行保留原始代码 -> #define SPI_MISO              13
#define SPI_MISO              13
// 中文逐行说明：下面这一行保留原始代码 -> #define SPI_SCK               12
#define SPI_SCK               12
// 中文逐行说明：下面这一行保留原始代码 -> #define SPI_CS                10
#define SPI_CS                10
// 中文逐行说明：下面这一行保留原始代码 -> #define CH9434A_INT           14
#define CH9434A_INT           14

// CH9434A 鐨?UART 閫氶亾鍒嗛厤銆?#define DEPTH_UART_CHANNEL    0
// 中文逐行说明：下面这一行保留原始代码 -> #define ULTRASONIC_FRONT_UART 1
#define ULTRASONIC_FRONT_UART 1
// 中文逐行说明：下面这一行保留原始代码 -> #define ULTRASONIC_LEFT_UART  2
#define ULTRASONIC_LEFT_UART  2
// 中文逐行说明：下面这一行保留原始代码 -> #define ULTRASONIC_RIGHT_UART 3
#define ULTRASONIC_RIGHT_UART 3

// 鍥哄畾闀垮害鍗忚甯ф牸寮忋€?#define FRAME_HEADER          0xAA
// 中文逐行说明：下面这一行保留原始代码 -> #define FRAME_TAIL            0x55
#define FRAME_TAIL            0x55
// 中文逐行说明：下面这一行保留原始代码 -> #define FRAME_LENGTH          8
#define FRAME_LENGTH          8

// 褰撳墠鍙繚鐣欎袱涓帶鍒跺懡浠わ細璁剧疆鎵ц鍣ㄨ緭鍑哄拰鍏ㄥ眬鎬ュ仠銆?#define CMD_SET_ACTUATORS     0x01
// 中文逐行说明：下面这一行保留原始代码 -> #define CMD_EMERGENCY_STOP    0x02
#define CMD_EMERGENCY_STOP    0x02

// Minima 鍥炰紶鐨勭姸鎬佸抚鍛戒护鍙枫€?#define STATUS_MOTION         0x81
// 中文逐行说明：下面这一行保留原始代码 -> #define STATUS_HEARTBEAT      0x82
#define STATUS_HEARTBEAT      0x82

// 浼犳劅鍣ㄦ暟閲忓拰涓插彛鍙傛暟銆?#define NUM_ULTRASONIC        3
// 中文逐行说明：下面这一行保留原始代码 -> #define ULTRASONIC_BAUDRATE   115200
#define ULTRASONIC_BAUDRATE   115200
// 中文逐行说明：下面这一行保留原始代码 -> #define ULTRASONIC_TIMEOUT    30
#define ULTRASONIC_TIMEOUT    30
// 中文逐行说明：下面这一行保留原始代码 -> #define DEPTH_UART_BAUDRATE   9600
#define DEPTH_UART_BAUDRATE   9600

// 瓒呭０娉㈤€昏緫缂栧彿銆?#define SENSOR_FRONT          0
// 中文逐行说明：下面这一行保留原始代码 -> #define SENSOR_LEFT           1
#define SENSOR_LEFT           1
// 中文逐行说明：下面这一行保留原始代码 -> #define SENSOR_RIGHT          2
#define SENSOR_RIGHT          2

// 鍓嶈繘瀛愮郴缁熸墽琛屽櫒浣嶃€?constexpr uint16_t ACT_FORWARD_PUMP    = 0x0001;
// 中文逐行说明：下面这一行保留原始代码 -> constexpr uint16_t ACT_FORWARD_VALVE_B = 0x0002;
constexpr uint16_t ACT_FORWARD_VALVE_B = 0x0002;
// 中文逐行说明：下面这一行保留原始代码 -> constexpr uint16_t ACT_FORWARD_VALVE_C = 0x0004;
constexpr uint16_t ACT_FORWARD_VALVE_C = 0x0004;

// 杞悜瀛愮郴缁熸墽琛屽櫒浣嶃€?constexpr uint16_t ACT_TURN_PUMP       = 0x0008;
// 中文逐行说明：下面这一行保留原始代码 -> constexpr uint16_t ACT_TURN_VALVE_E    = 0x0010;
constexpr uint16_t ACT_TURN_VALVE_E    = 0x0010;
// 中文逐行说明：下面这一行保留原始代码 -> constexpr uint16_t ACT_TURN_VALVE_F    = 0x0020;
constexpr uint16_t ACT_TURN_VALVE_F    = 0x0020;

// 娴矇瀛愮郴缁熸墽琛屽櫒浣嶃€?constexpr uint16_t ACT_BUOYANCY_PUMP   = 0x0040;
// 中文逐行说明：下面这一行保留原始代码 -> constexpr uint16_t ACT_BUOYANCY_VALVE_H= 0x0080;
constexpr uint16_t ACT_BUOYANCY_VALVE_H= 0x0080;
// 中文逐行说明：下面这一行保留原始代码 -> constexpr uint16_t ACT_BUOYANCY_VALVE_I= 0x0100;
constexpr uint16_t ACT_BUOYANCY_VALVE_I= 0x0100;

// 鍒嗙粍鎺╃爜锛屼究浜庡揩閫熷垽鏂綋鍓嶅摢涓瓙绯荤粺琚縺娲汇€?constexpr uint16_t ACT_FORWARD_GROUP =
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

// CRC8 鏍￠獙鍑芥暟锛孍SP32 鍜?Minima 涓や晶淇濇寔涓€鑷淬€?inline uint8_t calculateCRC8(uint8_t* data, uint8_t len) {
    // 中文逐行说明：下面这一行保留原始代码 -> uint8_t crc = 0x00;
    uint8_t crc = 0x00;
    // 中文逐行说明：下面这一行保留原始代码 -> for (uint8_t i = 0; i < len; i++) {
    for (uint8_t i = 0; i < len; i++) {
        // 中文逐行说明：下面这一行保留原始代码 -> crc ^= data[i];
        crc ^= data[i];
        // 中文逐行说明：下面这一行保留原始代码 -> for (uint8_t j = 0; j < 8; j++) {
        for (uint8_t j = 0; j < 8; j++) {
            // 中文逐行说明：下面这一行保留原始代码 -> if (crc & 0x80) {
            if (crc & 0x80) {
                // 中文逐行说明：下面这一行保留原始代码 -> crc = (crc << 1) ^ 0x07;
                crc = (crc << 1) ^ 0x07;
            // 中文逐行说明：下面这一行保留原始代码 -> } else {
            } else {
                // 中文逐行说明：下面这一行保留原始代码 -> crc <<= 1;
                crc <<= 1;
            // 中文逐行说明：下面这一行保留原始代码 -> }
            }
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
    // 中文逐行说明：下面这一行保留原始代码 -> return crc;
    return crc;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> #endif // ESP32_PROTOCOL_H
#endif // ESP32_PROTOCOL_H
