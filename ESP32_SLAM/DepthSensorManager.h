/**********************************************************************
 * DepthSensorManager.h
 *
 * MS5837-02BA 深度传感器管理器。
 * 通过 ESP32 I2C 读取气压/温度，换算深度后做卡尔曼滤波。
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

    bool isValid() const;
    float getDepthCm() const;
    float getRawDepthCm() const;
    float getDepthSpeedCmS() const;
    float getDepthAccelCmS2() const;
    float getTemperatureC() const;
    uint32_t getLastUpdate() const;
    const __FlashStringHelper* getStatusText() const;
    const __FlashStringHelper* getFailureText() const;
    void printDebug() const;

private:
    enum DebugStatus : uint8_t {
        DEBUG_NOT_STARTED = 0,
        DEBUG_I2C_INIT_FAILED,
        DEBUG_RESET_FAILED,
        DEBUG_PROM_READ_FAILED,
        DEBUG_PROM_CRC_FAILED,
        DEBUG_ADC_READ_FAILED,
        DEBUG_VALUE_OUT_OF_RANGE,
        DEBUG_VALID_DATA,
        DEBUG_STALE
    };

    enum ReadFailure : uint8_t {
        READ_OK = 0,
        READ_D1_TRIGGER_FAILED,
        READ_D1_FETCH_FAILED,
        READ_D2_TRIGGER_FAILED,
        READ_D2_FETCH_FAILED,
        READ_VALUE_OUT_OF_RANGE
    };

    float _rawDepthCm;
    float _filteredDepthCm;
    float _temperatureC;
    float _pressureMbar;
    float _zeroPressureMbar;
    float _depthSpeedCmS;
    float _depthAccelCmS2;
    bool _valid;
    bool _sensorReady;
    bool _filterInitialized;
    bool _zeroReferenceValid;
    bool _promCrcValid;
    uint32_t _lastUpdate;
    uint32_t _lastFilterUpdate;
    uint32_t _lastSampleMs;
    uint32_t _sampleCount;
    uint32_t _readErrorCount;
    uint32_t _lastD1;
    uint32_t _lastD2;
    uint8_t _lastPromReadIndex;
    uint8_t _lastI2cError;
    ReadFailure _lastReadFailure;
    uint16_t _prom[8];
    DebugStatus _debugStatus;

    // 非阻塞读取状态机（避免 delay(40) 阻塞 SPI 设备轮询）
    enum ReadPhase : uint8_t {
        PHASE_IDLE = 0,
        PHASE_WAIT_D1,
        PHASE_READ_D1,
        PHASE_WAIT_D2,
        PHASE_READ_D2
    };
    ReadPhase _readPhase;
    uint32_t _convStartMs;
    uint32_t _pendingD1;

    KalmanFilter _depthFilter;

    bool resetSensor();
    bool readProm();
    void printI2cScan();
    uint8_t calculatePromCrc(const uint16_t prom[8]) const;
    bool writeCommand(uint8_t command);
    bool readAdcResult(uint32_t& value);  // 仅读取 ADC 结果，不发转换命令
    bool readAdc(uint8_t conversionCommand, uint32_t& value, ReadFailure triggerFailure, ReadFailure fetchFailure);
    bool readMeasurement(float& pressureMbar, float& temperatureC);
    bool computeFromRaw(uint32_t d1, uint32_t d2, float& pressureMbar, float& temperatureC);
    float pressureToDepthCm(float pressureDeltaMbar) const;
    void commitDepth(float depthCm, float temperatureC, float pressureMbar);
    static const __FlashStringHelper* debugStatusString(DebugStatus status);
    static const __FlashStringHelper* readFailureString(ReadFailure failure);
};

#endif // ESP32_DEPTH_SENSOR_MANAGER_H
