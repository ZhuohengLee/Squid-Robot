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
#include "SDLogger.h"
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
    void setSDLogger(SDLogger* logger);

    // 机器人模式切换回调（由 ESP32.ino 注入）。
    void setEnterTestModeCallback(void (*cb)());
    void setEnterDebugModeCallback(void (*cb)());

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

    // 处理浏览器控制台输入（权限与串口相同，不经白名单过滤）。
    void processWebInput(const String& msg);

    // 每帧调用：检查全局平衡锁是否到期。
    void update(uint32_t nowMs);

    // 获取当前模式。
    ControlMode getMode() const;

private:
    String _cmdBuffer;
    String _hc12Buffer;

    SDLogger*     _sdLogger;
    SensorHub*    _sensorHub;
    StatusDisplay* _statusDisplay;
    MotionLink*   _motionLink;
    ForwardControl* _forwardControl;
    LeftTurnControl* _leftTurnControl;
    RightTurnControl* _rightTurnControl;
    DepthController* _depthController;
    AutoNavigator* _autoNavigator;

    ControlMode _mode;
    bool        _globalBalancing;
    uint32_t    _globalBalanceEndMs;

    void (*_enterTestModeCb)();
    void (*_enterDebugModeCb)();

    void processCommand(const String& cmd, bool fromHC12 = false);
    void sendSensorDataOverHC12();
    void toggleMode();
    void enterManualMode();
    void enterAutoMode();
    void handleManualCommand(const String& cmd);
    void handleDepthTargetCommand(const String& cmd, bool fromHC12 = false);
    void handleHC12ChannelCommand(const String& channel);
    void handleCalibrateCommand();
    void handleStopCommand();
    void printModeBanner() const;
};

#endif // ESP32_COMMAND_HANDLER_H
