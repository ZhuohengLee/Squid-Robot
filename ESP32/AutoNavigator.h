/**********************************************************************
 * AutoNavigator.h
 *
 * 杩欎釜鏂囦欢澹版槑鍩轰簬瓒呭０娉㈡暟鎹殑鑷姩瀵昏矾妯″潡銆? *********************************************************************/

// 中文逐行说明：下面这一行保留原始代码 -> #ifndef ESP32_AUTO_NAVIGATOR_H
#ifndef ESP32_AUTO_NAVIGATOR_H
// 中文逐行说明：下面这一行保留原始代码 -> #define ESP32_AUTO_NAVIGATOR_H
#define ESP32_AUTO_NAVIGATOR_H

// 中文逐行说明：下面这一行保留原始代码 -> #include <Arduino.h>
#include <Arduino.h>
// 中文逐行说明：下面这一行保留原始代码 -> #include "ForwardControl.h"
#include "ForwardControl.h"
// 中文逐行说明：下面这一行保留原始代码 -> #include "LeftTurnControl.h"
#include "LeftTurnControl.h"
// 中文逐行说明：下面这一行保留原始代码 -> #include "RightTurnControl.h"
#include "RightTurnControl.h"
// 中文逐行说明：下面这一行保留原始代码 -> #include "UltrasonicManager.h"
#include "UltrasonicManager.h"

// 中文逐行说明：下面这一行保留原始代码 -> class AutoNavigator {
class AutoNavigator {
// 中文逐行说明：下面这一行保留原始代码 -> public:
public:
    // 中文逐行说明：下面这一行保留原始代码 -> AutoNavigator();
    AutoNavigator();

    // 娉ㄥ叆鍓嶈繘鍜岃浆鍚戝瓙绯荤粺鎺у埗鍣ㄣ€?    void begin(ForwardControl* forwardControl, LeftTurnControl* leftTurnControl, RightTurnControl* rightTurnControl);

    // 鎵撳紑鎴栧叧闂嚜鍔ㄥ璺€?    void setEnabled(bool enabled);
    // 中文逐行说明：下面这一行保留原始代码 -> bool isEnabled() const;
    bool isEnabled() const;

    // 鏍规嵁婊ゆ尝鍚庣殑瓒呭０娉㈡暟鎹洿鏂拌嚜鍔ㄥ璺喅绛栥€?    void update(const UltrasonicManager& ultrasonicManager, uint32_t nowMs);

// 中文逐行说明：下面这一行保留原始代码 -> private:
private:
    // 中文逐行说明：下面这一行保留原始代码 -> ForwardControl* _forwardControl;
    ForwardControl* _forwardControl;
    // 中文逐行说明：下面这一行保留原始代码 -> LeftTurnControl* _leftTurnControl;
    LeftTurnControl* _leftTurnControl;
    // 中文逐行说明：下面这一行保留原始代码 -> RightTurnControl* _rightTurnControl;
    RightTurnControl* _rightTurnControl;
    // 中文逐行说明：下面这一行保留原始代码 -> bool _enabled;
    bool _enabled;
    // 中文逐行说明：下面这一行保留原始代码 -> uint32_t _decisionLockUntilMs;
    uint32_t _decisionLockUntilMs;
// 中文逐行说明：下面这一行保留原始代码 -> };
};

// 中文逐行说明：下面这一行保留原始代码 -> #endif // ESP32_AUTO_NAVIGATOR_H
#endif // ESP32_AUTO_NAVIGATOR_H
