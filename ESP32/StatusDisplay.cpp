/**********************************************************************
 * StatusDisplay.cpp
 *
 * 这个文件实现 Minima 回传状态帧的接收和串口显示。
 *********************************************************************/

#include "StatusDisplay.h"

StatusDisplay::StatusDisplay()
    : _verboseMode(true),
      _lastHeartbeat(0),
      _rxIndex(0) {}

void StatusDisplay::processMinimaFeedback() {
    while (UART_TO_MINIMA.available()) {
        const uint8_t byte = static_cast<uint8_t>(UART_TO_MINIMA.read());

        if (_rxIndex == 0 && byte != FRAME_HEADER) {
            continue;
        }

        _rxBuffer[_rxIndex++] = byte;
        if (_rxIndex < FRAME_LENGTH) {
            continue;
        }

        const bool validTail = _rxBuffer[7] == FRAME_TAIL;
        const uint8_t crc = calculateCRC8(&_rxBuffer[2], 4);
        const bool validCrc = crc == _rxBuffer[6];
        _rxIndex = 0;

        if (!validTail || !validCrc) {
            continue;
        }

        switch (_rxBuffer[2]) {
            case STATUS_MOTION:
                processMotionStatus(_rxBuffer[3]);
                break;

            case STATUS_HEARTBEAT:
                processHeartbeat();
                break;

            default:
                break;
        }
    }
}

void StatusDisplay::enableVerbose() {
    _verboseMode = true;
    Serial.println(F("\nVerbose mode enabled\n"));
}

void StatusDisplay::disableVerbose() {
    _verboseMode = false;
    Serial.println(F("\nVerbose mode disabled\n"));
}

void StatusDisplay::toggleVerbose() {
    if (_verboseMode) {
        disableVerbose();
    } else {
        enableVerbose();
    }
}

bool StatusDisplay::isVerboseEnabled() const {
    return _verboseMode;
}

void StatusDisplay::processMotionStatus(uint8_t status) {
    if (!_verboseMode) {
        return;
    }

    Serial.print(F("<- Motion: "));
    if (status & 0x01) Serial.print(F("FWD "));
    if (status & 0x02) Serial.print(F("TURN "));
    if (status & 0x04) Serial.print(F("BUOY "));
    if (status == 0) Serial.print(F("IDLE"));
    Serial.println();
}

void StatusDisplay::processHeartbeat() {
    _lastHeartbeat = millis();
}
