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
    // 控制模式只保留手动和自动两种，和用户操作约定保持一致。
    enum ControlMode : uint8_t {
        MODE_MANUAL = 0,
        MODE_AUTO = 1
    };

    CommandHandler();

    // 这里只做内部状态清理，串口初始化交给 MotionLink 统一管理。
    void begin();

    // 注入传感器和显示模块，用于校准和调试输出。
    void setSensorHub(SensorHub* hub);
    void setStatusDisplay(StatusDisplay* display);

    // 注入运动和控制模块，命令解析后会调用这些对象。
    void setMotionLink(MotionLink* motionLink);
    void setForwardControl(ForwardControl* forwardControl);
    void setLeftTurnControl(LeftTurnControl* leftTurnControl);
    void setRightTurnControl(RightTurnControl* rightTurnControl);
    void setDepthController(DepthController* depthController);
    void setAutoNavigator(AutoNavigator* autoNavigator);

    // 轮询 USB 串口输入。
    void processSerialInput();

    // 对外暴露当前模式，便于其他模块按需显示。
    ControlMode getMode() const;

private:
    // 串口命令缓冲区，按行读取。
    String _cmdBuffer;

    // 各个外部模块的指针。
    SensorHub* _sensorHub;
    StatusDisplay* _statusDisplay;
    MotionLink* _motionLink;
    ForwardControl* _forwardControl;
    LeftTurnControl* _leftTurnControl;
    RightTurnControl* _rightTurnControl;
    DepthController* _depthController;
    AutoNavigator* _autoNavigator;

    // 当前控制模式，默认是手动。
    ControlMode _mode;

    // 处理归一化后的整条命令。
    void processCommand(const String& cmd);

    // 切换当前工作模式。
    void toggleMode();
    void enterManualMode();
    void enterAutoMode();

    // 手动控制相关辅助函数。
    void handleManualCommand(const String& cmd);
    void handleDepthTargetCommand(const String& cmd);

    // 调试和系统命令辅助函数。
    void handleCalibrateCommand();
    void handleStopCommand();
    void printModeBanner() const;
};

#endif // ESP32_COMMAND_HANDLER_H
