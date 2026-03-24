/**********************************************************************
 * CommandHandler.h
 *
 * 杩欎釜鏂囦欢澹版槑 ESP32 渚х殑涓插彛鍛戒护瑙ｆ瀽鍣ㄣ€? * 瑙ｆ瀽鍣ㄨ礋璐ｆā寮忓垏鎹€佹墜鍔ㄨ繍鍔ㄥ懡浠ゃ€佺洰鏍囨繁搴﹀懡浠ゅ拰璋冭瘯鍛戒护銆? *********************************************************************/

// 中文逐行说明：下面这一行保留原始代码 -> #ifndef ESP32_COMMAND_HANDLER_H
#ifndef ESP32_COMMAND_HANDLER_H
// 中文逐行说明：下面这一行保留原始代码 -> #define ESP32_COMMAND_HANDLER_H
#define ESP32_COMMAND_HANDLER_H

// 中文逐行说明：下面这一行保留原始代码 -> #include <Arduino.h>
#include <Arduino.h>
// 中文逐行说明：下面这一行保留原始代码 -> #include "AutoNavigator.h"
#include "AutoNavigator.h"
// 中文逐行说明：下面这一行保留原始代码 -> #include "DepthController.h"
#include "DepthController.h"
// 中文逐行说明：下面这一行保留原始代码 -> #include "ForwardControl.h"
#include "ForwardControl.h"
// 中文逐行说明：下面这一行保留原始代码 -> #include "LeftTurnControl.h"
#include "LeftTurnControl.h"
// 中文逐行说明：下面这一行保留原始代码 -> #include "MotionLink.h"
#include "MotionLink.h"
// 中文逐行说明：下面这一行保留原始代码 -> #include "RightTurnControl.h"
#include "RightTurnControl.h"
// 中文逐行说明：下面这一行保留原始代码 -> #include "SensorHub.h"
#include "SensorHub.h"

// 中文逐行说明：下面这一行保留原始代码 -> class StatusDisplay;
class StatusDisplay;

// 中文逐行说明：下面这一行保留原始代码 -> class CommandHandler {
class CommandHandler {
// 中文逐行说明：下面这一行保留原始代码 -> public:
public:
    // 中文逐行说明：下面这一行保留原始代码 -> enum ControlMode : uint8_t {
    enum ControlMode : uint8_t {
        // 中文逐行说明：下面这一行保留原始代码 -> MODE_MANUAL = 0,
        MODE_MANUAL = 0,
        // 中文逐行说明：下面这一行保留原始代码 -> MODE_AUTO = 1
        MODE_AUTO = 1
    // 中文逐行说明：下面这一行保留原始代码 -> };
    };

    // 中文逐行说明：下面这一行保留原始代码 -> CommandHandler();
    CommandHandler();

    // 鍒濆鍖栧唴閮ㄧ姸鎬侊紱涓插彛鏈韩鐢?MotionLink 璐熻矗銆?    void begin();

    // 娉ㄥ叆浼犳劅鍣ㄥ拰鏄剧ず妯″潡銆?    void setSensorHub(SensorHub* hub);
    // 中文逐行说明：下面这一行保留原始代码 -> void setStatusDisplay(StatusDisplay* display);
    void setStatusDisplay(StatusDisplay* display);

    // 娉ㄥ叆杩愬姩鍜屾帶鍒舵ā鍧椼€?    void setMotionLink(MotionLink* motionLink);
    // 中文逐行说明：下面这一行保留原始代码 -> void setForwardControl(ForwardControl* forwardControl);
    void setForwardControl(ForwardControl* forwardControl);
    // 中文逐行说明：下面这一行保留原始代码 -> void setLeftTurnControl(LeftTurnControl* leftTurnControl);
    void setLeftTurnControl(LeftTurnControl* leftTurnControl);
    // 中文逐行说明：下面这一行保留原始代码 -> void setRightTurnControl(RightTurnControl* rightTurnControl);
    void setRightTurnControl(RightTurnControl* rightTurnControl);
    // 中文逐行说明：下面这一行保留原始代码 -> void setDepthController(DepthController* depthController);
    void setDepthController(DepthController* depthController);
    // 中文逐行说明：下面这一行保留原始代码 -> void setAutoNavigator(AutoNavigator* autoNavigator);
    void setAutoNavigator(AutoNavigator* autoNavigator);

    // 杞 USB 涓插彛杈撳叆銆?    void processSerialInput();

    // 鑾峰彇褰撳墠妯″紡銆?    ControlMode getMode() const;

// 中文逐行说明：下面这一行保留原始代码 -> private:
private:
    // 中文逐行说明：下面这一行保留原始代码 -> String _cmdBuffer;
    String _cmdBuffer;

    // 中文逐行说明：下面这一行保留原始代码 -> SensorHub* _sensorHub;
    SensorHub* _sensorHub;
    // 中文逐行说明：下面这一行保留原始代码 -> StatusDisplay* _statusDisplay;
    StatusDisplay* _statusDisplay;
    // 中文逐行说明：下面这一行保留原始代码 -> MotionLink* _motionLink;
    MotionLink* _motionLink;
    // 中文逐行说明：下面这一行保留原始代码 -> ForwardControl* _forwardControl;
    ForwardControl* _forwardControl;
    // 中文逐行说明：下面这一行保留原始代码 -> LeftTurnControl* _leftTurnControl;
    LeftTurnControl* _leftTurnControl;
    // 中文逐行说明：下面这一行保留原始代码 -> RightTurnControl* _rightTurnControl;
    RightTurnControl* _rightTurnControl;
    // 中文逐行说明：下面这一行保留原始代码 -> DepthController* _depthController;
    DepthController* _depthController;
    // 中文逐行说明：下面这一行保留原始代码 -> AutoNavigator* _autoNavigator;
    AutoNavigator* _autoNavigator;

    // 中文逐行说明：下面这一行保留原始代码 -> ControlMode _mode;
    ControlMode _mode;

    // 中文逐行说明：下面这一行保留原始代码 -> void processCommand(const String& cmd);
    void processCommand(const String& cmd);
    // 中文逐行说明：下面这一行保留原始代码 -> void toggleMode();
    void toggleMode();
    // 中文逐行说明：下面这一行保留原始代码 -> void enterManualMode();
    void enterManualMode();
    // 中文逐行说明：下面这一行保留原始代码 -> void enterAutoMode();
    void enterAutoMode();
    // 中文逐行说明：下面这一行保留原始代码 -> void handleManualCommand(const String& cmd);
    void handleManualCommand(const String& cmd);
    // 中文逐行说明：下面这一行保留原始代码 -> void handleDepthTargetCommand(const String& cmd);
    void handleDepthTargetCommand(const String& cmd);
    // 中文逐行说明：下面这一行保留原始代码 -> void handleCalibrateCommand();
    void handleCalibrateCommand();
    // 中文逐行说明：下面这一行保留原始代码 -> void handleStopCommand();
    void handleStopCommand();
    // 中文逐行说明：下面这一行保留原始代码 -> void printModeBanner() const;
    void printModeBanner() const;
// 中文逐行说明：下面这一行保留原始代码 -> };
};

// 中文逐行说明：下面这一行保留原始代码 -> #endif // ESP32_COMMAND_HANDLER_H
#endif // ESP32_COMMAND_HANDLER_H
