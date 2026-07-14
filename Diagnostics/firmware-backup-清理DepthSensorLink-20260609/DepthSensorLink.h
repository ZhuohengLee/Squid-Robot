/**********************************************************************
 * DepthSensorLink.h
 *
 * Minima 侧 MS5837-02BA 读取与回传。
 *********************************************************************/

#ifndef MINIMA_DEPTH_SENSOR_LINK_H
#define MINIMA_DEPTH_SENSOR_LINK_H

#include <Arduino.h>

class DepthSensorLink {
public:
    DepthSensorLink();

    bool begin();
    void update();
    void calibrateZero();

    bool fetchLatest(int16_t& depthMm, int8_t& temperatureC);

private:
    float _pressureMbar;
    float _zeroPressureMbar;
    float _depthCm;
    float _temperatureC;
    bool _sensorReady;
    bool _pressureValid;
    bool _zeroReferenceValid;
    bool _sampleReady;
    uint32_t _lastSampleMs;
    uint32_t _startupMs;
    uint16_t _prom[8];

    bool resetSensor();
    bool readProm();
    uint8_t calculatePromCrc(const uint16_t prom[8]) const;
    bool writeCommand(uint8_t command);
    bool readAdc(uint8_t conversionCommand, uint32_t& value);
    bool readMeasurement(float& pressureMbar, float& temperatureC);
    float pressureToDepthCm(float pressureDeltaMbar) const;
};

#endif // MINIMA_DEPTH_SENSOR_LINK_H
