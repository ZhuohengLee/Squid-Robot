/**********************************************************************
 * DepthSensorManager.h
 *
 * 这个文件声明通过 CH9434A 读取深度传感器的管理类。
 * 管理类会在内部对深度数据做卡尔曼滤波，再对外提供滤波结果。
 *********************************************************************/

#ifndef ESP32_DEPTH_SENSOR_MANAGER_H
#define ESP32_DEPTH_SENSOR_MANAGER_H

#include <Arduino.h>
#include "CH9434A.h"
#include "KalmanFilter.h"

class DepthSensorManager {
public:
    explicit DepthSensorManager(CH9434A* ch9434, uint8_t uartNum);

    bool begin();
    void update();
    void calibrateZero();

    bool isValid() const;
    float getDepthCm() const;
    float getRawDepthCm() const;
    float getDepthSpeedCmS() const;
    float getTemperatureC() const;
    uint32_t getLastUpdate() const;

private:
    CH9434A* _ch9434;
    uint8_t _uartNum;

    float _rawDepthCm;
    float _filteredDepthCm;
    float _temperatureC;
    float _depthOffsetCm;
    float _depthSpeedCmS;
    bool _valid;
    bool _filterInitialized;
    uint32_t _lastUpdate;
    uint32_t _lastFilterUpdate;

    char _lineBuffer[64];
    uint8_t _lineLength;
    uint8_t _binaryBuffer[4];
    uint8_t _binaryLength;

    KalmanFilter _depthFilter;

    bool parseAsciiLine(const char* line);
    bool parseBinaryByte(uint8_t byte);
    void commitDepth(float depthCm, float temperatureC, bool hasTemperature);
};

#endif // ESP32_DEPTH_SENSOR_MANAGER_H
