/**********************************************************************
 * UltrasonicManager.cpp
 *
 * 杩欎釜鏂囦欢瀹炵幇涓夎矾瓒呭０娉紶鎰熷櫒鐨勮疆璇€佽Е鍙戙€佽В鏋愬拰鍗″皵鏇兼护娉€? *********************************************************************/

// 中文逐行说明：下面这一行保留原始代码 -> #include "UltrasonicManager.h"
#include "UltrasonicManager.h"

// 中文逐行说明：下面这一行保留原始代码 -> namespace {
namespace {
// 中文逐行说明：下面这一行保留原始代码 -> constexpr uint32_t SCAN_INTERVAL_MS = 100;
constexpr uint32_t SCAN_INTERVAL_MS = 100;
// 中文逐行说明：下面这一行保留原始代码 -> constexpr uint32_t DATA_TIMEOUT_MS = 2000;
constexpr uint32_t DATA_TIMEOUT_MS = 2000;

// 中文逐行说明：下面这一行保留原始代码 -> const uint8_t UART_CHANNELS[NUM_ULTRASONIC] = {
const uint8_t UART_CHANNELS[NUM_ULTRASONIC] = {
    // 中文逐行说明：下面这一行保留原始代码 -> ULTRASONIC_FRONT_UART,
    ULTRASONIC_FRONT_UART,
    // 中文逐行说明：下面这一行保留原始代码 -> ULTRASONIC_LEFT_UART,
    ULTRASONIC_LEFT_UART,
    // 中文逐行说明：下面这一行保留原始代码 -> ULTRASONIC_RIGHT_UART,
    ULTRASONIC_RIGHT_UART,
// 中文逐行说明：下面这一行保留原始代码 -> };
};
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> UltrasonicManager::UltrasonicManager(CH9434A* ch9434)
UltrasonicManager::UltrasonicManager(CH9434A* ch9434)
    // 中文逐行说明：下面这一行保留原始代码 -> : _ch9434(ch9434),
    : _ch9434(ch9434),
      // 中文逐行说明：下面这一行保留原始代码 -> _lastScanTime(0) {
      _lastScanTime(0) {
    // 中文逐行说明：下面这一行保留原始代码 -> resetAll();
    resetAll();
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> bool UltrasonicManager::begin() {
bool UltrasonicManager::begin() {
    // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("Initializing ultrasonic sensors..."));
    Serial.println(F("Initializing ultrasonic sensors..."));

    // 中文逐行说明：下面这一行保留原始代码 -> for (uint8_t sensor = 0; sensor < NUM_ULTRASONIC; ++sensor) {
    for (uint8_t sensor = 0; sensor < NUM_ULTRASONIC; ++sensor) {
        // 中文逐行说明：下面这一行保留原始代码 -> const uint8_t uart = getUartChannel(sensor);
        const uint8_t uart = getUartChannel(sensor);
        // 中文逐行说明：下面这一行保留原始代码 -> Serial.print(F("  Configuring UART"));
        Serial.print(F("  Configuring UART"));
        // 中文逐行说明：下面这一行保留原始代码 -> Serial.print(uart);
        Serial.print(uart);
        // 中文逐行说明：下面这一行保留原始代码 -> Serial.print(F("... "));
        Serial.print(F("... "));

        // 中文逐行说明：下面这一行保留原始代码 -> if (!_ch9434->config(uart, ULTRASONIC_BAUDRATE, CH9434A_LCR_8N1)) {
        if (!_ch9434->config(uart, ULTRASONIC_BAUDRATE, CH9434A_LCR_8N1)) {
            // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("FAILED"));
            Serial.println(F("FAILED"));
            // 中文逐行说明：下面这一行保留原始代码 -> return false;
            return false;
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }

        // 中文逐行说明：下面这一行保留原始代码 -> _ch9434->flush(uart);
        _ch9434->flush(uart);
        // 中文逐行说明：下面这一行保留原始代码 -> Serial.println(F("OK"));
        Serial.println(F("OK"));
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> return true;
    return true;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void UltrasonicManager::update() {
void UltrasonicManager::update() {
    // 中文逐行说明：下面这一行保留原始代码 -> const uint32_t now = millis();
    const uint32_t now = millis();
    // 中文逐行说明：下面这一行保留原始代码 -> if (now - _lastScanTime < SCAN_INTERVAL_MS) {
    if (now - _lastScanTime < SCAN_INTERVAL_MS) {
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> _lastScanTime = now;
    _lastScanTime = now;

    // 中文逐行说明：下面这一行保留原始代码 -> for (uint8_t sensor = 0; sensor < NUM_ULTRASONIC; ++sensor) {
    for (uint8_t sensor = 0; sensor < NUM_ULTRASONIC; ++sensor) {
        // 中文逐行说明：下面这一行保留原始代码 -> readSensor(sensor);
        readSensor(sensor);

        // 中文逐行说明：下面这一行保留原始代码 -> if (now - _sensors[sensor].lastUpdate > DATA_TIMEOUT_MS) {
        if (now - _sensors[sensor].lastUpdate > DATA_TIMEOUT_MS) {
            // 中文逐行说明：下面这一行保留原始代码 -> _sensors[sensor].valid = false;
            _sensors[sensor].valid = false;
            // 中文逐行说明：下面这一行保留原始代码 -> _filterInitialized[sensor] = false;
            _filterInitialized[sensor] = false;
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> uint16_t UltrasonicManager::getDistance(uint8_t sensor) const {
uint16_t UltrasonicManager::getDistance(uint8_t sensor) const {
    // 中文逐行说明：下面这一行保留原始代码 -> return sensor < NUM_ULTRASONIC ? _sensors[sensor].filteredDistanceMm : 0;
    return sensor < NUM_ULTRASONIC ? _sensors[sensor].filteredDistanceMm : 0;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> uint16_t UltrasonicManager::getRawDistance(uint8_t sensor) const {
uint16_t UltrasonicManager::getRawDistance(uint8_t sensor) const {
    // 中文逐行说明：下面这一行保留原始代码 -> return sensor < NUM_ULTRASONIC ? _sensors[sensor].rawDistanceMm : 0;
    return sensor < NUM_ULTRASONIC ? _sensors[sensor].rawDistanceMm : 0;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> bool UltrasonicManager::isValid(uint8_t sensor) const {
bool UltrasonicManager::isValid(uint8_t sensor) const {
    // 中文逐行说明：下面这一行保留原始代码 -> return sensor < NUM_ULTRASONIC && _sensors[sensor].valid;
    return sensor < NUM_ULTRASONIC && _sensors[sensor].valid;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> uint32_t UltrasonicManager::getLastUpdate(uint8_t sensor) const {
uint32_t UltrasonicManager::getLastUpdate(uint8_t sensor) const {
    // 中文逐行说明：下面这一行保留原始代码 -> return sensor < NUM_ULTRASONIC ? _sensors[sensor].lastUpdate : 0;
    return sensor < NUM_ULTRASONIC ? _sensors[sensor].lastUpdate : 0;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void UltrasonicManager::getAllDistances(uint16_t* distances) const {
void UltrasonicManager::getAllDistances(uint16_t* distances) const {
    // 中文逐行说明：下面这一行保留原始代码 -> for (uint8_t sensor = 0; sensor < NUM_ULTRASONIC; ++sensor) {
    for (uint8_t sensor = 0; sensor < NUM_ULTRASONIC; ++sensor) {
        // 中文逐行说明：下面这一行保留原始代码 -> distances[sensor] = _sensors[sensor].valid ? _sensors[sensor].filteredDistanceMm : 0;
        distances[sensor] = _sensors[sensor].valid ? _sensors[sensor].filteredDistanceMm : 0;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void UltrasonicManager::reset(uint8_t sensor) {
void UltrasonicManager::reset(uint8_t sensor) {
    // 中文逐行说明：下面这一行保留原始代码 -> if (sensor >= NUM_ULTRASONIC) {
    if (sensor >= NUM_ULTRASONIC) {
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> _sensors[sensor].rawDistanceMm = 0;
    _sensors[sensor].rawDistanceMm = 0;
    // 中文逐行说明：下面这一行保留原始代码 -> _sensors[sensor].filteredDistanceMm = 0;
    _sensors[sensor].filteredDistanceMm = 0;
    // 中文逐行说明：下面这一行保留原始代码 -> _sensors[sensor].valid = false;
    _sensors[sensor].valid = false;
    // 中文逐行说明：下面这一行保留原始代码 -> _sensors[sensor].lastUpdate = 0;
    _sensors[sensor].lastUpdate = 0;
    // 中文逐行说明：下面这一行保留原始代码 -> _sensors[sensor].errorCount = 0;
    _sensors[sensor].errorCount = 0;
    // 中文逐行说明：下面这一行保留原始代码 -> _filterInitialized[sensor] = false;
    _filterInitialized[sensor] = false;
    // 中文逐行说明：下面这一行保留原始代码 -> _lastTrigger[sensor] = 0;
    _lastTrigger[sensor] = 0;
    // 中文逐行说明：下面这一行保留原始代码 -> _lastFilterUpdate[sensor] = 0;
    _lastFilterUpdate[sensor] = 0;
    // 中文逐行说明：下面这一行保留原始代码 -> _filters[sensor].reset();
    _filters[sensor].reset();
    // 中文逐行说明：下面这一行保留原始代码 -> _ch9434->flush(getUartChannel(sensor));
    _ch9434->flush(getUartChannel(sensor));
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void UltrasonicManager::resetAll() {
void UltrasonicManager::resetAll() {
    // 中文逐行说明：下面这一行保留原始代码 -> for (uint8_t sensor = 0; sensor < NUM_ULTRASONIC; ++sensor) {
    for (uint8_t sensor = 0; sensor < NUM_ULTRASONIC; ++sensor) {
        // 中文逐行说明：下面这一行保留原始代码 -> _sensors[sensor].rawDistanceMm = 0;
        _sensors[sensor].rawDistanceMm = 0;
        // 中文逐行说明：下面这一行保留原始代码 -> _sensors[sensor].filteredDistanceMm = 0;
        _sensors[sensor].filteredDistanceMm = 0;
        // 中文逐行说明：下面这一行保留原始代码 -> _sensors[sensor].valid = false;
        _sensors[sensor].valid = false;
        // 中文逐行说明：下面这一行保留原始代码 -> _sensors[sensor].lastUpdate = 0;
        _sensors[sensor].lastUpdate = 0;
        // 中文逐行说明：下面这一行保留原始代码 -> _sensors[sensor].errorCount = 0;
        _sensors[sensor].errorCount = 0;
        // 中文逐行说明：下面这一行保留原始代码 -> _filterInitialized[sensor] = false;
        _filterInitialized[sensor] = false;
        // 中文逐行说明：下面这一行保留原始代码 -> _lastTrigger[sensor] = 0;
        _lastTrigger[sensor] = 0;
        // 中文逐行说明：下面这一行保留原始代码 -> _lastFilterUpdate[sensor] = 0;
        _lastFilterUpdate[sensor] = 0;
        // 中文逐行说明：下面这一行保留原始代码 -> _filters[sensor].reset();
        _filters[sensor].reset();
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> bool UltrasonicManager::readSensor(uint8_t sensor) {
bool UltrasonicManager::readSensor(uint8_t sensor) {
    // 中文逐行说明：下面这一行保留原始代码 -> if (sensor >= NUM_ULTRASONIC) {
    if (sensor >= NUM_ULTRASONIC) {
        // 中文逐行说明：下面这一行保留原始代码 -> return false;
        return false;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> const uint8_t uart = getUartChannel(sensor);
    const uint8_t uart = getUartChannel(sensor);
    // 中文逐行说明：下面这一行保留原始代码 -> const uint32_t now = millis();
    const uint32_t now = millis();

    // 中文逐行说明：下面这一行保留原始代码 -> if (now - _lastTrigger[sensor] > 50) {
    if (now - _lastTrigger[sensor] > 50) {
        // 中文逐行说明：下面这一行保留原始代码 -> triggerMeasurement(sensor);
        triggerMeasurement(sensor);
        // 中文逐行说明：下面这一行保留原始代码 -> _lastTrigger[sensor] = now;
        _lastTrigger[sensor] = now;
        // 中文逐行说明：下面这一行保留原始代码 -> delay(20);
        delay(20);
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (_ch9434->available(uart) == 0) {
    if (_ch9434->available(uart) == 0) {
        // 中文逐行说明：下面这一行保留原始代码 -> if (++_sensors[sensor].errorCount > 3) {
        if (++_sensors[sensor].errorCount > 3) {
            // 中文逐行说明：下面这一行保留原始代码 -> _sensors[sensor].valid = false;
            _sensors[sensor].valid = false;
            // 中文逐行说明：下面这一行保留原始代码 -> _filterInitialized[sensor] = false;
            _filterInitialized[sensor] = false;
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }
        // 中文逐行说明：下面这一行保留原始代码 -> return false;
        return false;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> uint8_t buffer[10];
    uint8_t buffer[10];
    // 中文逐行说明：下面这一行保留原始代码 -> uint8_t bytesRead = 0;
    uint8_t bytesRead = 0;
    // 中文逐行说明：下面这一行保留原始代码 -> const uint32_t start = millis();
    const uint32_t start = millis();

    // 中文逐行说明：下面这一行保留原始代码 -> while (bytesRead < sizeof(buffer) && (millis() - start) < ULTRASONIC_TIMEOUT) {
    while (bytesRead < sizeof(buffer) && (millis() - start) < ULTRASONIC_TIMEOUT) {
        // 中文逐行说明：下面这一行保留原始代码 -> if (_ch9434->available(uart)) {
        if (_ch9434->available(uart)) {
            // 中文逐行说明：下面这一行保留原始代码 -> buffer[bytesRead++] = _ch9434->read(uart);
            buffer[bytesRead++] = _ch9434->read(uart);
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }
        // 中文逐行说明：下面这一行保留原始代码 -> delayMicroseconds(100);
        delayMicroseconds(100);
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (bytesRead < 4) {
    if (bytesRead < 4) {
        // 中文逐行说明：下面这一行保留原始代码 -> ++_sensors[sensor].errorCount;
        ++_sensors[sensor].errorCount;
        // 中文逐行说明：下面这一行保留原始代码 -> return false;
        return false;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> int frameStart = -1;
    int frameStart = -1;
    // 中文逐行说明：下面这一行保留原始代码 -> for (uint8_t i = 0; i <= bytesRead - 4; ++i) {
    for (uint8_t i = 0; i <= bytesRead - 4; ++i) {
        // 中文逐行说明：下面这一行保留原始代码 -> if (buffer[i] == 0xFF) {
        if (buffer[i] == 0xFF) {
            // 中文逐行说明：下面这一行保留原始代码 -> frameStart = i;
            frameStart = i;
            // 中文逐行说明：下面这一行保留原始代码 -> break;
            break;
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> if (frameStart < 0 || !validateFrame(&buffer[frameStart])) {
    if (frameStart < 0 || !validateFrame(&buffer[frameStart])) {
        // 中文逐行说明：下面这一行保留原始代码 -> ++_sensors[sensor].errorCount;
        ++_sensors[sensor].errorCount;
        // 中文逐行说明：下面这一行保留原始代码 -> return false;
        return false;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> const uint16_t rawDistanceMm = parseDistance(&buffer[frameStart]);
    const uint16_t rawDistanceMm = parseDistance(&buffer[frameStart]);
    // 中文逐行说明：下面这一行保留原始代码 -> if (rawDistanceMm < 50 || rawDistanceMm > 3000) {
    if (rawDistanceMm < 50 || rawDistanceMm > 3000) {
        // 中文逐行说明：下面这一行保留原始代码 -> ++_sensors[sensor].errorCount;
        ++_sensors[sensor].errorCount;
        // 中文逐行说明：下面这一行保留原始代码 -> return false;
        return false;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> const float rawDistanceCm = rawDistanceMm / 10.0f;
    const float rawDistanceCm = rawDistanceMm / 10.0f;
    // 中文逐行说明：下面这一行保留原始代码 -> float filteredDistanceCm = rawDistanceCm;
    float filteredDistanceCm = rawDistanceCm;

    // 中文逐行说明：下面这一行保留原始代码 -> if (!_filterInitialized[sensor]) {
    if (!_filterInitialized[sensor]) {
        // 中文逐行说明：下面这一行保留原始代码 -> _filters[sensor].reset(rawDistanceCm, 0.0f);
        _filters[sensor].reset(rawDistanceCm, 0.0f);
        // 中文逐行说明：下面这一行保留原始代码 -> _filterInitialized[sensor] = true;
        _filterInitialized[sensor] = true;
    // 中文逐行说明：下面这一行保留原始代码 -> } else {
    } else {
        // 中文逐行说明：下面这一行保留原始代码 -> float dt = static_cast<float>(now - _lastFilterUpdate[sensor]) * 0.001f;
        float dt = static_cast<float>(now - _lastFilterUpdate[sensor]) * 0.001f;
        // 中文逐行说明：下面这一行保留原始代码 -> if (dt < 0.05f) {
        if (dt < 0.05f) {
            // 中文逐行说明：下面这一行保留原始代码 -> dt = 0.05f;
            dt = 0.05f;
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }
        // 中文逐行说明：下面这一行保留原始代码 -> if (dt > 0.20f) {
        if (dt > 0.20f) {
            // 中文逐行说明：下面这一行保留原始代码 -> dt = 0.20f;
            dt = 0.20f;
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }

        // 中文逐行说明：下面这一行保留原始代码 -> _filters[sensor].update(rawDistanceCm, dt);
        _filters[sensor].update(rawDistanceCm, dt);
        // 中文逐行说明：下面这一行保留原始代码 -> filteredDistanceCm = _filters[sensor].getPosition();
        filteredDistanceCm = _filters[sensor].getPosition();
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 中文逐行说明：下面这一行保留原始代码 -> _sensors[sensor].rawDistanceMm = rawDistanceMm;
    _sensors[sensor].rawDistanceMm = rawDistanceMm;
    // 中文逐行说明：下面这一行保留原始代码 -> _sensors[sensor].filteredDistanceMm = static_cast<uint16_t>(filteredDistanceCm * 10.0f);
    _sensors[sensor].filteredDistanceMm = static_cast<uint16_t>(filteredDistanceCm * 10.0f);
    // 中文逐行说明：下面这一行保留原始代码 -> _sensors[sensor].valid = true;
    _sensors[sensor].valid = true;
    // 中文逐行说明：下面这一行保留原始代码 -> _sensors[sensor].lastUpdate = now;
    _sensors[sensor].lastUpdate = now;
    // 中文逐行说明：下面这一行保留原始代码 -> _sensors[sensor].errorCount = 0;
    _sensors[sensor].errorCount = 0;
    // 中文逐行说明：下面这一行保留原始代码 -> _lastFilterUpdate[sensor] = now;
    _lastFilterUpdate[sensor] = now;
    // 中文逐行说明：下面这一行保留原始代码 -> return true;
    return true;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> void UltrasonicManager::triggerMeasurement(uint8_t sensor) {
void UltrasonicManager::triggerMeasurement(uint8_t sensor) {
    // 中文逐行说明：下面这一行保留原始代码 -> const uint8_t uart = getUartChannel(sensor);
    const uint8_t uart = getUartChannel(sensor);
    // 中文逐行说明：下面这一行保留原始代码 -> _ch9434->flush(uart);
    _ch9434->flush(uart);
    // 中文逐行说明：下面这一行保留原始代码 -> _ch9434->write(uart, 0x00);
    _ch9434->write(uart, 0x00);
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> bool UltrasonicManager::validateFrame(const uint8_t* buffer) const {
bool UltrasonicManager::validateFrame(const uint8_t* buffer) const {
    // 中文逐行说明：下面这一行保留原始代码 -> return buffer[0] == 0xFF &&
    return buffer[0] == 0xFF &&
           // 中文逐行说明：下面这一行保留原始代码 -> static_cast<uint8_t>(buffer[0] + buffer[1] + buffer[2]) == buffer[3];
           static_cast<uint8_t>(buffer[0] + buffer[1] + buffer[2]) == buffer[3];
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> uint16_t UltrasonicManager::parseDistance(const uint8_t* buffer) const {
uint16_t UltrasonicManager::parseDistance(const uint8_t* buffer) const {
    // 中文逐行说明：下面这一行保留原始代码 -> return static_cast<uint16_t>(buffer[1] << 8) | buffer[2];
    return static_cast<uint16_t>(buffer[1] << 8) | buffer[2];
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 中文逐行说明：下面这一行保留原始代码 -> uint8_t UltrasonicManager::getUartChannel(uint8_t sensor) const {
uint8_t UltrasonicManager::getUartChannel(uint8_t sensor) const {
    // 中文逐行说明：下面这一行保留原始代码 -> return UART_CHANNELS[sensor];
    return UART_CHANNELS[sensor];
// 中文逐行说明：下面这一行保留原始代码 -> }
}
