/**********************************************************************
 * DepthSensorLink.cpp
 *
 * Minima 侧 MS5837-02BA 采集实现。
 *********************************************************************/

#include "DepthSensorLink.h"
#include <Wire.h>

namespace {
constexpr uint8_t MS5837_ADDRESS = 0x76;
constexpr uint8_t MS5837_CMD_RESET = 0x1E;
constexpr uint8_t MS5837_CMD_ADC_READ = 0x00;
constexpr uint8_t MS5837_CMD_CONVERT_D1_8192 = 0x4A;
constexpr uint8_t MS5837_CMD_CONVERT_D2_8192 = 0x5A;

constexpr uint32_t MS5837_RESET_DELAY_MS = 10;
constexpr uint32_t MS5837_CONVERSION_DELAY_MS = 20;
constexpr uint32_t SAMPLE_INTERVAL_MS = 100;
constexpr uint32_t STARTUP_ZERO_DELAY_MS = 2000;

constexpr float FRESH_WATER_DENSITY_KG_M3 = 997.0f;
constexpr float GRAVITY_M_S2 = 9.80665f;
constexpr float CM_PER_MBAR = 10000.0f / (FRESH_WATER_DENSITY_KG_M3 * GRAVITY_M_S2);
}

DepthSensorLink::DepthSensorLink()
    : _pressureMbar(0.0f),
      _zeroPressureMbar(0.0f),
      _depthCm(0.0f),
      _temperatureC(0.0f),
      _sensorReady(false),
      _pressureValid(false),
      _zeroReferenceValid(false),
      _sampleReady(false),
      _lastSampleMs(0),
      _startupMs(0) {
    for (uint8_t i = 0; i < 8; ++i) {
        _prom[i] = 0;
    }
}

bool DepthSensorLink::begin() {
    Wire.begin();
    Wire.setClock(400000);

    _sensorReady = false;
    _pressureValid = false;
    _zeroReferenceValid = false;
    _sampleReady = false;
    _lastSampleMs = 0;
    _startupMs = millis();

    if (!resetSensor()) {
        return false;
    }

    if (!readProm()) {
        return false;
    }

    _sensorReady = true;
    return true;
}

void DepthSensorLink::update() {
    const uint32_t now = millis();
    if (!_sensorReady || (_lastSampleMs != 0 && now - _lastSampleMs < SAMPLE_INTERVAL_MS)) {
        return;
    }

    _lastSampleMs = now;

    float pressureMbar = 0.0f;
    float temperatureC = 0.0f;
    if (!readMeasurement(pressureMbar, temperatureC)) {
        return;
    }

    _pressureMbar = pressureMbar;
    _temperatureC = temperatureC;
    _pressureValid = true;

    if (!_zeroReferenceValid && now - _startupMs >= STARTUP_ZERO_DELAY_MS) {
        _zeroPressureMbar = _pressureMbar;
        _zeroReferenceValid = true;
    }

    if (!_zeroReferenceValid) {
        return;
    }

    _depthCm = pressureToDepthCm(_pressureMbar - _zeroPressureMbar);
    _sampleReady = true;
}

void DepthSensorLink::calibrateZero() {
    if (!_pressureValid) {
        return;
    }

    _zeroPressureMbar = _pressureMbar;
    _zeroReferenceValid = true;
    _depthCm = 0.0f;
    _sampleReady = true;
}

bool DepthSensorLink::fetchLatest(int16_t& depthMm, int8_t& temperatureC) {
    if (!_sampleReady) {
        return false;
    }

    depthMm = static_cast<int16_t>(lroundf(_depthCm * 10.0f));
    temperatureC = static_cast<int8_t>(lroundf(_temperatureC));
    _sampleReady = false;
    return true;
}

bool DepthSensorLink::resetSensor() {
    if (!writeCommand(MS5837_CMD_RESET)) {
        return false;
    }

    delay(MS5837_RESET_DELAY_MS);
    return true;
}

bool DepthSensorLink::readProm() {
    for (uint8_t index = 0; index < 8; ++index) {
        Wire.beginTransmission(MS5837_ADDRESS);
        Wire.write(static_cast<uint8_t>(0xA0 + index * 2));
        if (Wire.endTransmission(false) != 0) {
            return false;
        }

        const uint8_t bytesRead = Wire.requestFrom(static_cast<int>(MS5837_ADDRESS), 2);
        if (bytesRead != 2) {
            return false;
        }

        _prom[index] = static_cast<uint16_t>(Wire.read() << 8);
        _prom[index] |= static_cast<uint16_t>(Wire.read());
    }

    const uint8_t expectedCrc = static_cast<uint8_t>(_prom[0] >> 12);
    return expectedCrc == calculatePromCrc(_prom);
}

uint8_t DepthSensorLink::calculatePromCrc(const uint16_t prom[8]) const {
    uint16_t promCopy[8];
    for (uint8_t i = 0; i < 8; ++i) {
        promCopy[i] = prom[i];
    }

    uint16_t remainder = 0;
    promCopy[0] &= 0x0FFF;
    promCopy[7] = 0;

    for (uint8_t count = 0; count < 16; ++count) {
        if (count & 1U) {
            remainder ^= static_cast<uint16_t>(promCopy[count >> 1] & 0x00FF);
        } else {
            remainder ^= static_cast<uint16_t>(promCopy[count >> 1] >> 8);
        }

        for (uint8_t bit = 0; bit < 8; ++bit) {
            if (remainder & 0x8000) {
                remainder = static_cast<uint16_t>((remainder << 1) ^ 0x3000);
            } else {
                remainder <<= 1;
            }
        }
    }

    return static_cast<uint8_t>((remainder >> 12) & 0x0F);
}

bool DepthSensorLink::writeCommand(uint8_t command) {
    Wire.beginTransmission(MS5837_ADDRESS);
    Wire.write(command);
    return Wire.endTransmission() == 0;
}

bool DepthSensorLink::readAdc(uint8_t conversionCommand, uint32_t& value) {
    if (!writeCommand(conversionCommand)) {
        return false;
    }

    delay(MS5837_CONVERSION_DELAY_MS);

    Wire.beginTransmission(MS5837_ADDRESS);
    Wire.write(MS5837_CMD_ADC_READ);
    if (Wire.endTransmission(false) != 0) {
        return false;
    }

    const uint8_t bytesRead = Wire.requestFrom(static_cast<int>(MS5837_ADDRESS), 3);
    if (bytesRead != 3) {
        return false;
    }

    value = static_cast<uint32_t>(Wire.read()) << 16;
    value |= static_cast<uint32_t>(Wire.read()) << 8;
    value |= static_cast<uint32_t>(Wire.read());
    return value != 0;
}

bool DepthSensorLink::readMeasurement(float& pressureMbar, float& temperatureC) {
    uint32_t d1 = 0;
    uint32_t d2 = 0;
    if (!readAdc(MS5837_CMD_CONVERT_D1_8192, d1) ||
        !readAdc(MS5837_CMD_CONVERT_D2_8192, d2)) {
        return false;
    }

    const int32_t dT = static_cast<int32_t>(d2) - (static_cast<int32_t>(_prom[5]) << 8);
    int64_t temp = 2000LL + ((static_cast<int64_t>(dT) * _prom[6]) >> 23);
    int64_t off = (static_cast<int64_t>(_prom[2]) << 17) +
                  ((static_cast<int64_t>(_prom[4]) * dT) >> 6);
    int64_t sens = (static_cast<int64_t>(_prom[1]) << 16) +
                   ((static_cast<int64_t>(_prom[3]) * dT) >> 7);

    if (temp < 2000) {
        const int64_t tempDelta = temp - 2000LL;
        temp -= (11LL * static_cast<int64_t>(dT) * static_cast<int64_t>(dT)) >> 35;
        off -= (31LL * tempDelta * tempDelta) >> 3;
        sens -= (63LL * tempDelta * tempDelta) >> 5;
    }

    const int64_t pressureRaw = ((((static_cast<int64_t>(d1) * sens) >> 21) - off) >> 15);
    pressureMbar = static_cast<float>(pressureRaw) * 0.01f;
    temperatureC = static_cast<float>(temp) * 0.01f;

    return pressureMbar >= 10.0f && pressureMbar <= 2000.0f &&
           temperatureC >= -40.0f && temperatureC <= 85.0f;
}

float DepthSensorLink::pressureToDepthCm(float pressureDeltaMbar) const {
    return pressureDeltaMbar * CM_PER_MBAR;
}
