/**********************************************************************
 * MotionControl.h
 *
 * Low-level Minima actuator executor with software-PWM buoyancy output.
 *********************************************************************/

#ifndef MOTION_CONTROL_H
#define MOTION_CONTROL_H

#include <Arduino.h>
#include "PinDefinitions.h"
#include "Protocol.h"

class MotionController {
public:
    MotionController();

    void begin();
    void applyMask(uint16_t mask);
    void applyBuoyancy(uint8_t direction, uint8_t pwm);
    void update();
    void emergencyStopAll();

    uint16_t getMask() const;
    bool isForwardActive() const;
    bool isTurnActive() const;
    bool isBuoyancyActive() const;
    bool isAnyActive() const;
    uint8_t getBuoyancyDirection() const;
    uint8_t getBuoyancyPwm() const;

    void printStatus();

private:
    uint16_t _mask;
    uint8_t _requestedBuoyancyDirection;
    uint8_t _requestedBuoyancyPwm;
    uint8_t _appliedBuoyancyDirection;
    uint8_t _appliedBuoyancyPwm;
    uint32_t _lastValveChangeMs;

    void writeOutputs();
    void writeBuoyancyOutputs(uint32_t nowMs);
};

#endif // MOTION_CONTROL_H
