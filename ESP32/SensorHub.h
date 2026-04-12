/**********************************************************************
 * SensorHub.h
 *
 * Aggregates sensor and Minima feedback output for the ESP32 console.
 *********************************************************************/

#ifndef ESP32_SENSOR_HUB_H
#define ESP32_SENSOR_HUB_H

#include <Arduino.h>
#include "DepthSensorManager.h"
#include "StatusDisplay.h"
#include "UltrasonicManager.h"

class SensorHub {
public:
    SensorHub();

    void setDepthSensorManager(DepthSensorManager* manager);
    void setStatusDisplay(StatusDisplay* display);
    void setUltrasonicManager(UltrasonicManager* manager);

    void calibrateDepthZero();
    void displayAll();
    void forceDisplayAll();  // 跳过速率限制，用于 HC-12 按需查询
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
    DepthSensorManager* _depthMgr;
    StatusDisplay*      _statusDisplay;
    UltrasonicManager*  _ultrasonicMgr;
    uint32_t _lastDisplay;
    uint32_t _lastBattMs;
    float    _lastBattV;
};

#endif // ESP32_SENSOR_HUB_H
