/**********************************************************************
 * RightTurnControl.h
 *
 * 这个文件声明右转控制模块。
 *********************************************************************/

#ifndef ESP32_RIGHT_TURN_CONTROL_H
#define ESP32_RIGHT_TURN_CONTROL_H

#include <Arduino.h>
#include "MotionLink.h"

class RightTurnControl {
public:
    RightTurnControl();

    void begin(MotionLink* motionLink);
    void execute();

private:
    MotionLink* _motionLink;
};

#endif // ESP32_RIGHT_TURN_CONTROL_H
