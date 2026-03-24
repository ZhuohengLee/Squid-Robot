/**********************************************************************
 * LeftTurnControl.h
 *
 * 这个文件声明左转控制模块。
 *********************************************************************/

#ifndef ESP32_LEFT_TURN_CONTROL_H
#define ESP32_LEFT_TURN_CONTROL_H

#include <Arduino.h>
#include "MotionLink.h"

class LeftTurnControl {
public:
    LeftTurnControl();

    void begin(MotionLink* motionLink);
    void execute();

private:
    MotionLink* _motionLink;
};

#endif // ESP32_LEFT_TURN_CONTROL_H
