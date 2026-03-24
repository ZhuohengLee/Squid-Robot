/**********************************************************************
 * CommandHandler.cpp
 *
 * 这个文件实现 ESP32 侧的串口命令解析和控制模式切换逻辑。
 *********************************************************************/

#include "CommandHandler.h"
#include "StatusDisplay.h"

namespace {
// 统一清理命令字符串的首尾空白并转成小写，降低解析分支复杂度。
String normalizeCommand(String cmd) {
    cmd.trim();
    cmd.toLowerCase();
    return cmd;
}

// 打印帮助时把当前支持的命令一次列清楚。
void printHelp() {
    Serial.println(F("\n================ COMMAND REFERENCE ================"));
    Serial.println(F("Mode:"));
    Serial.println(F("  q                - Toggle manual/auto mode"));
    Serial.println(F(""));
    Serial.println(F("Manual motion:"));
    Serial.println(F("  w                - Forward"));
    Serial.println(F("  a                - Turn left"));
    Serial.println(F("  d                - Turn right"));
    Serial.println(F("  j                - Ascend"));
    Serial.println(F("  k                - Descend"));
    Serial.println(F("  l<number>        - Hold a fixed depth in cm"));
    Serial.println(F("  s                - Emergency stop and clear auto actions"));
    Serial.println(F(""));
    Serial.println(F("Sensors:"));
    Serial.println(F("  c                - Calibrate current depth as zero"));
    Serial.println(F("  g                - Show all sensor readings"));
    Serial.println(F("  v                - Toggle verbose output"));
    Serial.println(F(""));
    Serial.println(F("System:"));
    Serial.println(F("  h                - Show this help"));
    Serial.println(F("===================================================\n"));
}
}

CommandHandler::CommandHandler()
    : _cmdBuffer(""),
      _sensorHub(nullptr),
      _statusDisplay(nullptr),
      _motionLink(nullptr),
      _forwardControl(nullptr),
      _leftTurnControl(nullptr),
      _rightTurnControl(nullptr),
      _depthController(nullptr),
      _autoNavigator(nullptr),
      _mode(MODE_MANUAL) {}

void CommandHandler::begin() {
    _cmdBuffer = "";
    _mode = MODE_MANUAL;
}

void CommandHandler::setSensorHub(SensorHub* hub) {
    _sensorHub = hub;
}

void CommandHandler::setStatusDisplay(StatusDisplay* display) {
    _statusDisplay = display;
}

void CommandHandler::setMotionLink(MotionLink* motionLink) {
    _motionLink = motionLink;
}

void CommandHandler::setForwardControl(ForwardControl* forwardControl) {
    _forwardControl = forwardControl;
}

void CommandHandler::setLeftTurnControl(LeftTurnControl* leftTurnControl) {
    _leftTurnControl = leftTurnControl;
}

void CommandHandler::setRightTurnControl(RightTurnControl* rightTurnControl) {
    _rightTurnControl = rightTurnControl;
}

void CommandHandler::setDepthController(DepthController* depthController) {
    _depthController = depthController;
}

void CommandHandler::setAutoNavigator(AutoNavigator* autoNavigator) {
    _autoNavigator = autoNavigator;
}

void CommandHandler::processSerialInput() {
    while (Serial.available()) {
        const char c = static_cast<char>(Serial.read());

        if (c == '\n' || c == '\r') {
            if (_cmdBuffer.length() > 0) {
                processCommand(normalizeCommand(_cmdBuffer));
            }
            _cmdBuffer = "";
            continue;
        }

        if (c != ' ') {
            _cmdBuffer += c;
        }
    }
}

CommandHandler::ControlMode CommandHandler::getMode() const {
    return _mode;
}

void CommandHandler::processCommand(const String& cmd) {
    if (cmd.length() == 0) {
        return;
    }

    if (cmd == "q") {
        toggleMode();
        return;
    }

    if (cmd == "h" || cmd == "help") {
        printHelp();
        return;
    }

    if (cmd == "v" || cmd == "verbose") {
        if (_statusDisplay) {
            _statusDisplay->toggleVerbose();
        }
        return;
    }

    if (cmd == "g" || cmd == "sensors") {
        if (_sensorHub) {
            _sensorHub->displayAll();
        }
        return;
    }

    if (cmd == "c" || cmd == "calibrate") {
        handleCalibrateCommand();
        return;
    }

    if (cmd == "s" || cmd == "stop") {
        handleStopCommand();
        return;
    }

    if (cmd.startsWith("l") && cmd.length() > 1) {
        handleDepthTargetCommand(cmd);
        return;
    }

    if (_mode == MODE_AUTO) {
        Serial.println(F("AUTO mode is active. Press q to return to manual control."));
        return;
    }

    handleManualCommand(cmd);
}

void CommandHandler::toggleMode() {
    if (_mode == MODE_MANUAL) {
        enterAutoMode();
    } else {
        enterManualMode();
    }
}

void CommandHandler::enterManualMode() {
    _mode = MODE_MANUAL;

    if (_autoNavigator) {
        _autoNavigator->setEnabled(false);
    }
    if (_forwardControl) {
        _forwardControl->stop();
    }
    if (_motionLink) {
        _motionLink->stopTurn();
        _motionLink->stopBuoyancy();
    }
    if (_depthController) {
        _depthController->clearTarget();
        _depthController->manualStop();
    }

    printModeBanner();
}

void CommandHandler::enterAutoMode() {
    _mode = MODE_AUTO;

    if (_forwardControl) {
        _forwardControl->stop();
    }
    if (_motionLink) {
        _motionLink->stopTurn();
        _motionLink->stopBuoyancy();
    }
    if (_depthController) {
        if (_sensorHub && _sensorHub->isHealthy()) {
            _depthController->holdCurrentDepth();
        } else {
            _depthController->clearTarget();
        }
    }
    if (_autoNavigator) {
        _autoNavigator->setEnabled(true);
    }

    printModeBanner();
}

void CommandHandler::handleManualCommand(const String& cmd) {
    if (!_motionLink || !_forwardControl || !_depthController || !_leftTurnControl || !_rightTurnControl) {
        Serial.println(F("Motion modules are not ready."));
        return;
    }

    if (cmd == "w" || cmd == "forward") {
        _forwardControl->start();
        _motionLink->stopTurn();
        return;
    }

    if (cmd == "a" || cmd == "left") {
        _forwardControl->stop();
        _leftTurnControl->execute();
        return;
    }

    if (cmd == "d" || cmd == "right") {
        _forwardControl->stop();
        _rightTurnControl->execute();
        return;
    }

    if (cmd == "j" || cmd == "ascend") {
        _depthController->clearTarget();
        _depthController->manualAscend();
        return;
    }

    if (cmd == "k" || cmd == "descend") {
        _depthController->clearTarget();
        _depthController->manualDescend();
        return;
    }

    Serial.print(F("Unknown manual command: "));
    Serial.println(cmd);
}

void CommandHandler::handleDepthTargetCommand(const String& cmd) {
    if (!_depthController) {
        Serial.println(F("Depth controller is not ready."));
        return;
    }

    const float targetDepthCm = cmd.substring(1).toFloat();
    if (targetDepthCm <= 0.0f) {
        Serial.println(F("Depth target must be greater than 0 cm."));
        return;
    }

    _depthController->setTargetDepth(targetDepthCm);
    Serial.print(F("Target depth set to "));
    Serial.print(targetDepthCm, 1);
    Serial.println(F(" cm"));
}

void CommandHandler::handleCalibrateCommand() {
    if (_autoNavigator) {
        _autoNavigator->setEnabled(false);
    }
    if (_forwardControl) {
        _forwardControl->stop();
    }
    if (_motionLink) {
        _motionLink->stopTurn();
        _motionLink->stopBuoyancy();
    }
    if (_depthController) {
        _depthController->clearTarget();
        _depthController->resetAfterCalibration();
    }
    if (_sensorHub) {
        _sensorHub->calibrateDepthZero();
    }

    Serial.println(F("Depth zero recalibrated."));

    if (_mode == MODE_AUTO && _autoNavigator) {
        if (_sensorHub && _sensorHub->isHealthy()) {
            if (_depthController) {
                _depthController->holdCurrentDepth();
            }
        }
        _autoNavigator->setEnabled(true);
    }
}

void CommandHandler::handleStopCommand() {
    _mode = MODE_MANUAL;

    if (_autoNavigator) {
        _autoNavigator->setEnabled(false);
    }
    if (_forwardControl) {
        _forwardControl->stop();
    }
    if (_motionLink) {
        _motionLink->emergencyStop();
        _motionLink->stopTurn();
        _motionLink->stopBuoyancy();
    }
    if (_depthController) {
        _depthController->clearTarget();
        _depthController->manualStop();
    }

    Serial.println(F("Emergency stop sent. Mode forced to MANUAL."));
}

void CommandHandler::printModeBanner() const {
    Serial.println(F("---------------------------------------------"));
    Serial.print(F("Control mode: "));
    Serial.println(_mode == MODE_AUTO ? F("AUTO") : F("MANUAL"));
    Serial.println(F("---------------------------------------------"));
}
