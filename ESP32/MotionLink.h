/**********************************************************************
 * MotionLink.h
 *
 * Serial link from ESP32 motion/control logic to the Minima executor.
 *********************************************************************/

#ifndef ESP32_MOTION_LINK_H
#define ESP32_MOTION_LINK_H

#include <Arduino.h>
#include "Protocol.h"

class MotionLink {
public:
    MotionLink();

    void begin();
    void applyMask(uint16_t actuatorMask, bool forceSend = false);
    void applyBuoyancy(uint8_t direction, uint8_t pwm, bool forceSend = false);
    void emergencyStop();
    void requestDepthZeroCalibration();
    uint16_t getLastMask() const;

private:
    uint16_t _lastMask;
    uint8_t _lastBuoyancyDirection;
    uint8_t _lastBuoyancyPwm;
    uint32_t _lastMaskSendMs;
    uint32_t _lastBuoyancySendMs;

    void sendCommand(uint8_t cmd, uint8_t data0 = 0, uint8_t data1 = 0, uint8_t data2 = 0);
};

#endif // ESP32_MOTION_LINK_H
