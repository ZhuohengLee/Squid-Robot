/**********************************************************************
 * DepthSensorManager.h
 *
 * 这个文件声明通过 CH9434A 读取深度传感器的管理类。
 *********************************************************************/

// 防止头文件被重复包含。
#ifndef ESP32_DEPTH_SENSOR_MANAGER_H
// 定义头文件保护宏。
#define ESP32_DEPTH_SENSOR_MANAGER_H

// 引入 Arduino 基础类型。
#include <Arduino.h>
// 引入 CH9434A 驱动接口。
#include "CH9434A.h"

// 声明深度传感器管理类。
class DepthSensorManager {
public:
    // 声明构造函数，并要求显式传入桥接芯片对象和 UART 号。
    explicit DepthSensorManager(CH9434A* ch9434, uint8_t uartNum);

    // 声明初始化传感器串口的函数。
    bool begin();
    // 声明轮询并解析深度数据的函数。
    void update();
    // 声明把当前深度设为零点的函数。
    void calibrateZero();

    // 声明读取“当前数据是否有效”的函数。
    bool isValid() const;
    // 声明读取当前深度值的函数。
    float getDepthCm() const;
    // 声明读取当前温度值的函数。
    float getTemperatureC() const;
    // 声明读取最后更新时间戳的函数。
    uint32_t getLastUpdate() const;

private:
    // 保存 CH9434A 驱动对象指针。
    CH9434A* _ch9434;
    // 保存当前深度传感器所在的 UART 编号。
    uint8_t _uartNum;

    // 保存最近一次解析出的深度值，单位厘米。
    float _depthCm;
    // 保存最近一次解析出的温度值，单位摄氏度。
    float _temperatureC;
    // 保存深度零点偏移量，单位厘米。
    float _depthOffsetCm;
    // 标记当前深度数据是否有效。
    bool _valid;
    // 保存最后一次收到有效数据的时间戳。
    uint32_t _lastUpdate;

    // 保存文本协议解析缓冲区。
    char _lineBuffer[64];
    // 保存当前文本缓冲区已写入长度。
    uint8_t _lineLength;
    // 保存二进制帧缓冲区。
    uint8_t _binaryBuffer[4];
    // 保存当前二进制缓冲区已写入长度。
    uint8_t _binaryLength;

    // 声明解析一整行文本深度数据的函数。
    bool parseAsciiLine(const char* line);
    // 声明逐字节解析二进制深度帧的函数。
    bool parseBinaryByte(uint8_t byte);
    // 声明提交有效深度结果的函数。
    void commitDepth(float depthCm, float temperatureC, bool hasTemperature);
};

// 结束头文件保护。
#endif // ESP32_DEPTH_SENSOR_MANAGER_H
