/**********************************************************************
 * DepthSensorManager.h
 *
 * 接收 Minima 回传的深度数据，并在 ESP32 侧做卡尔曼滤波。
 *********************************************************************/

#ifndef ESP32_DEPTH_SENSOR_MANAGER_H
#define ESP32_DEPTH_SENSOR_MANAGER_H

#include <Arduino.h>
#include "KalmanFilter.h"

class DepthSensorManager {
public:
    DepthSensorManager();

    bool begin();
    void update();
    void calibrateZero();
    void ingestMeasurement(int16_t depthMm, int8_t temperatureC);

    bool isValid() const;
    float getDepthCm() const;
    float getRawDepthCm() const;
    float getDepthSpeedCmS() const;
    float getTemperatureC() const;
    uint32_t getLastUpdate() const;
    void printDebug() const;

private:
    enum DebugStatus : uint8_t {
        DEBUG_NO_DATA = 0,
        DEBUG_VALID,
        DEBUG_STALE
    };

    float _rawDepthCm;
    float _filteredDepthCm;
    float _temperatureC;
    float _depthSpeedCmS;
    bool _valid;
    bool _filterInitialized;
    uint32_t _lastUpdate;
    uint32_t _lastFilterUpdate;
    uint32_t _frameCount;
    int16_t _lastDepthMm;
    DebugStatus _debugStatus;

    KalmanFilter _depthFilter;

    static const __FlashStringHelper* debugStatusString(DebugStatus status);
};

#endif // ESP32_DEPTH_SENSOR_MANAGER_H
