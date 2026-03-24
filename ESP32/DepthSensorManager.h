/**********************************************************************
 * DepthSensorManager.h
 *
 * 杩欎釜鏂囦欢澹版槑閫氳繃 CH9434A 璇诲彇娣卞害浼犳劅鍣ㄧ殑绠＄悊绫汇€? * 绠＄悊绫讳細鍦ㄥ唴閮ㄥ娣卞害鏁版嵁鍋氬崱灏旀浖婊ゆ尝锛屽啀瀵瑰鎻愪緵婊ゆ尝缁撴灉銆? *********************************************************************/

// 中文逐行说明：下面这一行保留原始代码 -> #ifndef ESP32_DEPTH_SENSOR_MANAGER_H
#ifndef ESP32_DEPTH_SENSOR_MANAGER_H
// 中文逐行说明：下面这一行保留原始代码 -> #define ESP32_DEPTH_SENSOR_MANAGER_H
#define ESP32_DEPTH_SENSOR_MANAGER_H

// 中文逐行说明：下面这一行保留原始代码 -> #include <Arduino.h>
#include <Arduino.h>
// 中文逐行说明：下面这一行保留原始代码 -> #include "CH9434A.h"
#include "CH9434A.h"
// 中文逐行说明：下面这一行保留原始代码 -> #include "KalmanFilter.h"
#include "KalmanFilter.h"

// 中文逐行说明：下面这一行保留原始代码 -> class DepthSensorManager {
class DepthSensorManager {
// 中文逐行说明：下面这一行保留原始代码 -> public:
public:
    // 中文逐行说明：下面这一行保留原始代码 -> explicit DepthSensorManager(CH9434A* ch9434, uint8_t uartNum);
    explicit DepthSensorManager(CH9434A* ch9434, uint8_t uartNum);

    // 中文逐行说明：下面这一行保留原始代码 -> bool begin();
    bool begin();
    // 中文逐行说明：下面这一行保留原始代码 -> void update();
    void update();
    // 中文逐行说明：下面这一行保留原始代码 -> void calibrateZero();
    void calibrateZero();

    // 中文逐行说明：下面这一行保留原始代码 -> bool isValid() const;
    bool isValid() const;
    // 中文逐行说明：下面这一行保留原始代码 -> float getDepthCm() const;
    float getDepthCm() const;
    // 中文逐行说明：下面这一行保留原始代码 -> float getRawDepthCm() const;
    float getRawDepthCm() const;
    // 中文逐行说明：下面这一行保留原始代码 -> float getDepthSpeedCmS() const;
    float getDepthSpeedCmS() const;
    // 中文逐行说明：下面这一行保留原始代码 -> float getTemperatureC() const;
    float getTemperatureC() const;
    // 中文逐行说明：下面这一行保留原始代码 -> uint32_t getLastUpdate() const;
    uint32_t getLastUpdate() const;

// 中文逐行说明：下面这一行保留原始代码 -> private:
private:
    // 中文逐行说明：下面这一行保留原始代码 -> CH9434A* _ch9434;
    CH9434A* _ch9434;
    // 中文逐行说明：下面这一行保留原始代码 -> uint8_t _uartNum;
    uint8_t _uartNum;

    // 中文逐行说明：下面这一行保留原始代码 -> float _rawDepthCm;
    float _rawDepthCm;
    // 中文逐行说明：下面这一行保留原始代码 -> float _filteredDepthCm;
    float _filteredDepthCm;
    // 中文逐行说明：下面这一行保留原始代码 -> float _temperatureC;
    float _temperatureC;
    // 中文逐行说明：下面这一行保留原始代码 -> float _depthOffsetCm;
    float _depthOffsetCm;
    // 中文逐行说明：下面这一行保留原始代码 -> float _depthSpeedCmS;
    float _depthSpeedCmS;
    // 中文逐行说明：下面这一行保留原始代码 -> bool _valid;
    bool _valid;
    // 中文逐行说明：下面这一行保留原始代码 -> bool _filterInitialized;
    bool _filterInitialized;
    // 中文逐行说明：下面这一行保留原始代码 -> uint32_t _lastUpdate;
    uint32_t _lastUpdate;
    // 中文逐行说明：下面这一行保留原始代码 -> uint32_t _lastFilterUpdate;
    uint32_t _lastFilterUpdate;

    // 中文逐行说明：下面这一行保留原始代码 -> char _lineBuffer[64];
    char _lineBuffer[64];
    // 中文逐行说明：下面这一行保留原始代码 -> uint8_t _lineLength;
    uint8_t _lineLength;
    // 中文逐行说明：下面这一行保留原始代码 -> uint8_t _binaryBuffer[4];
    uint8_t _binaryBuffer[4];
    // 中文逐行说明：下面这一行保留原始代码 -> uint8_t _binaryLength;
    uint8_t _binaryLength;

    // 中文逐行说明：下面这一行保留原始代码 -> KalmanFilter _depthFilter;
    KalmanFilter _depthFilter;

    // 中文逐行说明：下面这一行保留原始代码 -> bool parseAsciiLine(const char* line);
    bool parseAsciiLine(const char* line);
    // 中文逐行说明：下面这一行保留原始代码 -> bool parseBinaryByte(uint8_t byte);
    bool parseBinaryByte(uint8_t byte);
    // 中文逐行说明：下面这一行保留原始代码 -> void commitDepth(float depthCm, float temperatureC, bool hasTemperature);
    void commitDepth(float depthCm, float temperatureC, bool hasTemperature);
// 中文逐行说明：下面这一行保留原始代码 -> };
};

// 中文逐行说明：下面这一行保留原始代码 -> #endif // ESP32_DEPTH_SENSOR_MANAGER_H
#endif // ESP32_DEPTH_SENSOR_MANAGER_H
