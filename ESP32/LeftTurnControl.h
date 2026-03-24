/**********************************************************************
 * LeftTurnControl.h
 *
 * 这个文件声明左转子系统时序控制模块。
 *********************************************************************/

#ifndef ESP32_LEFT_TURN_CONTROL_H
#define ESP32_LEFT_TURN_CONTROL_H

#include <Arduino.h>
#include "Protocol.h"

class LeftTurnControl {
public:
    LeftTurnControl();

    void begin();
    void start();
    void cancel();
    void update(uint32_t nowMs);

    uint16_t getMask() const;
    bool isRunning() const;
    bool isBalancing() const;
    bool isBusy() const;

private:
    bool _running;
    bool _balancing;
    bool _balanceValveOpen;
    uint32_t _startMs;
    uint32_t _balanceStartMs;
};

#endif // ESP32_LEFT_TURN_CONTROL_H
