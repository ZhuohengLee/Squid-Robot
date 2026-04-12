/**********************************************************************
 * SensorHub.cpp
 *
 * Aggregates the basic sensor output shown on the ESP32 serial console.
 *********************************************************************/

#include "SensorHub.h"
#include "Protocol.h"
#include "TeeStream.h"

namespace {
constexpr uint32_t DISPLAY_INTERVAL_MS = 1000;
constexpr uint32_t BATT_UPDATE_MS      = 2000;  // 电压每 2 秒采样一次
const char* SENSOR_NAMES[NUM_ULTRASONIC] = {"Front", "Left ", "Right"};

void printMotionFlags(uint8_t status) {
    if (status & 0x01) g_dbg->print(F("FWD "));
    if (status & 0x02) g_dbg->print(F("TURN "));
    if (status & 0x04) g_dbg->print(F("BUOY "));
    if (status == 0) g_dbg->print(F("IDLE"));
}
}

SensorHub::SensorHub()
    : _depthMgr(nullptr),
      _statusDisplay(nullptr),
      _ultrasonicMgr(nullptr),
      _lastDisplay(0),
      _lastBattMs(0),
      _lastBattV(0.0f) {
    analogSetAttenuation(ADC_11db);
}

void SensorHub::setDepthSensorManager(DepthSensorManager* manager) {
    _depthMgr = manager;
}

void SensorHub::setStatusDisplay(StatusDisplay* display) {
    _statusDisplay = display;
}

void SensorHub::setUltrasonicManager(UltrasonicManager* manager) {
    _ultrasonicMgr = manager;
}

float SensorHub::readBatteryVoltage() const {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < BATT_SAMPLES; i++) {
        sum += analogRead(BATT_ADC_PIN);
        delayMicroseconds(200);
    }
    float vgpio = (float)(sum / BATT_SAMPLES) / BATT_ADC_MAX * BATT_ADC_REF_V;
    return vgpio * BATT_DIVIDER_RATIO;
}

void SensorHub::updateBattery() {
    const uint32_t now = millis();
    if (now - _lastBattMs >= BATT_UPDATE_MS) {
        _lastBattMs = now;
        _lastBattV  = readBatteryVoltage();
    }
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

    g_dbg->println(F("\n================ ALL SENSORS ================"));

    g_dbg->print(F("Depth: "));
    if (_depthMgr) {
        g_dbg->print(_depthMgr->getDepthCm(), 2);
        g_dbg->print(F(" cm | vz="));
        g_dbg->print(_depthMgr->getDepthSpeedCmS(), 2);
        g_dbg->print(F(" cm/s | az="));
        g_dbg->print(_depthMgr->getDepthAccelCmS2(), 2);
        g_dbg->println(F(" cm/s^2"));
    } else {
        g_dbg->println(F("disabled"));
    }

    if (_ultrasonicMgr) {
        for (uint8_t sensor = 0; sensor < NUM_ULTRASONIC; ++sensor) {
            g_dbg->print(F("Ultrasonic "));
            g_dbg->print(SENSOR_NAMES[sensor]);
            g_dbg->print(F(": "));

            if (_ultrasonicMgr->isValid(sensor)) {
                g_dbg->print(_ultrasonicMgr->getDistance(sensor) / 10.0f, 1);
                g_dbg->println(F(" cm"));
            } else {
                g_dbg->println(F("offline"));
            }
        }
    }

    if (_statusDisplay) {
        g_dbg->print(F("Minima: motion="));
        printMotionFlags(_statusDisplay->getLastMotionStatus());
        g_dbg->println();
    }

    // 电池电压
    g_dbg->print(F("Battery: "));
    g_dbg->print(_lastBattV, 2);
    g_dbg->print(F(" V"));
    if      (_lastBattV >= 12.0f) g_dbg->print(F("  [满电]"));
    else if (_lastBattV >= 11.1f) g_dbg->print(F("  [正常]"));
    else if (_lastBattV >= 10.5f) g_dbg->print(F("  [偏低]"));
    else if (_lastBattV >  5.0f)  g_dbg->print(F("  [⚠ 低电！]"));
    else                          g_dbg->print(F("  [未检测]"));
    g_dbg->println();

    g_dbg->println(F("=============================================\n"));
}

void SensorHub::forceDisplayAll() {
    _lastDisplay = 0;  // 重置计时器，让 displayAll() 立即执行
    displayAll();
}

void SensorHub::displayCompact() {
    g_dbg->print(F("Sensors: depth="));
    if (_depthMgr) {
        g_dbg->print(_depthMgr->getDepthCm(), 1);
        g_dbg->print(F("cm"));
    } else {
        g_dbg->print(F("--"));
    }

    if (_ultrasonicMgr) {
        g_dbg->print(F(" | us="));
        for (uint8_t sensor = 0; sensor < NUM_ULTRASONIC; ++sensor) {
            if (_ultrasonicMgr->isValid(sensor)) {
                g_dbg->print(_ultrasonicMgr->getDistance(sensor) / 10.0f, 0);
            } else {
                g_dbg->print(F("--"));
            }

            if (sensor + 1 < NUM_ULTRASONIC) {
                g_dbg->print(' ');
            }
        }
    }

    g_dbg->print(F(" | batt="));
    g_dbg->print(_lastBattV, 1);
    g_dbg->print(F("V"));
    g_dbg->println();
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
