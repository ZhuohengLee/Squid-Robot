/**********************************************************************
 * SensorHub.h
 *
 * Aggregates sensor and Minima feedback output for the ESP32 console.
 *********************************************************************/

#ifndef ESP32_SENSOR_HUB_H
#define ESP32_SENSOR_HUB_H

#include <Arduino.h>
#include "DepthSensorManager.h"
#include "ImuManager.h"
#include "StatusDisplay.h"
#include "UltrasonicManager.h"

class SensorHub {
public:
    SensorHub();

    void setDepthSensorManager(DepthSensorManager* manager);
    void setStatusDisplay(StatusDisplay* display);
    void setUltrasonicManager(UltrasonicManager* manager);
    void setImuManager(ImuManager* manager);

    void calibrateDepthZero();
    void displayAll();
    void forceDisplayAll();  // 立即渲染一次（不受开关/限速影响）
    bool toggleDisplay();    // g 命令：切换全部传感器周期显示，返回新状态
    bool isDisplayOn() const { return _displayOn; }
    void displayCompact();
    bool isHealthy() const;
    bool hasDepthSensor() const;
    bool isDepthOnline() const;
    uint8_t getSensorCount() const;

    // 电池电压（均值滤波，每次调用都重新采样）
    float readBatteryVoltage() const;
    float getBatteryVoltage() const { return _lastBattV; }
    void  updateBattery();

private:
    void renderAll();   // 实际渲染 ALL SENSORS（不含开关/限速判断）

    DepthSensorManager* _depthMgr;
    StatusDisplay*      _statusDisplay;
    UltrasonicManager*  _ultrasonicMgr;
    ImuManager*         _imuMgr;
    bool     _displayOn = true;   // 默认开（DEBUG 默认打印全部传感器）
    uint32_t _lastDisplay;
    uint32_t _lastBattMs;
    float    _lastBattV;
};

#endif // ESP32_SENSOR_HUB_H
