/**********************************************************************
 * CommandHandler.cpp
 *
 * 这个文件实现 ESP32 侧的串口命令解析和控制模式切换逻辑。
 * 运动并发规则如下：
 * 1. 前进、转向、浮沉属于不同子系统，可以同时执行。
 * 2. 左转和右转属于同一子系统，后到命令会覆盖前一条方向命令。
 * 3. 手动浮沉会清除当前定深目标，但不会打断前进或转向。
 *********************************************************************/

#include "CommandHandler.h"
#include "Protocol.h"
#include "StatusDisplay.h"

namespace {
constexpr size_t MAX_COMMAND_LENGTH = 64;

String normalizeCommand(String cmd) {
    cmd.trim();
    cmd.toLowerCase();
    return cmd;
}

void printHelp() {
    Serial.println(F("\n================ COMMAND REFERENCE ================"));
    Serial.println(F("Mode:"));
    Serial.println(F("  q                - Toggle manual/auto mode"));
    Serial.println(F(""));
    Serial.println(F("Manual motion:"));
    Serial.println(F("  w                - Start/keep forward motion"));
    Serial.println(F("  a                - Start a left turn sequence"));
    Serial.println(F("  d                - Start a right turn sequence"));
    Serial.println(F("  j                - Ascend"));
    Serial.println(F("  k                - Descend"));
    Serial.println(F("  l<number>        - Hold a fixed depth in cm"));
    Serial.println(F("  s                - Emergency stop"));
    Serial.println(F(""));
    Serial.println(F("Sensors:"));
    Serial.println(F("  c                - Calibrate current depth as zero"));
    Serial.println(F("  g                - Show all sensor readings"));
    Serial.println(F("  v                - Toggle verbose output"));
    Serial.println(F(""));
    Serial.println(F("HC-12 channel:"));
    Serial.println(F("  ESP025           - Set this robot HC-12 to ch.025"));
    Serial.println(F("  (send HC025 via bridge to set bridge end)"));
    Serial.println(F("  Sync order: ESP first, then HC"));
    Serial.println(F(""));
    Serial.println(F("System:"));
    Serial.println(F("  h                - Show this help"));
    Serial.println(F("===================================================\n"));
}
}

CommandHandler::CommandHandler()
    : _cmdBuffer(""),
      _hc12Buffer(""),
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
    _cmdBuffer  = "";
    _hc12Buffer = "";
    _mode = MODE_MANUAL;
    pinMode(HC12_SET_PIN, OUTPUT);
    digitalWrite(HC12_SET_PIN, HIGH);
    HC12_SERIAL.begin(HC12_BAUD_RATE, SERIAL_8N1, HC12_RX_PIN, HC12_TX_PIN);
    Serial.println(F("[CommandHandler] HC-12 serial initialized."));
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
            if (_cmdBuffer.length() > MAX_COMMAND_LENGTH) {
                Serial.println(F("[CommandHandler] USB input overflow, buffer cleared."));
                _cmdBuffer = "";
            }
        }
    }
}

void CommandHandler::processHC12Input() {
    while (HC12_SERIAL.available()) {
        const char c = static_cast<char>(HC12_SERIAL.read());

        if (c == '\n' || c == '\r') {
            if (_hc12Buffer.length() > 0) {
                processCommand(normalizeCommand(_hc12Buffer));
            }
            _hc12Buffer = "";
            continue;
        }

        if (c != ' ') {
            _hc12Buffer += c;
            // HC-12 单包最大 58 字节，超出视为噪声丢弃
            if (_hc12Buffer.length() > MAX_COMMAND_LENGTH) {
                _hc12Buffer = "";
            }
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

    // ESP + 3位数字 → 配置本端 HC-12 信道，例如 ESP025
    if (cmd.length() == 6 && cmd.startsWith("esp") &&
        isDigit(cmd[3]) && isDigit(cmd[4]) && isDigit(cmd[5])) {
        handleHC12ChannelCommand(cmd.substring(3));
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
        Serial.println(F("AUTO mode is active. Press q to return to manual mode."));
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
        _forwardControl->emergencyStop();
    }

    if (_leftTurnControl) {
        _leftTurnControl->cancel();
    }

    if (_rightTurnControl) {
        _rightTurnControl->cancel();
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
        _forwardControl->emergencyStop();
    }

    if (_leftTurnControl) {
        _leftTurnControl->cancel();
    }

    if (_rightTurnControl) {
        _rightTurnControl->cancel();
    }

    if (_depthController) {
        _depthController->manualStop();
        if (_sensorHub && _sensorHub->isDepthOnline()) {
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
    if (!_forwardControl || !_leftTurnControl || !_rightTurnControl || !_depthController) {
        Serial.println(F("Motion modules are not ready."));
        return;
    }

    if (cmd == "w" || cmd == "forward") {
        if (_forwardControl->isRunning()) {
            _forwardControl->emergencyStop();
        } else if (!_forwardControl->isBalancing()) {
            _forwardControl->start();
        }
        // 正在平衡中时忽略命令
        return;
    }

    if (cmd == "a" || cmd == "left") {
        _rightTurnControl->cancel();
        _leftTurnControl->start();
        return;
    }

    if (cmd == "d" || cmd == "right") {
        _leftTurnControl->cancel();
        _rightTurnControl->start();
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
        _forwardControl->emergencyStop();
    }

    if (_leftTurnControl) {
        _leftTurnControl->cancel();
    }

    if (_rightTurnControl) {
        _rightTurnControl->cancel();
    }

    if (_depthController) {
        _depthController->clearTarget();
        _depthController->manualStop();
        _depthController->resetAfterCalibration();
    }

    if (_sensorHub) {
        _sensorHub->calibrateDepthZero();
    }

    Serial.println(F("Depth zero recalibrated."));

    if (_mode == MODE_AUTO && _depthController && _autoNavigator) {
        if (_sensorHub && _sensorHub->isDepthOnline()) {
            _depthController->holdCurrentDepth();
        } else {
            _depthController->clearTarget();
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
        _forwardControl->stop();  // 带气压平衡
    }

    if (_leftTurnControl) {
        _leftTurnControl->cancel();
    }

    if (_rightTurnControl) {
        _rightTurnControl->cancel();
    }

    if (_depthController) {
        _depthController->clearTarget();
        _depthController->manualStop();
    }

    if (_motionLink) {
        _motionLink->emergencyStop();
    }

    Serial.println(F("Stop: pressure balance in progress..."));
}

void CommandHandler::handleHC12ChannelCommand(const String& channel) {
    Serial.print(F("[HC-12] Setting channel to "));
    Serial.print(channel);
    Serial.print(F(" ..."));

    // 先在当前信道通知 Minima，此时双方还在同一信道可以收到
    HC12_SERIAL.print(F("[Robot] Switching to ch."));
    HC12_SERIAL.print(channel);
    HC12_SERIAL.print(F(", send HC"));
    HC12_SERIAL.print(channel);
    HC12_SERIAL.println(F(" now"));
    delay(100); // 等待发送完成再切换

    // 拉低 SET 脚，进入 AT 命令模式（HC-12 需要 ≥40ms 才进入）
    digitalWrite(HC12_SET_PIN, LOW);
    delay(200);

    HC12_SERIAL.print(F("AT+C"));
    HC12_SERIAL.println(channel);

    // 等待 HC-12 响应，最多 1000ms
    String response = "";
    uint32_t start = millis();
    while (millis() - start < 1000) {
        if (HC12_SERIAL.available()) {
            response += (char)HC12_SERIAL.read();
        }
    }

    // 拉高 SET 脚，退出 AT 模式，恢复透传
    digitalWrite(HC12_SET_PIN, HIGH);
    delay(200);

    response.trim();
    if (response.indexOf("OK") >= 0) {
        Serial.print(F(" OK (ch."));
        Serial.print(channel);
        Serial.print(F(") — now send HC"));
        Serial.print(channel);
        Serial.println(F(" to bridge end"));
    } else {
        Serial.print(F(" FAILED"));
        if (response.length() > 0) {
            Serial.print(F(": "));
            Serial.print(response);
        } else {
            Serial.print(F(": no response — check SET wiring (IO47)"));
        }
        Serial.println();
    }
}

void CommandHandler::printModeBanner() const {
    Serial.println(F("---------------------------------------------"));
    Serial.print(F("Control mode: "));
    Serial.println(_mode == MODE_AUTO ? F("AUTO") : F("MANUAL"));
    Serial.println(F("---------------------------------------------"));
}
