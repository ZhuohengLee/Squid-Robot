/**********************************************************************
 * CommandHandler.h
 *
 * 这个文件声明 ESP32 侧的串口命令解析器。
 * 解析器负责模式切换、手动运动命令、目标深度命令和调试命令。
 *********************************************************************/

#ifndef ESP32_COMMAND_HANDLER_H
#define ESP32_COMMAND_HANDLER_H

#include <Arduino.h>
#include "AutoNavigator.h"
#include "DepthController.h"
#include "ForwardControl.h"
#include "LeftTurnControl.h"
#include "MotionLink.h"
#include "RightTurnControl.h"
#include "SensorHub.h"

class StatusDisplay;

class CommandHandler {
public:
    enum ControlMode : uint8_t {
        MODE_MANUAL = 0,
        MODE_AUTO = 1
    };

    CommandHandler();

    // 初始化内部状态；串口本身由 MotionLink 负责。
    void begin();

    // 注入传感器和显示模块。
    void setSensorHub(SensorHub* hub);
    void setStatusDisplay(StatusDisplay* display);

    // 注入运动和控制模块。
    void setMotionLink(MotionLink* motionLink);
    void setForwardControl(ForwardControl* forwardControl);
    void setLeftTurnControl(LeftTurnControl* leftTurnControl);
    void setRightTurnControl(RightTurnControl* rightTurnControl);
    void setDepthController(DepthController* depthController);
    void setAutoNavigator(AutoNavigator* autoNavigator);

    // 轮询 USB 串口输入。
    void processSerialInput();

    // 轮询 HC-12 无线串口输入（与 USB 串口执行相同的命令集）。
    void processHC12Input();

    // 获取当前模式。
    ControlMode getMode() const;

private:
    String _cmdBuffer;
    String _hc12Buffer;

    SensorHub* _sensorHub;
    StatusDisplay* _statusDisplay;
    MotionLink* _motionLink;
    ForwardControl* _forwardControl;
    LeftTurnControl* _leftTurnControl;
    RightTurnControl* _rightTurnControl;
    DepthController* _depthController;
    AutoNavigator* _autoNavigator;

    ControlMode _mode;

    void processCommand(const String& cmd);
    void toggleMode();
    void enterManualMode();
    void enterAutoMode();
    void handleManualCommand(const String& cmd);
    void handleDepthTargetCommand(const String& cmd);
    void handleHC12ChannelCommand(const String& channel);
    void handleCalibrateCommand();
    void handleStopCommand();
    void printModeBanner() const;
};

#endif // ESP32_COMMAND_HANDLER_H
