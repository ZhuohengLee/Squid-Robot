/**********************************************************************
 * UltrasonicManager.h
 *
 * 这个文件声明三路超声波传感器管理器。
 *********************************************************************/

// 防止头文件被重复包含。
#ifndef ESP32_ULTRASONIC_MANAGER_H
// 定义头文件保护宏。
#define ESP32_ULTRASONIC_MANAGER_H

// 引入 Arduino 基础类型。
#include <Arduino.h>
// 引入 CH9434A 驱动接口。
#include "CH9434A.h"
// 引入协议与通道分配常量。
#include "Protocol.h"

// 定义单路超声波传感器的运行状态结构体。
struct UltrasonicData {
    // 保存最近一次测得的距离，单位毫米。
    uint16_t distance_mm;
    // 标记当前距离值是否有效。
    bool valid;
    // 保存最近一次收到有效数据的时间戳。
    uint32_t lastUpdate;
    // 保存连续错误计数。
    uint8_t errorCount;
};

// 声明超声波管理类。
class UltrasonicManager {
public:
    // 声明构造函数，并要求显式传入 CH9434A 驱动对象。
    explicit UltrasonicManager(CH9434A* ch9434);

    // 声明初始化全部超声波串口的函数。
    bool begin();
    // 声明轮询全部超声波数据的函数。
    void update();

    // 声明读取某一路距离值的函数。
    uint16_t getDistance(uint8_t sensor) const;
    // 声明读取某一路数据有效性的函数。
    bool isValid(uint8_t sensor) const;
    // 声明读取某一路最后更新时间的函数。
    uint32_t getLastUpdate(uint8_t sensor) const;
    // 声明批量获取全部距离值的函数。
    void getAllDistances(uint16_t* distances) const;

    // 声明重置单路传感器状态的函数。
    void reset(uint8_t sensor);
    // 声明重置全部传感器状态的函数。
    void resetAll();

private:
    // 保存 CH9434A 驱动对象指针。
    CH9434A* _ch9434;
    // 保存 3 路超声波的状态数组。
    UltrasonicData _sensors[NUM_ULTRASONIC];
    // 保存每一路最近一次触发测量的时间。
    uint32_t _lastTrigger[NUM_ULTRASONIC];
    // 保存最近一次整体扫描时间。
    uint32_t _lastScanTime;

    // 声明读取某一路超声波数据的函数。
    bool readSensor(uint8_t sensor);
    // 声明触发某一路超声波测量的函数。
    void triggerMeasurement(uint8_t sensor);
    // 声明校验超声波返回帧的函数。
    bool validateFrame(const uint8_t* buffer) const;
    // 声明从数据帧中解析距离的函数。
    uint16_t parseDistance(const uint8_t* buffer) const;
    // 声明把逻辑方向映射成 UART 通道号的函数。
    uint8_t getUartChannel(uint8_t sensor) const;
};

// 结束头文件保护。
#endif // ESP32_ULTRASONIC_MANAGER_H
