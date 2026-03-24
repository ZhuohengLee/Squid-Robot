/**********************************************************************
 * DepthSensorManager.cpp
 *
 * 杩欎釜鏂囦欢瀹炵幇閫氳繃 CH9434A 涓插彛璇诲彇娣卞害浼犳劅鍣ㄥ苟鍋氬崱灏旀浖婊ゆ尝鐨勯€昏緫銆? *********************************************************************/

// 中文逐行说明：下面这一行保留原始代码 -> #include "DepthSensorManager.h"
#include "DepthSensorManager.h"
// 中文逐行说明：下面这一行保留原始代码 -> #include "Protocol.h"
#include "Protocol.h"
// 中文逐行说明：下面这一行保留原始代码 -> #include <cstring>
#include <cstring>

// 中文逐行说明：下面这一行保留原始代码 -> DepthSensorManager::DepthSensorManager(CH9434A* ch9434, uint8_t uartNum)
DepthSensorManager::DepthSensorManager(CH9434A* ch9434, uint8_t uartNum)
    // 中文逐行说明：下面这一行保留原始代码 -> : _ch9434(ch9434),
    : _ch9434(ch9434),
      // 中文逐行说明：下面这一行保留原始代码 -> _uartNum(uartNum),
      _uartNum(uartNum),
      // 中文逐行说明：下面这一行保留原始代码 -> _rawDepthCm(0.0f),
      _rawDepthCm(0.0f),
      // 中文逐行说明：下面这一行保留原始代码 -> _filteredDepthCm(0.0f),
      _filteredDepthCm(0.0f),
      // 中文逐行说明：下面这一行保留原始代码 -> _temperatureC(0.0f),
      _temperatureC(0.0f),
      // 中文逐行说明：下面这一行保留原始代码 -> _depthOffsetCm(0.0f),
      _depthOffsetCm(0.0f),
      // 中文逐行说明：下面这一行保留原始代码 -> _depthSpeedCmS(0.0f),
      _depthSpeedCmS(0.0f),
      // 中文逐行说明：下面这一行保留原始代码 -> _valid(false),
      _valid(false),
      // 中文逐行说明：下面这一行保留原始代码 -> _filterInitialized(false),
      _filterInitialized(false),
      // 中文逐行说明：下面这一行保留原始代码 -> _lastUpdate(0),
      _lastUpdate(0),
      // 中文逐行说明：下面这一行保留原始代码 -> _lastFilterUpdate(0),
      _lastFilterUpdate(0),
      // 中文逐行说明：下面这一行保留原始代码 -> _lineLength(0),
      _lineLength(0),
      // 中文逐行说明：下面这一行保留原始代码 -> _binaryLength(0) {}
      _binaryLength(0) {}

// 中文逐行说明：下面这一行保留原始代码 -> bool DepthSensorManager::begin() {
bool DepthSensorManager::begin() {
    // 中文逐行说明：下面这一行保留原始代码 -> if (!_ch9434->config(_uartNum, DEPTH_UART_BAUDRATE, CH9434A_LCR_8N1)) {
    if (!_ch9434->config(_uartNum, DEPTH_UART_BAUDRATE, CH9434A_LCR_8N1)) {
        // 中文逐行说明：下面这一行保留原始代码 -> return false;
        return false;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> _ch9434->flush(_uartNum);
    _ch9434->flush(_uartNum);
    // 中文逐行说明：下面这一行保留原始代码 -> return true;
    return true;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void DepthSensorManager::update() {
void DepthSensorManager::update() {
    // 中文逐行说明：下面这一行保留原始代码 -> while (_ch9434->available(_uartNum)) {
    while (_ch9434->available(_uartNum)) {
        // 中文逐行说明：下面这一行保留原始代码 -> const uint8_t byte = _ch9434->read(_uartNum);
        const uint8_t byte = _ch9434->read(_uartNum);

        // 中文逐行说明：下面这一行保留原始代码 -> if (parseBinaryByte(byte)) {
        if (parseBinaryByte(byte)) {
            // 中文逐行说明：下面这一行保留原始代码 -> continue;
            continue;
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }

        // 中文逐行说明：下面这一行保留原始代码 -> if (byte == '\r' || byte == '\n') {
        if (byte == '\r' || byte == '\n') {
            // 中文逐行说明：下面这一行保留原始代码 -> if (_lineLength > 0) {
            if (_lineLength > 0) {
                // 中文逐行说明：下面这一行保留原始代码 -> _lineBuffer[_lineLength] = '\0';
                _lineBuffer[_lineLength] = '\0';
                // 中文逐行说明：下面这一行保留原始代码 -> parseAsciiLine(_lineBuffer);
                parseAsciiLine(_lineBuffer);
                // 中文逐行说明：下面这一行保留原始代码 -> _lineLength = 0;
                _lineLength = 0;
            // 中文逐行说明：下面这一行保留原始代码 -> }
            }
            // 中文逐行说明：下面这一行保留原始代码 -> continue;
            continue;
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }

        // 中文逐行说明：下面这一行保留原始代码 -> if (_lineLength + 1 >= sizeof(_lineBuffer)) {
        if (_lineLength + 1 >= sizeof(_lineBuffer)) {
            // 中文逐行说明：下面这一行保留原始代码 -> _lineLength = 0;
            _lineLength = 0;
            // 中文逐行说明：下面这一行保留原始代码 -> continue;
            continue;
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }

        // 中文逐行说明：下面这一行保留原始代码 -> if (isPrintable(byte)) {
        if (isPrintable(byte)) {
            // 中文逐行说明：下面这一行保留原始代码 -> _lineBuffer[_lineLength++] = static_cast<char>(byte);
            _lineBuffer[_lineLength++] = static_cast<char>(byte);
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (_valid && millis() - _lastUpdate > 2000) {
    if (_valid && millis() - _lastUpdate > 2000) {
        // 中文逐行说明：下面这一行保留原始代码 -> _valid = false;
        _valid = false;
        // 中文逐行说明：下面这一行保留原始代码 -> _filterInitialized = false;
        _filterInitialized = false;
        // 中文逐行说明：下面这一行保留原始代码 -> _depthSpeedCmS = 0.0f;
        _depthSpeedCmS = 0.0f;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void DepthSensorManager::calibrateZero() {
void DepthSensorManager::calibrateZero() {
    // 中文逐行说明：下面这一行保留原始代码 -> _depthOffsetCm += _filteredDepthCm;
    _depthOffsetCm += _filteredDepthCm;
    // 中文逐行说明：下面这一行保留原始代码 -> _rawDepthCm = 0.0f;
    _rawDepthCm = 0.0f;
    // 中文逐行说明：下面这一行保留原始代码 -> _filteredDepthCm = 0.0f;
    _filteredDepthCm = 0.0f;
    // 中文逐行说明：下面这一行保留原始代码 -> _depthSpeedCmS = 0.0f;
    _depthSpeedCmS = 0.0f;
    // 中文逐行说明：下面这一行保留原始代码 -> _filterInitialized = false;
    _filterInitialized = false;
    // 中文逐行说明：下面这一行保留原始代码 -> _depthFilter.reset();
    _depthFilter.reset();
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> bool DepthSensorManager::isValid() const {
bool DepthSensorManager::isValid() const {
    // 中文逐行说明：下面这一行保留原始代码 -> return _valid;
    return _valid;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> float DepthSensorManager::getDepthCm() const {
float DepthSensorManager::getDepthCm() const {
    // 中文逐行说明：下面这一行保留原始代码 -> return _filteredDepthCm;
    return _filteredDepthCm;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> float DepthSensorManager::getRawDepthCm() const {
float DepthSensorManager::getRawDepthCm() const {
    // 中文逐行说明：下面这一行保留原始代码 -> return _rawDepthCm;
    return _rawDepthCm;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> float DepthSensorManager::getDepthSpeedCmS() const {
float DepthSensorManager::getDepthSpeedCmS() const {
    // 中文逐行说明：下面这一行保留原始代码 -> return _depthSpeedCmS;
    return _depthSpeedCmS;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> float DepthSensorManager::getTemperatureC() const {
float DepthSensorManager::getTemperatureC() const {
    // 中文逐行说明：下面这一行保留原始代码 -> return _temperatureC;
    return _temperatureC;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> uint32_t DepthSensorManager::getLastUpdate() const {
uint32_t DepthSensorManager::getLastUpdate() const {
    // 中文逐行说明：下面这一行保留原始代码 -> return _lastUpdate;
    return _lastUpdate;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> bool DepthSensorManager::parseAsciiLine(const char* line) {
bool DepthSensorManager::parseAsciiLine(const char* line) {
    // 中文逐行说明：下面这一行保留原始代码 -> char buffer[64];
    char buffer[64];
    // 中文逐行说明：下面这一行保留原始代码 -> strncpy(buffer, line, sizeof(buffer) - 1);
    strncpy(buffer, line, sizeof(buffer) - 1);
    // 中文逐行说明：下面这一行保留原始代码 -> buffer[sizeof(buffer) - 1] = '\0';
    buffer[sizeof(buffer) - 1] = '\0';

    // 中文逐行说明：下面这一行保留原始代码 -> char* endPtr = nullptr;
    char* endPtr = nullptr;
    // 中文逐行说明：下面这一行保留原始代码 -> const float first = strtof(buffer, &endPtr);
    const float first = strtof(buffer, &endPtr);
    // 中文逐行说明：下面这一行保留原始代码 -> if (endPtr == buffer) {
    if (endPtr == buffer) {
        // 中文逐行说明：下面这一行保留原始代码 -> return false;
        return false;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> while (*endPtr == ' ' || *endPtr == ',' || *endPtr == '\t' || *endPtr == ':') {
    while (*endPtr == ' ' || *endPtr == ',' || *endPtr == '\t' || *endPtr == ':') {
        // 中文逐行说明：下面这一行保留原始代码 -> ++endPtr;
        ++endPtr;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (*endPtr == '\0') {
    if (*endPtr == '\0') {
        // 中文逐行说明：下面这一行保留原始代码 -> commitDepth(first, _temperatureC, false);
        commitDepth(first, _temperatureC, false);
        // 中文逐行说明：下面这一行保留原始代码 -> return true;
        return true;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> char* secondEndPtr = nullptr;
    char* secondEndPtr = nullptr;
    // 中文逐行说明：下面这一行保留原始代码 -> const float second = strtof(endPtr, &secondEndPtr);
    const float second = strtof(endPtr, &secondEndPtr);
    // 中文逐行说明：下面这一行保留原始代码 -> if (secondEndPtr == endPtr) {
    if (secondEndPtr == endPtr) {
        // 中文逐行说明：下面这一行保留原始代码 -> commitDepth(first, _temperatureC, false);
        commitDepth(first, _temperatureC, false);
        // 中文逐行说明：下面这一行保留原始代码 -> return true;
        return true;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> commitDepth(first, second, true);
    commitDepth(first, second, true);
    // 中文逐行说明：下面这一行保留原始代码 -> return true;
    return true;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> bool DepthSensorManager::parseBinaryByte(uint8_t byte) {
bool DepthSensorManager::parseBinaryByte(uint8_t byte) {
    // 中文逐行说明：下面这一行保留原始代码 -> if (_binaryLength == 0) {
    if (_binaryLength == 0) {
        // 中文逐行说明：下面这一行保留原始代码 -> if (byte != 0xFF) {
        if (byte != 0xFF) {
            // 中文逐行说明：下面这一行保留原始代码 -> return false;
            return false;
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }

        // 中文逐行说明：下面这一行保留原始代码 -> _binaryBuffer[_binaryLength++] = byte;
        _binaryBuffer[_binaryLength++] = byte;
        // 中文逐行说明：下面这一行保留原始代码 -> return true;
        return true;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> _binaryBuffer[_binaryLength++] = byte;
    _binaryBuffer[_binaryLength++] = byte;
    // 中文逐行说明：下面这一行保留原始代码 -> if (_binaryLength < sizeof(_binaryBuffer)) {
    if (_binaryLength < sizeof(_binaryBuffer)) {
        // 中文逐行说明：下面这一行保留原始代码 -> return true;
        return true;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> const uint8_t checksum = static_cast<uint8_t>(_binaryBuffer[0] + _binaryBuffer[1] + _binaryBuffer[2]);
    const uint8_t checksum = static_cast<uint8_t>(_binaryBuffer[0] + _binaryBuffer[1] + _binaryBuffer[2]);
    // 中文逐行说明：下面这一行保留原始代码 -> const bool valid = checksum == _binaryBuffer[3];
    const bool valid = checksum == _binaryBuffer[3];
    // 中文逐行说明：下面这一行保留原始代码 -> _binaryLength = 0;
    _binaryLength = 0;

    // 中文逐行说明：下面这一行保留原始代码 -> if (!valid) {
    if (!valid) {
        // 中文逐行说明：下面这一行保留原始代码 -> return false;
        return false;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> const uint16_t depthMm = static_cast<uint16_t>(_binaryBuffer[1] << 8) | _binaryBuffer[2];
    const uint16_t depthMm = static_cast<uint16_t>(_binaryBuffer[1] << 8) | _binaryBuffer[2];
    // 中文逐行说明：下面这一行保留原始代码 -> commitDepth(depthMm / 10.0f, _temperatureC, false);
    commitDepth(depthMm / 10.0f, _temperatureC, false);
    // 中文逐行说明：下面这一行保留原始代码 -> return true;
    return true;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void DepthSensorManager::commitDepth(float depthCm, float temperatureC, bool hasTemperature) {
void DepthSensorManager::commitDepth(float depthCm, float temperatureC, bool hasTemperature) {
    // 中文逐行说明：下面这一行保留原始代码 -> const uint32_t now = millis();
    const uint32_t now = millis();
    // 中文逐行说明：下面这一行保留原始代码 -> const float correctedDepthCm = depthCm - _depthOffsetCm;
    const float correctedDepthCm = depthCm - _depthOffsetCm;

    // 中文逐行说明：下面这一行保留原始代码 -> _rawDepthCm = correctedDepthCm;
    _rawDepthCm = correctedDepthCm;

    // 中文逐行说明：下面这一行保留原始代码 -> if (!_filterInitialized) {
    if (!_filterInitialized) {
        // 中文逐行说明：下面这一行保留原始代码 -> _depthFilter.reset(correctedDepthCm, 0.0f);
        _depthFilter.reset(correctedDepthCm, 0.0f);
        // 中文逐行说明：下面这一行保留原始代码 -> _filteredDepthCm = correctedDepthCm;
        _filteredDepthCm = correctedDepthCm;
        // 中文逐行说明：下面这一行保留原始代码 -> _depthSpeedCmS = 0.0f;
        _depthSpeedCmS = 0.0f;
        // 中文逐行说明：下面这一行保留原始代码 -> _filterInitialized = true;
        _filterInitialized = true;
    // 中文逐行说明：下面这一行保留原始代码 -> } else {
    } else {
        // 中文逐行说明：下面这一行保留原始代码 -> float dt = static_cast<float>(now - _lastFilterUpdate) * 0.001f;
        float dt = static_cast<float>(now - _lastFilterUpdate) * 0.001f;
        // 中文逐行说明：下面这一行保留原始代码 -> if (dt < 0.01f) {
        if (dt < 0.01f) {
            // 中文逐行说明：下面这一行保留原始代码 -> dt = 0.01f;
            dt = 0.01f;
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }
        // 中文逐行说明：下面这一行保留原始代码 -> if (dt > 0.20f) {
        if (dt > 0.20f) {
            // 中文逐行说明：下面这一行保留原始代码 -> dt = 0.20f;
            dt = 0.20f;
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }

        // 中文逐行说明：下面这一行保留原始代码 -> _depthFilter.update(correctedDepthCm, dt);
        _depthFilter.update(correctedDepthCm, dt);
        // 中文逐行说明：下面这一行保留原始代码 -> _filteredDepthCm = _depthFilter.getPosition();
        _filteredDepthCm = _depthFilter.getPosition();
        // 中文逐行说明：下面这一行保留原始代码 -> _depthSpeedCmS = _depthFilter.getVelocity();
        _depthSpeedCmS = _depthFilter.getVelocity();
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (hasTemperature) {
    if (hasTemperature) {
        // 中文逐行说明：下面这一行保留原始代码 -> _temperatureC = temperatureC;
        _temperatureC = temperatureC;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> _lastUpdate = now;
    _lastUpdate = now;
    // 中文逐行说明：下面这一行保留原始代码 -> _lastFilterUpdate = now;
    _lastFilterUpdate = now;
    // 中文逐行说明：下面这一行保留原始代码 -> _valid = true;
    _valid = true;
// 中文逐行说明：下面这一行保留原始代码 -> }
}
