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
    void displayCompact();
    bool isHealthy() const;
    bool hasDepthSensor() const;
    bool isDepthOnline() const;
    uint8_t getSensorCount() const;

private:
    DepthSensorManager* _depthMgr;
    StatusDisplay* _statusDisplay;
    UltrasonicManager* _ultrasonicMgr;
    uint32_t _lastDisplay;
};

#endif // ESP32_SENSOR_HUB_H
