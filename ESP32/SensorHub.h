/**********************************************************************
 * SensorHub.h
 *
 * 这个文件声明 ESP32 侧的多传感器汇总接口。
 *********************************************************************/

// 防止头文件被重复包含。
#ifndef ESP32_SENSOR_HUB_H
// 定义头文件保护宏。
#define ESP32_SENSOR_HUB_H

// 引入 Arduino 基础类型。
#include <Arduino.h>
// 引入深度传感器管理类定义。
#include "DepthSensorManager.h"
// 引入超声波管理类定义。
#include "UltrasonicManager.h"

// 声明传感器汇总类。
class SensorHub {
public:
    // 声明构造函数。
    SensorHub();

    // 声明注入深度传感器管理器的函数。
    void setDepthSensorManager(DepthSensorManager* manager);
    // 声明注入超声波管理器的函数。
    void setUltrasonicManager(UltrasonicManager* manager);

    // 声明执行深度零点校准的函数。
    void calibrateDepthZero();
    // 声明完整打印所有传感器状态的函数。
    void displayAll();
    // 声明紧凑打印所有传感器状态的函数。
    void displayCompact();
    // 声明执行传感器健康检查的函数。
    bool isHealthy() const;
    bool hasDepthSensor() const;
    bool isDepthOnline() const;
    // 声明返回当前有效传感器数量的函数。
    uint8_t getSensorCount() const;

private:
    // 保存深度传感器管理器指针。
    DepthSensorManager* _depthMgr;
    // 保存超声波管理器指针。
    UltrasonicManager* _ultrasonicMgr;
    // 保存上一次完整打印的时间戳。
    uint32_t _lastDisplay;
};

// 结束头文件保护。
#endif // ESP32_SENSOR_HUB_H
