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
#include "TeeStream.h"

namespace {
String normalizeCommand(String cmd) {
    cmd.trim();
    cmd.toLowerCase();
    return cmd;
}

// HC-12 白名单：只有匹配已知模式的命令才允许执行，过滤噪声。
bool isValidHC12Command(const String& cmd) {
    if (cmd.length() == 0) return false;

    // 单字母命令
    static const char singleCmds[] = {'w', 'a', 'd', 's', 'j', 'k', 'q', 'g', 'c', 'v', 'h'};
    if (cmd.length() == 1) {
        for (char c : singleCmds) {
            if (cmd[0] == c) return true;
        }
        return false;
    }

    // 双字母命令
    if (cmd == "mt" || cmd == "md") return true;

    // l + 数字（定深）
    if (cmd[0] == 'l' && cmd.length() > 1) {
        for (uint8_t i = 1; i < cmd.length(); i++) {
            if (!isDigit(cmd[i]) && cmd[i] != '.') return false;
        }
        return true;
    }

    // mark + 备注
    if (cmd.startsWith("mark")) return true;

    // esp + 3位数字（信道切换）
    if (cmd.length() == 6 && cmd.startsWith("esp") &&
        isDigit(cmd[3]) && isDigit(cmd[4]) && isDigit(cmd[5])) return true;

    // 长文本命令
    if (cmd == "forward" || cmd == "left"   || cmd == "right" ||
        cmd == "stop"    || cmd == "ascend"  || cmd == "descend" ||
        cmd == "sensors" || cmd == "calibrate" || cmd == "verbose" ||
        cmd == "help") return true;

    return false;
}

void printHelp() {
    g_dbg->println(F("\n================ COMMAND REFERENCE ================"));
    g_dbg->println(F("Mode:"));
    g_dbg->println(F("  q                - Toggle manual/auto mode"));
    g_dbg->println(F(""));
    g_dbg->println(F("Manual motion:"));
    g_dbg->println(F("  w                - Start/keep forward motion"));
    g_dbg->println(F("  a                - Start a left turn sequence"));
    g_dbg->println(F("  d                - Start a right turn sequence"));
    g_dbg->println(F("  j                - Ascend"));
    g_dbg->println(F("  k                - Descend"));
    g_dbg->println(F("  l<number>        - Hold a fixed depth in cm"));
    g_dbg->println(F("  s                - Emergency stop"));
    g_dbg->println(F(""));
    g_dbg->println(F("Sensors:"));
    g_dbg->println(F("  c                - Calibrate current depth as zero"));
    g_dbg->println(F("  g                - Show all sensor readings"));
    g_dbg->println(F("  v                - Toggle verbose output"));
    g_dbg->println(F(""));
    g_dbg->println(F("HC-12 channel:"));
    g_dbg->println(F("  ESP025           - Set this robot HC-12 to ch.025"));
    g_dbg->println(F("  (send HC025 via bridge to set bridge end)"));
    g_dbg->println(F("  Sync order: ESP first, then HC"));
    g_dbg->println(F(""));
    g_dbg->println(F("System:"));
    g_dbg->println(F("  h                - Show this help"));
    g_dbg->println(F(""));
    g_dbg->println(F("Robot mode:"));
    g_dbg->println(F("  mt               - Enter TEST mode (WiFi off, SD logging)"));
    g_dbg->println(F("  md               - Enter DEBUG mode (WiFi on, FTP/OTA)"));
    g_dbg->println(F("  mark <note>      - Insert a timestamped mark in events.log"));
    g_dbg->println(F("===================================================\n"));
}
}

CommandHandler::CommandHandler()
    : _cmdBuffer(""),
      _hc12Buffer(""),
      _sdLogger(nullptr),
      _sensorHub(nullptr),
      _statusDisplay(nullptr),
      _motionLink(nullptr),
      _forwardControl(nullptr),
      _leftTurnControl(nullptr),
      _rightTurnControl(nullptr),
      _depthController(nullptr),
      _autoNavigator(nullptr),
      _mode(MODE_MANUAL),
      _globalBalancing(false),
      _globalBalanceEndMs(0),
      _enterTestModeCb(nullptr),
      _enterDebugModeCb(nullptr) {}

void CommandHandler::begin() {
    _cmdBuffer         = "";
    _hc12Buffer        = "";
    _mode              = MODE_MANUAL;
    _globalBalancing   = false;
    _globalBalanceEndMs = 0;
    pinMode(HC12_SET_PIN, OUTPUT);
    digitalWrite(HC12_SET_PIN, HIGH);
    HC12_SERIAL.begin(HC12_BAUD_RATE, SERIAL_8N1, HC12_RX_PIN, HC12_TX_PIN);
    g_dbg->println(F("[CommandHandler] HC-12 serial initialized."));
}

void CommandHandler::setSDLogger(SDLogger* logger) { _sdLogger = logger; }
void CommandHandler::setEnterTestModeCallback(void (*cb)())  { _enterTestModeCb  = cb; }
void CommandHandler::setEnterDebugModeCallback(void (*cb)()) { _enterDebugModeCb = cb; }

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

void CommandHandler::processWebInput(const String& msg) {
    if (msg.length() == 0) return;
    const String cmd = normalizeCommand(msg);
    if (_sdLogger) _sdLogger->logCommand(millis(), "web", cmd.c_str());
    processCommand(cmd);
}

void CommandHandler::processSerialInput() {
    while (Serial.available()) {
        const char c = static_cast<char>(Serial.read());

        if (c == '\n' || c == '\r') {
            if (_cmdBuffer.length() > 0) {
                const String cmd = normalizeCommand(_cmdBuffer);
                if (_sdLogger) _sdLogger->logCommand(millis(), "serial", cmd.c_str());
                processCommand(cmd);
            }
            _cmdBuffer = "";
            continue;
        }

        _cmdBuffer += c;
        if (_cmdBuffer.length() > 128) _cmdBuffer = "";
    }
}

void CommandHandler::processHC12Input() {
    while (HC12_SERIAL.available()) {
        const char c = static_cast<char>(HC12_SERIAL.read());

        if (c == '\n' || c == '\r') {
            if (_hc12Buffer.length() > 0) {
                const String cmd = normalizeCommand(_hc12Buffer);

                // 白名单过滤：噪声字符串直接丢弃
                if (!isValidHC12Command(cmd)) {
                    _hc12Buffer = "";
                    continue;
                }

                // g/sensors 命令：把传感器数据直接写回 HC-12 发送端
                if (cmd == "g" || cmd == "sensors") {
                    sendSensorDataOverHC12();
                }

                // 必须在 processCommand 之前 log——否则 "md" 会先 endSession 再 log，
                // 导致 logCommand 早返回，HC-12 发出的 md 永远进不了 commands.csv。
                if (_sdLogger) _sdLogger->logCommand(millis(), "hc12", cmd.c_str());

                processCommand(cmd, /*fromHC12=*/true);

                // 信道切换命令已在 handleHC12ChannelCommand() 内发过回复，不重复发 ACK。
                const bool isChannelCmd =
                    cmd.length() == 6 && cmd.startsWith("esp") &&
                    isDigit(cmd[3]) && isDigit(cmd[4]) && isDigit(cmd[5]);
                if (!isChannelCmd) {
                    HC12_SERIAL.print(F("[OK]"));
                    HC12_SERIAL.println(cmd);
                }
            }
            _hc12Buffer = "";
            continue;
        }

        // 保留内部空格（mark 命令需要），trim() 在 normalizeCommand() 中处理首尾空格
        _hc12Buffer += c;
        if (_hc12Buffer.length() > 64) {
            _hc12Buffer = "";  // 超长视为噪声丢弃
        }
    }
}

void CommandHandler::sendSensorDataOverHC12() {
    if (!_sensorHub) {
        return;
    }
    // 临时把 g_dbg 重定向到 HC-12，把传感器数据直接回传给发送端
    Print* savedDbg = g_dbg;
    g_dbg = &HC12_SERIAL;
    _sensorHub->forceDisplayAll();
    g_dbg = savedDbg;
}

CommandHandler::ControlMode CommandHandler::getMode() const {
    return _mode;
}

void CommandHandler::update(uint32_t nowMs) {
    if (_globalBalancing && nowMs >= _globalBalanceEndMs) {
        _globalBalancing = false;
        g_dbg->println(F("Global balance complete. System ready."));
    }
}

void CommandHandler::processCommand(const String& cmd, bool fromHC12) {
    if (cmd.length() == 0) {
        return;
    }

    // 机器人模式切换和 mark 标记——不受急停锁和模式限制
    if (cmd == "mt") {
        if (_enterTestModeCb) _enterTestModeCb();
        return;
    }

    if (cmd == "md") {
        if (_enterDebugModeCb) _enterDebugModeCb();
        return;
    }

    if (cmd == "stat") {
        if (_sdLogger) _sdLogger->printStats();
        return;
    }

    if (cmd.startsWith(F("mark"))) {
        // "mark" 或 "mark <note>"
        const String note = (cmd.length() > 5) ? cmd.substring(5) : String(F("(no note)"));
        if (_sdLogger) _sdLogger->logEvent(millis(), note.c_str());
        g_dbg->print(F("Mark logged: "));
        g_dbg->println(note);
        return;
    }

    // 急停后 5s 内拒绝控制命令（s 命令本身始终允许）
    if (_globalBalancing && cmd != "s" && cmd != "stop") {
        g_dbg->println(F("Commands locked: pressure balance in progress."));
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
        handleDepthTargetCommand(cmd, fromHC12);
        return;
    }

    if (_mode == MODE_AUTO) {
        g_dbg->println(F("AUTO mode is active. Press q to return to manual mode."));
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
        g_dbg->println(F("Motion modules are not ready."));
        return;
    }

    if (cmd == "w" || cmd == "forward") {
        if (_forwardControl->isRunning()) {
            _forwardControl->stop();   // 有序停止并触发气压平衡
            g_dbg->println(F("Forward stop: pressure balance in progress..."));
        } else if (!_forwardControl->isBalancing()) {
            _forwardControl->start();
            g_dbg->println(F("Forward started."));
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
        _depthController->manualAscend();   // 同向再按停止，异向切换方向
        g_dbg->println(_depthController->getManualDirection() == BUOYANCY_STOP
                       ? F("Ascend stopped.") : F("Ascending..."));
        return;
    }

    if (cmd == "k" || cmd == "descend") {
        _depthController->manualDescend();  // 同向再按停止，异向切换方向
        g_dbg->println(_depthController->getManualDirection() == BUOYANCY_STOP
                       ? F("Descend stopped.") : F("Descending..."));
        return;
    }

    g_dbg->print(F("Unknown manual command: "));
    g_dbg->println(cmd);
}

void CommandHandler::handleDepthTargetCommand(const String& cmd, bool fromHC12) {
    auto reply = [&](const __FlashStringHelper* msg) {
        g_dbg->println(msg);
        if (fromHC12) HC12_SERIAL.println(msg);
    };

    if (!_depthController) {
        reply(F("[ERR] Depth controller not ready."));
        return;
    }

    if (_sensorHub && !_sensorHub->isDepthOnline()) {
        reply(F("[ERR] Depth sensor offline, cannot set target."));
        return;
    }

    const float targetDepthCm = cmd.substring(1).toFloat();
    if (targetDepthCm <= 0.0f) {
        reply(F("[ERR] Depth target must be > 0 cm."));
        return;
    }

    _depthController->setTargetDepth(targetDepthCm);
    g_dbg->print(F("Target depth set to "));
    g_dbg->print(targetDepthCm, 1);
    g_dbg->println(F(" cm"));
    if (fromHC12) {
        HC12_SERIAL.print(F("[OK] depth target="));
        HC12_SERIAL.print(targetDepthCm, 1);
        HC12_SERIAL.println(F("cm"));
    }
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

    g_dbg->println(F("Depth zero recalibrated."));

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

    // 急停所有系统
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
    if (_motionLink) {
        _motionLink->emergencyStop();
    }

    // 延迟 10ms 后对全部子系统触发全局气压平衡（500ms/5s）
    delay(10);
    if (_forwardControl)   _forwardControl->forceBalance();
    if (_leftTurnControl)  _leftTurnControl->forceBalance();
    if (_rightTurnControl) _rightTurnControl->forceBalance();
    if (_depthController)  _depthController->forceBalance();

    // 锁定控制命令 5s
    _globalBalancing    = true;
    _globalBalanceEndMs = millis() + 5000;

    g_dbg->println(F("EMERGENCY STOP: global pressure balance started (5s), commands locked."));
}

void CommandHandler::handleHC12ChannelCommand(const String& channel) {
    g_dbg->print(F("[HC-12] Setting channel to "));
    g_dbg->print(channel);
    g_dbg->print(F(" ..."));

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
        g_dbg->print(F(" OK (ch."));
        g_dbg->print(channel);
        g_dbg->print(F(") — now send HC"));
        g_dbg->print(channel);
        g_dbg->println(F(" to bridge end"));
    } else {
        g_dbg->print(F(" FAILED"));
        if (response.length() > 0) {
            g_dbg->print(F(": "));
            g_dbg->print(response);
        } else {
            g_dbg->print(F(": no response — check SET wiring (IO47)"));
        }
        g_dbg->println();
    }
}

void CommandHandler::printModeBanner() const {
    g_dbg->println(F("---------------------------------------------"));
    g_dbg->print(F("Control mode: "));
    g_dbg->println(_mode == MODE_AUTO ? F("AUTO") : F("MANUAL"));
    g_dbg->println(F("---------------------------------------------"));
}
