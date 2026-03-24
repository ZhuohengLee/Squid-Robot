/**********************************************************************
 * ForwardControl.h
 *
 * 这个文件声明前进控制模块。
 *********************************************************************/

#ifndef ESP32_FORWARD_CONTROL_H
#define ESP32_FORWARD_CONTROL_H

#include <Arduino.h>
#include "MotionLink.h"

class ForwardControl {
public:
    ForwardControl();

    void begin(MotionLink* motionLink);
    void start();
    void stop();
    bool isActive() const;

private:
    MotionLink* _motionLink;
    bool _active;
};

#endif // ESP32_FORWARD_CONTROL_H
