/**********************************************************************
 * CommandHandler.cpp
 *
 * 杩欎釜鏂囦欢瀹炵幇 ESP32 渚х殑涓插彛鍛戒护瑙ｆ瀽鍜屾帶鍒舵ā寮忓垏鎹㈤€昏緫銆? * 杩愬姩骞跺彂瑙勫垯濡備笅锛? * 1. 鍓嶈繘銆佽浆鍚戙€佹诞娌夊睘浜庝笉鍚屽瓙绯荤粺锛屽彲浠ュ悓鏃舵墽琛屻€? * 2. 宸﹁浆鍜屽彸杞睘浜庡悓涓€瀛愮郴缁燂紝鍚庡埌鍛戒护浼氳鐩栧墠涓€鏉℃柟鍚戝懡浠ゃ€? * 3. 鎵嬪姩娴矇浼氭竻闄ゅ綋鍓嶅畾娣辩洰鏍囷紝浣嗕笉浼氭墦鏂墠杩涙垨杞悜銆? *********************************************************************/

// 中文逐行说明：下面这一行保留原始代码 -> #include "CommandHandler.h"
#include "CommandHandler.h"
// 中文逐行说明：下面这一行保留原始代码 -> #include "StatusDisplay.h"
#include "StatusDisplay.h"

// 中文逐行说明：下面这一行保留原始代码 -> namespace {
namespace {
// 中文逐行说明：下面这一行保留原始代码 -> String normalizeCommand(String cmd) {
String normalizeCommand(String cmd) {
    // 中文逐行说明：下面这一行保留原始代码 -> cmd.trim();
    cmd.trim();
    // 中文逐行说明：下面这一行保留原始代码 -> cmd.toLowerCase();
    cmd.toLowerCase();
    // 中文逐行说明：下面这一行保留原始代码 -> return cmd;
    return cmd;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void printHelp() {
void printHelp() {
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("\n================ COMMAND REFERENCE ================"));
    Serial.println(F("\n================ COMMAND REFERENCE ================"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("Mode:"));
    Serial.println(F("Mode:"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("  q                - Toggle manual/auto mode"));
    Serial.println(F("  q                - Toggle manual/auto mode"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F(""));
    Serial.println(F(""));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("Manual motion:"));
    Serial.println(F("Manual motion:"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("  w                - Start/keep forward motion"));
    Serial.println(F("  w                - Start/keep forward motion"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("  a                - Start a left turn sequence"));
    Serial.println(F("  a                - Start a left turn sequence"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("  d                - Start a right turn sequence"));
    Serial.println(F("  d                - Start a right turn sequence"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("  j                - Ascend"));
    Serial.println(F("  j                - Ascend"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("  k                - Descend"));
    Serial.println(F("  k                - Descend"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("  l<number>        - Hold a fixed depth in cm"));
    Serial.println(F("  l<number>        - Hold a fixed depth in cm"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("  s                - Emergency stop"));
    Serial.println(F("  s                - Emergency stop"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F(""));
    Serial.println(F(""));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("Sensors:"));
    Serial.println(F("Sensors:"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("  c                - Calibrate current depth as zero"));
    Serial.println(F("  c                - Calibrate current depth as zero"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("  g                - Show all sensor readings"));
    Serial.println(F("  g                - Show all sensor readings"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("  v                - Toggle verbose output"));
    Serial.println(F("  v                - Toggle verbose output"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F(""));
    Serial.println(F(""));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("System:"));
    Serial.println(F("System:"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("  h                - Show this help"));
    Serial.println(F("  h                - Show this help"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("===================================================\n"));
    Serial.println(F("===================================================\n"));
// 中文逐行说明：下面这一行保留原始代码 -> }
}
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> CommandHandler::CommandHandler()
CommandHandler::CommandHandler()
    // 中文逐行说明：下面这一行保留原始代码 -> : _cmdBuffer(""),
    : _cmdBuffer(""),
      // 中文逐行说明：下面这一行保留原始代码 -> _sensorHub(nullptr),
      _sensorHub(nullptr),
      // 中文逐行说明：下面这一行保留原始代码 -> _statusDisplay(nullptr),
      _statusDisplay(nullptr),
      // 中文逐行说明：下面这一行保留原始代码 -> _motionLink(nullptr),
      _motionLink(nullptr),
      // 中文逐行说明：下面这一行保留原始代码 -> _forwardControl(nullptr),
      _forwardControl(nullptr),
      // 中文逐行说明：下面这一行保留原始代码 -> _leftTurnControl(nullptr),
      _leftTurnControl(nullptr),
      // 中文逐行说明：下面这一行保留原始代码 -> _rightTurnControl(nullptr),
      _rightTurnControl(nullptr),
      // 中文逐行说明：下面这一行保留原始代码 -> _depthController(nullptr),
      _depthController(nullptr),
      // 中文逐行说明：下面这一行保留原始代码 -> _autoNavigator(nullptr),
      _autoNavigator(nullptr),
      // 中文逐行说明：下面这一行保留原始代码 -> _mode(MODE_MANUAL) {}
      _mode(MODE_MANUAL) {}

// 中文逐行说明：下面这一行保留原始代码 -> void CommandHandler::begin() {
void CommandHandler::begin() {
    // 中文逐行说明：下面这一行保留原始代码 -> _cmdBuffer = "";
    _cmdBuffer = "";
    // 中文逐行说明：下面这一行保留原始代码 -> _mode = MODE_MANUAL;
    _mode = MODE_MANUAL;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void CommandHandler::setSensorHub(SensorHub* hub) {
void CommandHandler::setSensorHub(SensorHub* hub) {
    // 中文逐行说明：下面这一行保留原始代码 -> _sensorHub = hub;
    _sensorHub = hub;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void CommandHandler::setStatusDisplay(StatusDisplay* display) {
void CommandHandler::setStatusDisplay(StatusDisplay* display) {
    // 中文逐行说明：下面这一行保留原始代码 -> _statusDisplay = display;
    _statusDisplay = display;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void CommandHandler::setMotionLink(MotionLink* motionLink) {
void CommandHandler::setMotionLink(MotionLink* motionLink) {
    // 中文逐行说明：下面这一行保留原始代码 -> _motionLink = motionLink;
    _motionLink = motionLink;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void CommandHandler::setForwardControl(ForwardControl* forwardControl) {
void CommandHandler::setForwardControl(ForwardControl* forwardControl) {
    // 中文逐行说明：下面这一行保留原始代码 -> _forwardControl = forwardControl;
    _forwardControl = forwardControl;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void CommandHandler::setLeftTurnControl(LeftTurnControl* leftTurnControl) {
void CommandHandler::setLeftTurnControl(LeftTurnControl* leftTurnControl) {
    // 中文逐行说明：下面这一行保留原始代码 -> _leftTurnControl = leftTurnControl;
    _leftTurnControl = leftTurnControl;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void CommandHandler::setRightTurnControl(RightTurnControl* rightTurnControl) {
void CommandHandler::setRightTurnControl(RightTurnControl* rightTurnControl) {
    // 中文逐行说明：下面这一行保留原始代码 -> _rightTurnControl = rightTurnControl;
    _rightTurnControl = rightTurnControl;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void CommandHandler::setDepthController(DepthController* depthController) {
void CommandHandler::setDepthController(DepthController* depthController) {
    // 中文逐行说明：下面这一行保留原始代码 -> _depthController = depthController;
    _depthController = depthController;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void CommandHandler::setAutoNavigator(AutoNavigator* autoNavigator) {
void CommandHandler::setAutoNavigator(AutoNavigator* autoNavigator) {
    // 中文逐行说明：下面这一行保留原始代码 -> _autoNavigator = autoNavigator;
    _autoNavigator = autoNavigator;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void CommandHandler::processSerialInput() {
void CommandHandler::processSerialInput() {
    // 中文逐行说明：下面这一行保留原始代码 -> while (Serial.available()) {
    while (Serial.available()) {
        // 中文逐行说明：下面这一行保留原始代码 -> const char c = static_cast<char>(Serial.read());
        const char c = static_cast<char>(Serial.read());

        // 中文逐行说明：下面这一行保留原始代码 -> if (c == '\n' || c == '\r') {
        if (c == '\n' || c == '\r') {
            // 中文逐行说明：下面这一行保留原始代码 -> if (_cmdBuffer.length() > 0) {
            if (_cmdBuffer.length() > 0) {
                // 中文逐行说明：下面这一行保留原始代码 -> processCommand(normalizeCommand(_cmdBuffer));
                processCommand(normalizeCommand(_cmdBuffer));
            // 中文逐行说明：下面这一行保留原始代码 -> }
            }
            // 中文逐行说明：下面这一行保留原始代码 -> _cmdBuffer = "";
            _cmdBuffer = "";
            // 中文逐行说明：下面这一行保留原始代码 -> continue;
            continue;
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }

        // 中文逐行说明：下面这一行保留原始代码 -> if (c != ' ') {
        if (c != ' ') {
            // 中文逐行说明：下面这一行保留原始代码 -> _cmdBuffer += c;
            _cmdBuffer += c;
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> CommandHandler::ControlMode CommandHandler::getMode() const {
CommandHandler::ControlMode CommandHandler::getMode() const {
    // 中文逐行说明：下面这一行保留原始代码 -> return _mode;
    return _mode;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void CommandHandler::processCommand(const String& cmd) {
void CommandHandler::processCommand(const String& cmd) {
    // 中文逐行说明：下面这一行保留原始代码 -> if (cmd.length() == 0) {
    if (cmd.length() == 0) {
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (cmd == "q") {
    if (cmd == "q") {
        // 中文逐行说明：下面这一行保留原始代码 -> toggleMode();
        toggleMode();
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (cmd == "h" || cmd == "help") {
    if (cmd == "h" || cmd == "help") {
        // 中文逐行说明：下面这一行保留原始代码 -> printHelp();
        printHelp();
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (cmd == "v" || cmd == "verbose") {
    if (cmd == "v" || cmd == "verbose") {
        // 中文逐行说明：下面这一行保留原始代码 -> if (_statusDisplay) {
        if (_statusDisplay) {
            // 中文逐行说明：下面这一行保留原始代码 -> _statusDisplay->toggleVerbose();
            _statusDisplay->toggleVerbose();
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (cmd == "g" || cmd == "sensors") {
    if (cmd == "g" || cmd == "sensors") {
        // 中文逐行说明：下面这一行保留原始代码 -> if (_sensorHub) {
        if (_sensorHub) {
            // 中文逐行说明：下面这一行保留原始代码 -> _sensorHub->displayAll();
            _sensorHub->displayAll();
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (cmd == "c" || cmd == "calibrate") {
    if (cmd == "c" || cmd == "calibrate") {
        // 中文逐行说明：下面这一行保留原始代码 -> handleCalibrateCommand();
        handleCalibrateCommand();
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (cmd == "s" || cmd == "stop") {
    if (cmd == "s" || cmd == "stop") {
        // 中文逐行说明：下面这一行保留原始代码 -> handleStopCommand();
        handleStopCommand();
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (cmd.startsWith("l") && cmd.length() > 1) {
    if (cmd.startsWith("l") && cmd.length() > 1) {
        // 中文逐行说明：下面这一行保留原始代码 -> handleDepthTargetCommand(cmd);
        handleDepthTargetCommand(cmd);
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (_mode == MODE_AUTO) {
    if (_mode == MODE_AUTO) {
        // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("AUTO mode is active. Press q to return to manual mode."));
        Serial.println(F("AUTO mode is active. Press q to return to manual mode."));
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> handleManualCommand(cmd);
    handleManualCommand(cmd);
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void CommandHandler::toggleMode() {
void CommandHandler::toggleMode() {
    // 中文逐行说明：下面这一行保留原始代码 -> if (_mode == MODE_MANUAL) {
    if (_mode == MODE_MANUAL) {
        // 中文逐行说明：下面这一行保留原始代码 -> enterAutoMode();
        enterAutoMode();
    // 中文逐行说明：下面这一行保留原始代码 -> } else {
    } else {
        // 中文逐行说明：下面这一行保留原始代码 -> enterManualMode();
        enterManualMode();
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void CommandHandler::enterManualMode() {
void CommandHandler::enterManualMode() {
    // 中文逐行说明：下面这一行保留原始代码 -> _mode = MODE_MANUAL;
    _mode = MODE_MANUAL;

    // 中文逐行说明：下面这一行保留原始代码 -> if (_autoNavigator) {
    if (_autoNavigator) {
        // 中文逐行说明：下面这一行保留原始代码 -> _autoNavigator->setEnabled(false);
        _autoNavigator->setEnabled(false);
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (_forwardControl) {
    if (_forwardControl) {
        // 中文逐行说明：下面这一行保留原始代码 -> _forwardControl->emergencyStop();
        _forwardControl->emergencyStop();
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (_leftTurnControl) {
    if (_leftTurnControl) {
        // 中文逐行说明：下面这一行保留原始代码 -> _leftTurnControl->cancel();
        _leftTurnControl->cancel();
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (_rightTurnControl) {
    if (_rightTurnControl) {
        // 中文逐行说明：下面这一行保留原始代码 -> _rightTurnControl->cancel();
        _rightTurnControl->cancel();
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (_depthController) {
    if (_depthController) {
        // 中文逐行说明：下面这一行保留原始代码 -> _depthController->clearTarget();
        _depthController->clearTarget();
        // 中文逐行说明：下面这一行保留原始代码 -> _depthController->manualStop();
        _depthController->manualStop();
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> printModeBanner();
    printModeBanner();
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void CommandHandler::enterAutoMode() {
void CommandHandler::enterAutoMode() {
    // 中文逐行说明：下面这一行保留原始代码 -> _mode = MODE_AUTO;
    _mode = MODE_AUTO;

    // 中文逐行说明：下面这一行保留原始代码 -> if (_forwardControl) {
    if (_forwardControl) {
        // 中文逐行说明：下面这一行保留原始代码 -> _forwardControl->emergencyStop();
        _forwardControl->emergencyStop();
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (_leftTurnControl) {
    if (_leftTurnControl) {
        // 中文逐行说明：下面这一行保留原始代码 -> _leftTurnControl->cancel();
        _leftTurnControl->cancel();
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (_rightTurnControl) {
    if (_rightTurnControl) {
        // 中文逐行说明：下面这一行保留原始代码 -> _rightTurnControl->cancel();
        _rightTurnControl->cancel();
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (_depthController) {
    if (_depthController) {
        // 中文逐行说明：下面这一行保留原始代码 -> _depthController->manualStop();
        _depthController->manualStop();
        // 中文逐行说明：下面这一行保留原始代码 -> if (_sensorHub && _sensorHub->isHealthy()) {
        if (_sensorHub && _sensorHub->isHealthy()) {
            // 中文逐行说明：下面这一行保留原始代码 -> _depthController->holdCurrentDepth();
            _depthController->holdCurrentDepth();
        // 中文逐行说明：下面这一行保留原始代码 -> } else {
        } else {
            // 中文逐行说明：下面这一行保留原始代码 -> _depthController->clearTarget();
            _depthController->clearTarget();
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (_autoNavigator) {
    if (_autoNavigator) {
        // 中文逐行说明：下面这一行保留原始代码 -> _autoNavigator->setEnabled(true);
        _autoNavigator->setEnabled(true);
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> printModeBanner();
    printModeBanner();
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void CommandHandler::handleManualCommand(const String& cmd) {
void CommandHandler::handleManualCommand(const String& cmd) {
    // 中文逐行说明：下面这一行保留原始代码 -> if (!_forwardControl || !_leftTurnControl || !_rightTurnControl || !_depthController) {
    if (!_forwardControl || !_leftTurnControl || !_rightTurnControl || !_depthController) {
        // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("Motion modules are not ready."));
        Serial.println(F("Motion modules are not ready."));
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (cmd == "w" || cmd == "forward") {
    if (cmd == "w" || cmd == "forward") {
        // 中文逐行说明：下面这一行保留原始代码 -> _forwardControl->start();
        _forwardControl->start();
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (cmd == "a" || cmd == "left") {
    if (cmd == "a" || cmd == "left") {
        // 中文逐行说明：下面这一行保留原始代码 -> _rightTurnControl->cancel();
        _rightTurnControl->cancel();
        // 中文逐行说明：下面这一行保留原始代码 -> _leftTurnControl->start();
        _leftTurnControl->start();
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (cmd == "d" || cmd == "right") {
    if (cmd == "d" || cmd == "right") {
        // 中文逐行说明：下面这一行保留原始代码 -> _leftTurnControl->cancel();
        _leftTurnControl->cancel();
        // 中文逐行说明：下面这一行保留原始代码 -> _rightTurnControl->start();
        _rightTurnControl->start();
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (cmd == "j" || cmd == "ascend") {
    if (cmd == "j" || cmd == "ascend") {
        // 中文逐行说明：下面这一行保留原始代码 -> _depthController->clearTarget();
        _depthController->clearTarget();
        // 中文逐行说明：下面这一行保留原始代码 -> _depthController->manualAscend();
        _depthController->manualAscend();
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (cmd == "k" || cmd == "descend") {
    if (cmd == "k" || cmd == "descend") {
        // 中文逐行说明：下面这一行保留原始代码 -> _depthController->clearTarget();
        _depthController->clearTarget();
        // 中文逐行说明：下面这一行保留原始代码 -> _depthController->manualDescend();
        _depthController->manualDescend();
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> Serial.print(F("Unknown manual command: "));
    Serial.print(F("Unknown manual command: "));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(cmd);
    Serial.println(cmd);
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void CommandHandler::handleDepthTargetCommand(const String& cmd) {
void CommandHandler::handleDepthTargetCommand(const String& cmd) {
    // 中文逐行说明：下面这一行保留原始代码 -> if (!_depthController) {
    if (!_depthController) {
        // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("Depth controller is not ready."));
        Serial.println(F("Depth controller is not ready."));
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> const float targetDepthCm = cmd.substring(1).toFloat();
    const float targetDepthCm = cmd.substring(1).toFloat();
    // 中文逐行说明：下面这一行保留原始代码 -> if (targetDepthCm <= 0.0f) {
    if (targetDepthCm <= 0.0f) {
        // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("Depth target must be greater than 0 cm."));
        Serial.println(F("Depth target must be greater than 0 cm."));
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> _depthController->setTargetDepth(targetDepthCm);
    _depthController->setTargetDepth(targetDepthCm);
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.print(F("Target depth set to "));
    Serial.print(F("Target depth set to "));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.print(targetDepthCm, 1);
    Serial.print(targetDepthCm, 1);
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F(" cm"));
    Serial.println(F(" cm"));
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void CommandHandler::handleCalibrateCommand() {
void CommandHandler::handleCalibrateCommand() {
    // 中文逐行说明：下面这一行保留原始代码 -> if (_autoNavigator) {
    if (_autoNavigator) {
        // 中文逐行说明：下面这一行保留原始代码 -> _autoNavigator->setEnabled(false);
        _autoNavigator->setEnabled(false);
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (_forwardControl) {
    if (_forwardControl) {
        // 中文逐行说明：下面这一行保留原始代码 -> _forwardControl->emergencyStop();
        _forwardControl->emergencyStop();
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (_leftTurnControl) {
    if (_leftTurnControl) {
        // 中文逐行说明：下面这一行保留原始代码 -> _leftTurnControl->cancel();
        _leftTurnControl->cancel();
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (_rightTurnControl) {
    if (_rightTurnControl) {
        // 中文逐行说明：下面这一行保留原始代码 -> _rightTurnControl->cancel();
        _rightTurnControl->cancel();
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (_depthController) {
    if (_depthController) {
        // 中文逐行说明：下面这一行保留原始代码 -> _depthController->clearTarget();
        _depthController->clearTarget();
        // 中文逐行说明：下面这一行保留原始代码 -> _depthController->manualStop();
        _depthController->manualStop();
        // 中文逐行说明：下面这一行保留原始代码 -> _depthController->resetAfterCalibration();
        _depthController->resetAfterCalibration();
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (_sensorHub) {
    if (_sensorHub) {
        // 中文逐行说明：下面这一行保留原始代码 -> _sensorHub->calibrateDepthZero();
        _sensorHub->calibrateDepthZero();
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("Depth zero recalibrated."));
    Serial.println(F("Depth zero recalibrated."));

    // 中文逐行说明：下面这一行保留原始代码 -> if (_mode == MODE_AUTO && _depthController && _autoNavigator) {
    if (_mode == MODE_AUTO && _depthController && _autoNavigator) {
        // 中文逐行说明：下面这一行保留原始代码 -> if (_sensorHub && _sensorHub->isHealthy()) {
        if (_sensorHub && _sensorHub->isHealthy()) {
            // 中文逐行说明：下面这一行保留原始代码 -> _depthController->holdCurrentDepth();
            _depthController->holdCurrentDepth();
        // 中文逐行说明：下面这一行保留原始代码 -> } else {
        } else {
            // 中文逐行说明：下面这一行保留原始代码 -> _depthController->clearTarget();
            _depthController->clearTarget();
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }
        // 中文逐行说明：下面这一行保留原始代码 -> _autoNavigator->setEnabled(true);
        _autoNavigator->setEnabled(true);
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void CommandHandler::handleStopCommand() {
void CommandHandler::handleStopCommand() {
    // 中文逐行说明：下面这一行保留原始代码 -> _mode = MODE_MANUAL;
    _mode = MODE_MANUAL;

    // 中文逐行说明：下面这一行保留原始代码 -> if (_autoNavigator) {
    if (_autoNavigator) {
        // 中文逐行说明：下面这一行保留原始代码 -> _autoNavigator->setEnabled(false);
        _autoNavigator->setEnabled(false);
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (_forwardControl) {
    if (_forwardControl) {
        // 中文逐行说明：下面这一行保留原始代码 -> _forwardControl->emergencyStop();
        _forwardControl->emergencyStop();
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (_leftTurnControl) {
    if (_leftTurnControl) {
        // 中文逐行说明：下面这一行保留原始代码 -> _leftTurnControl->cancel();
        _leftTurnControl->cancel();
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (_rightTurnControl) {
    if (_rightTurnControl) {
        // 中文逐行说明：下面这一行保留原始代码 -> _rightTurnControl->cancel();
        _rightTurnControl->cancel();
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (_depthController) {
    if (_depthController) {
        // 中文逐行说明：下面这一行保留原始代码 -> _depthController->clearTarget();
        _depthController->clearTarget();
        // 中文逐行说明：下面这一行保留原始代码 -> _depthController->manualStop();
        _depthController->manualStop();
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (_motionLink) {
    if (_motionLink) {
        // 中文逐行说明：下面这一行保留原始代码 -> _motionLink->emergencyStop();
        _motionLink->emergencyStop();
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("Emergency stop sent. Mode forced to MANUAL."));
    Serial.println(F("Emergency stop sent. Mode forced to MANUAL."));
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void CommandHandler::printModeBanner() const {
void CommandHandler::printModeBanner() const {
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("---------------------------------------------"));
    Serial.println(F("---------------------------------------------"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.print(F("Control mode: "));
    Serial.print(F("Control mode: "));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(_mode == MODE_AUTO ? F("AUTO") : F("MANUAL"));
    Serial.println(_mode == MODE_AUTO ? F("AUTO") : F("MANUAL"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("---------------------------------------------"));
    Serial.println(F("---------------------------------------------"));
// 中文逐行说明：下面这一行保留原始代码 -> }
}
