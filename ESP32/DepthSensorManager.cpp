/**********************************************************************
 * DepthSensorManager.cpp
 *
 * Minima 深度回传接收与滤波实现。
 *********************************************************************/

#include "DepthSensorManager.h"

namespace {
constexpr uint32_t DATA_TIMEOUT_MS = 2000;
}

DepthSensorManager::DepthSensorManager()
    : _rawDepthCm(0.0f),
      _filteredDepthCm(0.0f),
      _temperatureC(0.0f),
      _depthSpeedCmS(0.0f),
      _valid(false),
      _filterInitialized(false),
      _lastUpdate(0),
      _lastFilterUpdate(0),
      _frameCount(0),
      _lastDepthMm(0),
      _debugStatus(DEBUG_NO_DATA) {}

bool DepthSensorManager::begin() {
    _rawDepthCm = 0.0f;
    _filteredDepthCm = 0.0f;
    _temperatureC = 0.0f;
    _depthSpeedCmS = 0.0f;
    _valid = false;
    _filterInitialized = false;
    _lastUpdate = 0;
    _lastFilterUpdate = 0;
    _frameCount = 0;
    _lastDepthMm = 0;
    _debugStatus = DEBUG_NO_DATA;
    _depthFilter.reset();
    return true;
}

void DepthSensorManager::update() {
    if (_valid && millis() - _lastUpdate > DATA_TIMEOUT_MS) {
        _valid = false;
        _filterInitialized = false;
        _depthSpeedCmS = 0.0f;
        _debugStatus = DEBUG_STALE;
    }
}

void DepthSensorManager::calibrateZero() {
    _rawDepthCm = 0.0f;
    _filteredDepthCm = 0.0f;
    _depthSpeedCmS = 0.0f;
    _filterInitialized = false;
    _depthFilter.reset();
}

void DepthSensorManager::ingestMeasurement(int16_t depthMm, int8_t temperatureC) {
    const uint32_t now = millis();
    const float depthCm = static_cast<float>(depthMm) * 0.1f;

    _lastDepthMm = depthMm;
    _temperatureC = static_cast<float>(temperatureC);
    _rawDepthCm = depthCm;

    if (!_filterInitialized) {
        _depthFilter.reset(depthCm, 0.0f);
        _filteredDepthCm = depthCm;
        _depthSpeedCmS = 0.0f;
        _filterInitialized = true;
    } else {
        float dt = static_cast<float>(now - _lastFilterUpdate) * 0.001f;
        if (dt < 0.01f) {
            dt = 0.01f;
        }
        if (dt > 0.20f) {
            dt = 0.20f;
        }

        _depthFilter.update(depthCm, dt);
        _filteredDepthCm = _depthFilter.getPosition();
        _depthSpeedCmS = _depthFilter.getVelocity();
    }

    _lastUpdate = now;
    _lastFilterUpdate = now;
    _valid = true;
    _debugStatus = DEBUG_VALID;
    ++_frameCount;
}

bool DepthSensorManager::isValid() const {
    return _valid;
}

float DepthSensorManager::getDepthCm() const {
    return _filteredDepthCm;
}

float DepthSensorManager::getRawDepthCm() const {
    return _rawDepthCm;
}

float DepthSensorManager::getDepthSpeedCmS() const {
    return _depthSpeedCmS;
}

float DepthSensorManager::getTemperatureC() const {
    return _temperatureC;
}

uint32_t DepthSensorManager::getLastUpdate() const {
    return _lastUpdate;
}

void DepthSensorManager::printDebug() const {
    Serial.print(F("Depth Link: frames="));
    Serial.print(_frameCount);
    Serial.print(F(" | status="));
    Serial.print(debugStatusString(_debugStatus));
    Serial.print(F(" | last_mm="));
    Serial.print(_lastDepthMm);
    Serial.print(F(" | temp="));
    Serial.print(_temperatureC, 1);
    Serial.println(F(" C"));
}

const __FlashStringHelper* DepthSensorManager::debugStatusString(DebugStatus status) {
    switch (status) {
        case DEBUG_NO_DATA:
            return F("no_data");
        case DEBUG_VALID:
            return F("valid");
        case DEBUG_STALE:
            return F("stale");
        default:
            return F("unknown");
    }
}
