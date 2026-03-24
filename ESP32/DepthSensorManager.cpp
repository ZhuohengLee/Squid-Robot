/**********************************************************************
 * DepthSensorManager.cpp
 *
 * 这个文件实现通过 CH9434A 串口读取深度传感器并做卡尔曼滤波的逻辑。
 *********************************************************************/

#include "DepthSensorManager.h"
#include "Protocol.h"
#include <cstring>

DepthSensorManager::DepthSensorManager(CH9434A* ch9434, uint8_t uartNum)
    : _ch9434(ch9434),
      _uartNum(uartNum),
      _rawDepthCm(0.0f),
      _filteredDepthCm(0.0f),
      _temperatureC(0.0f),
      _depthOffsetCm(0.0f),
      _depthSpeedCmS(0.0f),
      _valid(false),
      _filterInitialized(false),
      _lastUpdate(0),
      _lastFilterUpdate(0),
      _lineLength(0),
      _binaryLength(0) {}

bool DepthSensorManager::begin() {
    if (!_ch9434->config(_uartNum, DEPTH_UART_BAUDRATE, CH9434A_LCR_8N1)) {
        return false;
    }

    _ch9434->flush(_uartNum);
    return true;
}

void DepthSensorManager::update() {
    while (_ch9434->available(_uartNum)) {
        const uint8_t byte = _ch9434->read(_uartNum);

        if (parseBinaryByte(byte)) {
            continue;
        }

        if (byte == '\r' || byte == '\n') {
            if (_lineLength > 0) {
                _lineBuffer[_lineLength] = '\0';
                parseAsciiLine(_lineBuffer);
                _lineLength = 0;
            }
            continue;
        }

        if (_lineLength + 1 >= sizeof(_lineBuffer)) {
            _lineLength = 0;
            continue;
        }

        if (isPrintable(byte)) {
            _lineBuffer[_lineLength++] = static_cast<char>(byte);
        }
    }

    if (_valid && millis() - _lastUpdate > 2000) {
        _valid = false;
        _filterInitialized = false;
        _depthSpeedCmS = 0.0f;
    }
}

void DepthSensorManager::calibrateZero() {
    _depthOffsetCm += _filteredDepthCm;
    _rawDepthCm = 0.0f;
    _filteredDepthCm = 0.0f;
    _depthSpeedCmS = 0.0f;
    _filterInitialized = false;
    _depthFilter.reset();
}

bool DepthSensorManager::isValid() const {
    return _valid;
}

float DepthSensorManager::getDepthCm() const {
    return _filteredDepthCm;
}

float DepthSensorManager::getRawDepthCm() const {
    return _rawDepthCm;
}

float DepthSensorManager::getDepthSpeedCmS() const {
    return _depthSpeedCmS;
}

float DepthSensorManager::getTemperatureC() const {
    return _temperatureC;
}

uint32_t DepthSensorManager::getLastUpdate() const {
    return _lastUpdate;
}

bool DepthSensorManager::parseAsciiLine(const char* line) {
    char buffer[64];
    strncpy(buffer, line, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char* endPtr = nullptr;
    const float first = strtof(buffer, &endPtr);
    if (endPtr == buffer) {
        return false;
    }

    while (*endPtr == ' ' || *endPtr == ',' || *endPtr == '\t' || *endPtr == ':') {
        ++endPtr;
    }

    if (*endPtr == '\0') {
        commitDepth(first, _temperatureC, false);
        return true;
    }

    char* secondEndPtr = nullptr;
    const float second = strtof(endPtr, &secondEndPtr);
    if (secondEndPtr == endPtr) {
        commitDepth(first, _temperatureC, false);
        return true;
    }

    commitDepth(first, second, true);
    return true;
}

bool DepthSensorManager::parseBinaryByte(uint8_t byte) {
    if (_binaryLength == 0) {
        if (byte != 0xFF) {
            return false;
        }

        _binaryBuffer[_binaryLength++] = byte;
        return true;
    }

    _binaryBuffer[_binaryLength++] = byte;
    if (_binaryLength < sizeof(_binaryBuffer)) {
        return true;
    }

    const uint8_t checksum = static_cast<uint8_t>(_binaryBuffer[0] + _binaryBuffer[1] + _binaryBuffer[2]);
    const bool valid = checksum == _binaryBuffer[3];
    _binaryLength = 0;

    if (!valid) {
        return false;
    }

    const uint16_t depthMm = static_cast<uint16_t>(_binaryBuffer[1] << 8) | _binaryBuffer[2];
    commitDepth(depthMm / 10.0f, _temperatureC, false);
    return true;
}

void DepthSensorManager::commitDepth(float depthCm, float temperatureC, bool hasTemperature) {
    const uint32_t now = millis();
    const float correctedDepthCm = depthCm - _depthOffsetCm;

    _rawDepthCm = correctedDepthCm;

    if (!_filterInitialized) {
        _depthFilter.reset(correctedDepthCm, 0.0f);
        _filteredDepthCm = correctedDepthCm;
        _depthSpeedCmS = 0.0f;
        _filterInitialized = true;
    } else {
        float dt = static_cast<float>(now - _lastFilterUpdate) * 0.001f;
        if (dt < 0.01f) {
            dt = 0.01f;
        }
        if (dt > 0.20f) {
            dt = 0.20f;
        }

        _depthFilter.update(correctedDepthCm, dt);
        _filteredDepthCm = _depthFilter.getPosition();
        _depthSpeedCmS = _depthFilter.getVelocity();
    }

    if (hasTemperature) {
        _temperatureC = temperatureC;
    }

    _lastUpdate = now;
    _lastFilterUpdate = now;
    _valid = true;
}
