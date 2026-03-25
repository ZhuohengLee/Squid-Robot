/**********************************************************************
 * SensorHub.cpp
 *
 * Aggregates the basic sensor output shown on the ESP32 serial console.
 *********************************************************************/

#include "SensorHub.h"

namespace {
constexpr uint32_t DISPLAY_INTERVAL_MS = 1000;
const char* SENSOR_NAMES[NUM_ULTRASONIC] = {"Front", "Left ", "Right"};
}

SensorHub::SensorHub()
    : _depthMgr(nullptr),
      _ultrasonicMgr(nullptr),
      _lastDisplay(0) {}

void SensorHub::setDepthSensorManager(DepthSensorManager* manager) {
    _depthMgr = manager;
}

void SensorHub::setUltrasonicManager(UltrasonicManager* manager) {
    _ultrasonicMgr = manager;
}

void SensorHub::calibrateDepthZero() {
    if (_depthMgr) {
        _depthMgr->calibrateZero();
    }
}

void SensorHub::displayAll() {
    const uint32_t now = millis();
    if (now - _lastDisplay < DISPLAY_INTERVAL_MS) {
        return;
    }
    _lastDisplay = now;

    Serial.println(F("\n================ ALL SENSORS ================"));

    Serial.print(F("Depth: "));
    if (_depthMgr && _depthMgr->isValid()) {
        Serial.print(_depthMgr->getDepthCm(), 2);
        Serial.print(F(" cm | Temp: "));
        Serial.print(_depthMgr->getTemperatureC(), 1);
        Serial.println(F(" C"));
    } else if (!_depthMgr) {
        Serial.println(F("disabled"));
    } else {
        Serial.println(F("offline"));
    }

    if (_ultrasonicMgr) {
        for (uint8_t sensor = 0; sensor < NUM_ULTRASONIC; ++sensor) {
            Serial.print(F("Ultrasonic "));
            Serial.print(SENSOR_NAMES[sensor]);
            Serial.print(F(": "));

            if (_ultrasonicMgr->isValid(sensor)) {
                Serial.print(_ultrasonicMgr->getDistance(sensor) / 10.0f, 1);
                Serial.println(F(" cm"));
            } else {
                Serial.println(F("offline"));
            }
        }
    }

    Serial.println(F("=============================================\n"));
}

void SensorHub::displayCompact() {
    Serial.print(F("Sensors: depth="));
    if (_depthMgr && _depthMgr->isValid()) {
        Serial.print(_depthMgr->getDepthCm(), 1);
        Serial.print(F("cm"));
    } else {
        Serial.print(F("--"));
    }

    if (_ultrasonicMgr) {
        Serial.print(F(" | us="));
        for (uint8_t sensor = 0; sensor < NUM_ULTRASONIC; ++sensor) {
            if (_ultrasonicMgr->isValid(sensor)) {
                Serial.print(_ultrasonicMgr->getDistance(sensor) / 10.0f, 0);
            } else {
                Serial.print(F("--"));
            }

            if (sensor + 1 < NUM_ULTRASONIC) {
                Serial.print(' ');
            }
        }
    }

    Serial.println();
}

bool SensorHub::isHealthy() const {
    const bool depthOk = !_depthMgr || _depthMgr->isValid();
    uint8_t ultrasonicOk = 0;

    if (_ultrasonicMgr) {
        for (uint8_t sensor = 0; sensor < NUM_ULTRASONIC; ++sensor) {
            if (_ultrasonicMgr->isValid(sensor)) {
                ++ultrasonicOk;
            }
        }
    }

    return depthOk && ultrasonicOk >= 2;
}

bool SensorHub::hasDepthSensor() const {
    return _depthMgr != nullptr;
}

bool SensorHub::isDepthOnline() const {
    return _depthMgr && _depthMgr->isValid();
}

uint8_t SensorHub::getSensorCount() const {
    uint8_t count = 0;

    if (_depthMgr && _depthMgr->isValid()) {
        ++count;
    }

    if (_ultrasonicMgr) {
        for (uint8_t sensor = 0; sensor < NUM_ULTRASONIC; ++sensor) {
            if (_ultrasonicMgr->isValid(sensor)) {
                ++count;
            }
        }
    }

    return count;
}
