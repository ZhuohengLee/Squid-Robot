/**********************************************************************
 * StatusDisplay.cpp
 *
 * 这个文件实现 Minima 回传状态帧的接收和串口显示。
 *********************************************************************/

#include "StatusDisplay.h"

StatusDisplay::StatusDisplay()
    : _verboseMode(true),
      _lastHeartbeat(0),
      _rxIndex(0),
      _depthMgr(nullptr) {}

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

        const bool validLength = _rxBuffer[1] == FRAME_LENGTH;
        const bool validTail = _rxBuffer[7] == FRAME_TAIL;
        const uint8_t crc = calculateCRC8(&_rxBuffer[2], 4);
        const bool validCrc = crc == _rxBuffer[6];
        _rxIndex = 0;

        if (!validLength || !validTail || !validCrc) {
            continue;
        }

        switch (_rxBuffer[2]) {
            case STATUS_MOTION:
                processMotionStatus(_rxBuffer[3]);
                break;

            case STATUS_HEARTBEAT:
                processHeartbeat();
                break;

            case STATUS_DEPTH:
                processDepthStatus(_rxBuffer[3], _rxBuffer[4], _rxBuffer[5]);
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

void StatusDisplay::setDepthSensorManager(DepthSensorManager* manager) {
    _depthMgr = manager;
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

void StatusDisplay::processDepthStatus(uint8_t data0, uint8_t data1, uint8_t data2) {
    if (!_depthMgr) {
        return;
    }

    const int16_t depthMm = static_cast<int16_t>(
        static_cast<uint16_t>(data0) |
        (static_cast<uint16_t>(data1) << 8));
    const int8_t temperatureC = static_cast<int8_t>(static_cast<int16_t>(data2) - 40);
    _depthMgr->ingestMeasurement(depthMm, temperatureC);
}
