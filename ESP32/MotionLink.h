/**********************************************************************
 * MotionLink.h
 *
 * 这个文件声明 ESP32 到 Minima 的运动命令发送接口。
 *********************************************************************/

#ifndef ESP32_MOTION_LINK_H
#define ESP32_MOTION_LINK_H

#include <Arduino.h>
#include "Protocol.h"

class MotionLink {
public:
    void begin();
    void sendCommand(uint8_t cmd, uint8_t data0 = 0, uint8_t data1 = 0, uint8_t data2 = 0);

    void startForward();
    void stopForward();
    void turnLeft();
    void turnRight();
    void stopTurn();
    void ascend();
    void descend();
    void stopBuoyancy();
    void emergencyStop();
};

#endif // ESP32_MOTION_LINK_H
