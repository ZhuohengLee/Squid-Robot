/**********************************************************************
 * StatusDisplay.cpp
 *
 * 这个文件实现 Minima 回传状态帧的接收和串口显示。
 *********************************************************************/

#include "StatusDisplay.h"
#include "TeeStream.h"

StatusDisplay::StatusDisplay()
    : _lastHeartbeat(0),
      _lastMotionStatus(0),   // 初始 IDLE；运动状态变化时才打印回传
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

            default:
                break;
        }
    }
}

uint8_t StatusDisplay::getLastMotionStatus() const {
    return _lastMotionStatus;
}

bool StatusDisplay::hasRecentHeartbeat(uint32_t nowMs, uint32_t timeoutMs) const {
    return _lastHeartbeat != 0 && nowMs - _lastHeartbeat <= timeoutMs;
}

void StatusDisplay::processMotionStatus(uint8_t status) {
    // 仅在运动状态变化时打印（Minima 回传信息，始终保留，避免刷屏）
    if (status == _lastMotionStatus) {
        return;
    }
    _lastMotionStatus = status;

    g_dbg->print(F("<- Minima: "));
    if (status & 0x01) g_dbg->print(F("FWD "));
    if (status & 0x02) g_dbg->print(F("TURN "));
    if (status & 0x04) g_dbg->print(F("BUOY "));
    if (status == 0) g_dbg->print(F("IDLE"));
    g_dbg->println();
}

void StatusDisplay::processHeartbeat() {
    _lastHeartbeat = millis();
}
