/**********************************************************************
 * RightTurnControl.h
 *
 * 杩欎釜鏂囦欢澹版槑鍙宠浆瀛愮郴缁熸椂搴忔帶鍒舵ā鍧椼€? *********************************************************************/

// 中文逐行说明：下面这一行保留原始代码 -> #ifndef ESP32_RIGHT_TURN_CONTROL_H
#ifndef ESP32_RIGHT_TURN_CONTROL_H
// 中文逐行说明：下面这一行保留原始代码 -> #define ESP32_RIGHT_TURN_CONTROL_H
#define ESP32_RIGHT_TURN_CONTROL_H

// 中文逐行说明：下面这一行保留原始代码 -> #include <Arduino.h>
#include <Arduino.h>
// 中文逐行说明：下面这一行保留原始代码 -> #include "Protocol.h"
#include "Protocol.h"

// 中文逐行说明：下面这一行保留原始代码 -> class RightTurnControl {
class RightTurnControl {
// 中文逐行说明：下面这一行保留原始代码 -> public:
public:
    // 中文逐行说明：下面这一行保留原始代码 -> RightTurnControl();
    RightTurnControl();

    // 中文逐行说明：下面这一行保留原始代码 -> void begin();
    void begin();
    // 中文逐行说明：下面这一行保留原始代码 -> void start();
    void start();
    // 中文逐行说明：下面这一行保留原始代码 -> void cancel();
    void cancel();
    // 中文逐行说明：下面这一行保留原始代码 -> void update(uint32_t nowMs);
    void update(uint32_t nowMs);

    // 中文逐行说明：下面这一行保留原始代码 -> uint16_t getMask() const;
    uint16_t getMask() const;
    // 中文逐行说明：下面这一行保留原始代码 -> bool isRunning() const;
    bool isRunning() const;
    // 中文逐行说明：下面这一行保留原始代码 -> bool isBalancing() const;
    bool isBalancing() const;
    // 中文逐行说明：下面这一行保留原始代码 -> bool isBusy() const;
    bool isBusy() const;

// 中文逐行说明：下面这一行保留原始代码 -> private:
private:
    // 中文逐行说明：下面这一行保留原始代码 -> bool _running;
    bool _running;
    // 中文逐行说明：下面这一行保留原始代码 -> bool _balancing;
    bool _balancing;
    // 中文逐行说明：下面这一行保留原始代码 -> bool _balanceValveOpen;
    bool _balanceValveOpen;
    // 中文逐行说明：下面这一行保留原始代码 -> uint32_t _startMs;
    uint32_t _startMs;
    // 中文逐行说明：下面这一行保留原始代码 -> uint32_t _balanceStartMs;
    uint32_t _balanceStartMs;
// 中文逐行说明：下面这一行保留原始代码 -> };
};

// 中文逐行说明：下面这一行保留原始代码 -> #endif // ESP32_RIGHT_TURN_CONTROL_H
#endif // ESP32_RIGHT_TURN_CONTROL_H
