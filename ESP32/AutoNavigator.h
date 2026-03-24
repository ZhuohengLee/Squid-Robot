/**********************************************************************
 * AutoNavigator.h
 *
 * 这个文件声明基于超声波的自动寻路模块。
 *********************************************************************/

#ifndef ESP32_AUTO_NAVIGATOR_H
#define ESP32_AUTO_NAVIGATOR_H

#include <Arduino.h>
#include "ForwardControl.h"
#include "LeftTurnControl.h"
#include "RightTurnControl.h"
#include "UltrasonicManager.h"

class AutoNavigator {
public:
    AutoNavigator();

    void begin(ForwardControl* forwardControl, LeftTurnControl* leftTurnControl, RightTurnControl* rightTurnControl);
    void setEnabled(bool enabled);
    bool isEnabled() const;
    void update(const UltrasonicManager& ultrasonicManager, uint32_t nowMs);

private:
    ForwardControl* _forwardControl;
    LeftTurnControl* _leftTurnControl;
    RightTurnControl* _rightTurnControl;
    bool _enabled;
    uint32_t _decisionLockUntilMs;
};

#endif // ESP32_AUTO_NAVIGATOR_H
