/**********************************************************************
 * StatusDisplay.cpp
 *
 * 杩欎釜鏂囦欢瀹炵幇 Minima 鍥炰紶鐘舵€佸抚鐨勬帴鏀跺拰涓插彛鏄剧ず銆? *********************************************************************/

// 中文逐行说明：下面这一行保留原始代码 -> #include "StatusDisplay.h"
#include "StatusDisplay.h"

// 中文逐行说明：下面这一行保留原始代码 -> StatusDisplay::StatusDisplay()
StatusDisplay::StatusDisplay()
    // 中文逐行说明：下面这一行保留原始代码 -> : _verboseMode(true),
    : _verboseMode(true),
      // 中文逐行说明：下面这一行保留原始代码 -> _lastHeartbeat(0),
      _lastHeartbeat(0),
      // 中文逐行说明：下面这一行保留原始代码 -> _rxIndex(0) {}
      _rxIndex(0) {}

// 中文逐行说明：下面这一行保留原始代码 -> void StatusDisplay::processMinimaFeedback() {
void StatusDisplay::processMinimaFeedback() {
    // 中文逐行说明：下面这一行保留原始代码 -> while (UART_TO_MINIMA.available()) {
    while (UART_TO_MINIMA.available()) {
        // 中文逐行说明：下面这一行保留原始代码 -> const uint8_t byte = static_cast<uint8_t>(UART_TO_MINIMA.read());
        const uint8_t byte = static_cast<uint8_t>(UART_TO_MINIMA.read());

        // 中文逐行说明：下面这一行保留原始代码 -> if (_rxIndex == 0 && byte != FRAME_HEADER) {
        if (_rxIndex == 0 && byte != FRAME_HEADER) {
            // 中文逐行说明：下面这一行保留原始代码 -> continue;
            continue;
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }

        // 中文逐行说明：下面这一行保留原始代码 -> _rxBuffer[_rxIndex++] = byte;
        _rxBuffer[_rxIndex++] = byte;
        // 中文逐行说明：下面这一行保留原始代码 -> if (_rxIndex < FRAME_LENGTH) {
        if (_rxIndex < FRAME_LENGTH) {
            // 中文逐行说明：下面这一行保留原始代码 -> continue;
            continue;
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }

        // 中文逐行说明：下面这一行保留原始代码 -> const bool validTail = _rxBuffer[7] == FRAME_TAIL;
        const bool validTail = _rxBuffer[7] == FRAME_TAIL;
        // 中文逐行说明：下面这一行保留原始代码 -> const uint8_t crc = calculateCRC8(&_rxBuffer[2], 4);
        const uint8_t crc = calculateCRC8(&_rxBuffer[2], 4);
        // 中文逐行说明：下面这一行保留原始代码 -> const bool validCrc = crc == _rxBuffer[6];
        const bool validCrc = crc == _rxBuffer[6];
        // 中文逐行说明：下面这一行保留原始代码 -> _rxIndex = 0;
        _rxIndex = 0;

        // 中文逐行说明：下面这一行保留原始代码 -> if (!validTail || !validCrc) {
        if (!validTail || !validCrc) {
            // 中文逐行说明：下面这一行保留原始代码 -> continue;
            continue;
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }

        // 中文逐行说明：下面这一行保留原始代码 -> switch (_rxBuffer[2]) {
        switch (_rxBuffer[2]) {
            // 中文逐行说明：下面这一行保留原始代码 -> case STATUS_MOTION:
            case STATUS_MOTION:
                // 中文逐行说明：下面这一行保留原始代码 -> processMotionStatus(_rxBuffer[3]);
                processMotionStatus(_rxBuffer[3]);
                // 中文逐行说明：下面这一行保留原始代码 -> break;
                break;

            // 中文逐行说明：下面这一行保留原始代码 -> case STATUS_HEARTBEAT:
            case STATUS_HEARTBEAT:
                // 中文逐行说明：下面这一行保留原始代码 -> processHeartbeat();
                processHeartbeat();
                // 中文逐行说明：下面这一行保留原始代码 -> break;
                break;

            // 中文逐行说明：下面这一行保留原始代码 -> default:
            default:
                // 中文逐行说明：下面这一行保留原始代码 -> break;
                break;
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void StatusDisplay::enableVerbose() {
void StatusDisplay::enableVerbose() {
    // 中文逐行说明：下面这一行保留原始代码 -> _verboseMode = true;
    _verboseMode = true;
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("\nVerbose mode enabled\n"));
    Serial.println(F("\nVerbose mode enabled\n"));
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void StatusDisplay::disableVerbose() {
void StatusDisplay::disableVerbose() {
    // 中文逐行说明：下面这一行保留原始代码 -> _verboseMode = false;
    _verboseMode = false;
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("\nVerbose mode disabled\n"));
    Serial.println(F("\nVerbose mode disabled\n"));
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void StatusDisplay::toggleVerbose() {
void StatusDisplay::toggleVerbose() {
    // 中文逐行说明：下面这一行保留原始代码 -> if (_verboseMode) {
    if (_verboseMode) {
        // 中文逐行说明：下面这一行保留原始代码 -> disableVerbose();
        disableVerbose();
    // 中文逐行说明：下面这一行保留原始代码 -> } else {
    } else {
        // 中文逐行说明：下面这一行保留原始代码 -> enableVerbose();
        enableVerbose();
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> bool StatusDisplay::isVerboseEnabled() const {
bool StatusDisplay::isVerboseEnabled() const {
    // 中文逐行说明：下面这一行保留原始代码 -> return _verboseMode;
    return _verboseMode;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void StatusDisplay::processMotionStatus(uint8_t status) {
void StatusDisplay::processMotionStatus(uint8_t status) {
    // 中文逐行说明：下面这一行保留原始代码 -> if (!_verboseMode) {
    if (!_verboseMode) {
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> Serial.print(F("<- Motion: "));
    Serial.print(F("<- Motion: "));
    // 中文逐行说明：下面这一行保留原始代码 -> if (status & 0x01) Serial.print(F("FWD "));
    if (status & 0x01) Serial.print(F("FWD "));
    // 中文逐行说明：下面这一行保留原始代码 -> if (status & 0x02) Serial.print(F("TURN "));
    if (status & 0x02) Serial.print(F("TURN "));
    // 中文逐行说明：下面这一行保留原始代码 -> if (status & 0x04) Serial.print(F("BUOY "));
    if (status & 0x04) Serial.print(F("BUOY "));
    // 中文逐行说明：下面这一行保留原始代码 -> if (status == 0) Serial.print(F("IDLE"));
    if (status == 0) Serial.print(F("IDLE"));
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println();
    Serial.println();
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void StatusDisplay::processHeartbeat() {
void StatusDisplay::processHeartbeat() {
    // 中文逐行说明：下面这一行保留原始代码 -> _lastHeartbeat = millis();
    _lastHeartbeat = millis();
// 中文逐行说明：下面这一行保留原始代码 -> }
}
