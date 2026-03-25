/**********************************************************************
 * DepthSensorManager.cpp
 *
 * MS5837-02BA I2C 读取与深度滤波实现。
 *********************************************************************/

#include "DepthSensorManager.h"
#include "Protocol.h"
#include <Wire.h>

namespace {
constexpr uint8_t MS5837_CMD_RESET = 0x1E;
constexpr uint8_t MS5837_CMD_ADC_READ = 0x00;
constexpr uint8_t MS5837_CMD_CONVERT_D1_8192 = 0x4A;
constexpr uint8_t MS5837_CMD_CONVERT_D2_8192 = 0x5A;

constexpr uint32_t MS5837_RESET_DELAY_MS = 20;
constexpr uint32_t MS5837_CONVERSION_DELAY_MS = 20;
constexpr uint32_t SAMPLE_INTERVAL_MS = 100;
constexpr uint32_t DATA_TIMEOUT_MS = 2000;
constexpr uint8_t PROM_WORD_COUNT = 7;
constexpr uint8_t PROM_READ_RETRIES = 3;
constexpr uint8_t ADC_READ_RETRIES = 3;

constexpr float FRESH_WATER_DENSITY_KG_M3 = 997.0f;
constexpr float GRAVITY_M_S2 = 9.80665f;
constexpr float CM_PER_MBAR = 10000.0f / (FRESH_WATER_DENSITY_KG_M3 * GRAVITY_M_S2);
}

DepthSensorManager::DepthSensorManager()
    : _rawDepthCm(0.0f),
      _filteredDepthCm(0.0f),
      _temperatureC(0.0f),
      _pressureMbar(0.0f),
      _zeroPressureMbar(0.0f),
      _depthSpeedCmS(0.0f),
      _valid(false),
      _sensorReady(false),
      _filterInitialized(false),
      _zeroReferenceValid(false),
      _promCrcValid(false),
      _lastUpdate(0),
      _lastFilterUpdate(0),
      _lastSampleMs(0),
      _sampleCount(0),
      _readErrorCount(0),
      _lastD1(0),
      _lastD2(0),
      _lastPromReadIndex(0xFF),
      _lastI2cError(0),
      _lastReadFailure(READ_OK),
      _debugStatus(DEBUG_NOT_STARTED) {
    for (uint8_t i = 0; i < 8; ++i) {
        _prom[i] = 0;
    }
}

bool DepthSensorManager::begin() {
    _valid = false;
    _sensorReady = false;
    _filterInitialized = false;
    _zeroReferenceValid = false;
    _promCrcValid = false;
    _lastUpdate = 0;
    _lastFilterUpdate = 0;
    _lastSampleMs = 0;
    _sampleCount = 0;
    _readErrorCount = 0;
    _lastD1 = 0;
    _lastD2 = 0;
    _lastPromReadIndex = 0xFF;
    _lastI2cError = 0;
    _lastReadFailure = READ_OK;

    if (!Wire.begin(DEPTH_I2C_SDA, DEPTH_I2C_SCL, DEPTH_I2C_FREQ)) {
        _debugStatus = DEBUG_I2C_INIT_FAILED;
        return false;
    }

    Wire.setTimeOut(50);

    if (!resetSensor()) {
        _debugStatus = DEBUG_RESET_FAILED;
        return false;
    }

    if (!readProm()) {
        return false;
    }

    _sensorReady = true;
    _debugStatus = DEBUG_VALID_DATA;
    return true;
}

void DepthSensorManager::update() {
    const uint32_t now = millis();

    if (_sensorReady && (_lastSampleMs == 0 || now - _lastSampleMs >= SAMPLE_INTERVAL_MS)) {
        _lastSampleMs = now;

        float pressureMbar = 0.0f;
        float temperatureC = 0.0f;
        if (readMeasurement(pressureMbar, temperatureC)) {
            if (!_zeroReferenceValid) {
                _zeroPressureMbar = pressureMbar;
                _zeroReferenceValid = true;
            }

            commitDepth(pressureToDepthCm(pressureMbar - _zeroPressureMbar),
                        temperatureC,
                        pressureMbar);
            _debugStatus = DEBUG_VALID_DATA;
            ++_sampleCount;
        } else {
            ++_readErrorCount;
        }
    }

    if (_valid && now - _lastUpdate > DATA_TIMEOUT_MS) {
        _valid = false;
        _filterInitialized = false;
        _depthSpeedCmS = 0.0f;
        _debugStatus = DEBUG_STALE;
    }
}

void DepthSensorManager::calibrateZero() {
    if (!_zeroReferenceValid) {
        return;
    }

    _zeroPressureMbar = _pressureMbar;
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

void DepthSensorManager::printDebug() const {
    Serial.print(F("Depth I2C0: addr=0x"));
    if (DEPTH_I2C_ADDRESS < 0x10) {
        Serial.print('0');
    }
    Serial.print(DEPTH_I2C_ADDRESS, HEX);
    Serial.print(F(" | samples="));
    Serial.print(_sampleCount);
    Serial.print(F(" | err="));
    Serial.print(_readErrorCount);
    Serial.print(F(" | prom_crc="));
    Serial.print(_promCrcValid ? F("yes") : F("no"));
    Serial.print(F(" | prom_idx="));
    if (_lastPromReadIndex == 0xFF) {
        Serial.print(F("--"));
    } else {
        Serial.print(_lastPromReadIndex);
    }
    Serial.print(F(" | i2c_err="));
    Serial.print(_lastI2cError);
    Serial.print(F(" | fail="));
    Serial.print(readFailureString(_lastReadFailure));
    Serial.print(F(" | status="));
    Serial.println(debugStatusString(_debugStatus));

    Serial.print(F("  Last D1/D2: "));
    Serial.print(_lastD1);
    Serial.print(F(" / "));
    Serial.println(_lastD2);

    Serial.print(F("  Pressure: "));
    Serial.print(_pressureMbar, 2);
    Serial.print(F(" mbar | Zero: "));
    Serial.print(_zeroPressureMbar, 2);
    Serial.print(F(" mbar | Raw depth: "));
    Serial.print(_rawDepthCm, 2);
    Serial.print(F(" cm | Filtered depth: "));
    Serial.print(_filteredDepthCm, 2);
    Serial.println(F(" cm"));
}

bool DepthSensorManager::resetSensor() {
    if (!writeCommand(MS5837_CMD_RESET)) {
        return false;
    }

    delay(MS5837_RESET_DELAY_MS);
    return true;
}

bool DepthSensorManager::readProm() {
    _prom[7] = 0;

    for (uint8_t index = 0; index < PROM_WORD_COUNT; ++index) {
        _lastPromReadIndex = index;
        bool success = false;

        for (uint8_t attempt = 0; attempt < PROM_READ_RETRIES; ++attempt) {
            Wire.beginTransmission(DEPTH_I2C_ADDRESS);
            Wire.write(static_cast<uint8_t>(0xA0 + index * 2));
            _lastI2cError = Wire.endTransmission(true);
            if (_lastI2cError != 0) {
                delay(2);
                continue;
            }

            delayMicroseconds(300);
            const uint8_t bytesRead = Wire.requestFrom(static_cast<int>(DEPTH_I2C_ADDRESS), 2);
            if (bytesRead == 2) {
                _prom[index] = static_cast<uint16_t>(Wire.read() << 8);
                _prom[index] |= static_cast<uint16_t>(Wire.read());
                _lastI2cError = 0;
                success = true;
                break;
            }

            while (Wire.available()) {
                (void)Wire.read();
            }
            _lastI2cError = 0xFE;
            delay(2);
        }

        if (!success) {
            _debugStatus = DEBUG_PROM_READ_FAILED;
            return false;
        }
    }

    const uint8_t expectedCrc = static_cast<uint8_t>(_prom[0] >> 12);
    const uint8_t actualCrc = calculatePromCrc(_prom);
    _promCrcValid = expectedCrc == actualCrc;
    if (!_promCrcValid) {
        _debugStatus = DEBUG_PROM_CRC_FAILED;
        return false;
    }

    return true;
}

void DepthSensorManager::printI2cScan() {
    bool foundAny = false;

    Serial.print(F("I2C scan on SDA=IO"));
    Serial.print(DEPTH_I2C_SDA);
    Serial.print(F(", SCL=IO"));
    Serial.println(DEPTH_I2C_SCL);

    for (uint8_t addr = 0x08; addr <= 0x77; ++addr) {
        Wire.beginTransmission(addr);
        if (Wire.endTransmission() == 0) {
            foundAny = true;
            Serial.print(F("  Found I2C device at 0x"));
            if (addr < 0x10) {
                Serial.print('0');
            }
            Serial.println(addr, HEX);
        }
    }

    if (!foundAny) {
        Serial.println(F("  No I2C devices found."));
    }

    for (uint8_t candidate = 0x76; candidate <= 0x77; ++candidate) {
        Wire.beginTransmission(candidate);
        const uint8_t result = Wire.endTransmission();
        Serial.print(F("  Probe 0x"));
        Serial.print(candidate, HEX);
        Serial.print(F(" -> "));
        Serial.println(result == 0 ? F("ACK") : F("no ACK"));
    }
}

uint8_t DepthSensorManager::calculatePromCrc(const uint16_t prom[8]) const {
    uint16_t promCopy[8];
    for (uint8_t i = 0; i < 8; ++i) {
        promCopy[i] = prom[i];
    }

    uint16_t remainder = 0;
    promCopy[0] &= 0x0FFF;
    promCopy[7] = 0;

    for (uint8_t count = 0; count < 16; ++count) {
        if (count & 1U) {
            remainder ^= static_cast<uint16_t>(promCopy[count >> 1] & 0x00FF);
        } else {
            remainder ^= static_cast<uint16_t>(promCopy[count >> 1] >> 8);
        }

        for (uint8_t bit = 0; bit < 8; ++bit) {
            if (remainder & 0x8000) {
                remainder = static_cast<uint16_t>((remainder << 1) ^ 0x3000);
            } else {
                remainder <<= 1;
            }
        }
    }

    return static_cast<uint8_t>((remainder >> 12) & 0x0F);
}

bool DepthSensorManager::writeCommand(uint8_t command) {
    Wire.beginTransmission(DEPTH_I2C_ADDRESS);
    Wire.write(command);
    return Wire.endTransmission() == 0;
}

bool DepthSensorManager::readAdc(uint8_t conversionCommand, uint32_t& value, ReadFailure triggerFailure, ReadFailure fetchFailure) {
    for (uint8_t attempt = 0; attempt < ADC_READ_RETRIES; ++attempt) {
        if (!writeCommand(conversionCommand)) {
            _debugStatus = DEBUG_ADC_READ_FAILED;
            _lastReadFailure = triggerFailure;
            _lastI2cError = 0xFC;
            delay(2);
            continue;
        }

        delay(MS5837_CONVERSION_DELAY_MS);

        Wire.beginTransmission(DEPTH_I2C_ADDRESS);
        Wire.write(MS5837_CMD_ADC_READ);
        _lastI2cError = Wire.endTransmission(true);
        if (_lastI2cError != 0) {
            _debugStatus = DEBUG_ADC_READ_FAILED;
            _lastReadFailure = fetchFailure;
            delay(2);
            continue;
        }

        delayMicroseconds(300);
        const uint8_t bytesRead = Wire.requestFrom(static_cast<int>(DEPTH_I2C_ADDRESS), 3);
        if (bytesRead == 3) {
            value = static_cast<uint32_t>(Wire.read()) << 16;
            value |= static_cast<uint32_t>(Wire.read()) << 8;
            value |= static_cast<uint32_t>(Wire.read());
            _lastReadFailure = READ_OK;
            _lastI2cError = 0;
            return value != 0;
        }

        while (Wire.available()) {
            (void)Wire.read();
        }

        _debugStatus = DEBUG_ADC_READ_FAILED;
        _lastReadFailure = fetchFailure;
        _lastI2cError = 0xFD;
        delay(2);
    }

    return false;
}

bool DepthSensorManager::readMeasurement(float& pressureMbar, float& temperatureC) {
    uint32_t d1 = 0;
    uint32_t d2 = 0;
    if (!readAdc(MS5837_CMD_CONVERT_D1_8192, d1, READ_D1_TRIGGER_FAILED, READ_D1_FETCH_FAILED) ||
        !readAdc(MS5837_CMD_CONVERT_D2_8192, d2, READ_D2_TRIGGER_FAILED, READ_D2_FETCH_FAILED)) {
        return false;
    }

    _lastD1 = d1;
    _lastD2 = d2;

    const int32_t dT = static_cast<int32_t>(d2) - (static_cast<int32_t>(_prom[5]) << 8);
    int64_t temp = 2000LL + ((static_cast<int64_t>(dT) * _prom[6]) >> 23);
    int64_t off = (static_cast<int64_t>(_prom[2]) << 17) +
                  ((static_cast<int64_t>(_prom[4]) * dT) >> 6);
    int64_t sens = (static_cast<int64_t>(_prom[1]) << 16) +
                   ((static_cast<int64_t>(_prom[3]) * dT) >> 7);

    int64_t tempCompensation = 0;
    int64_t offCompensation = 0;
    int64_t sensCompensation = 0;
    if (temp < 2000) {
        const int64_t tempDelta = temp - 2000LL;
        tempCompensation = (11LL * static_cast<int64_t>(dT) * static_cast<int64_t>(dT)) >> 35;
        offCompensation = (31LL * tempDelta * tempDelta) >> 3;
        sensCompensation = (63LL * tempDelta * tempDelta) >> 5;
    }

    temp -= tempCompensation;
    off -= offCompensation;
    sens -= sensCompensation;

    const int64_t pressureRaw = ((((static_cast<int64_t>(d1) * sens) >> 21) - off) >> 15);
    pressureMbar = static_cast<float>(pressureRaw) * 0.01f;
    temperatureC = static_cast<float>(temp) * 0.01f;

    if (pressureMbar < 10.0f || pressureMbar > 2000.0f ||
        temperatureC < -40.0f || temperatureC > 85.0f) {
        _debugStatus = DEBUG_VALUE_OUT_OF_RANGE;
        _lastReadFailure = READ_VALUE_OUT_OF_RANGE;
        return false;
    }

    _lastReadFailure = READ_OK;
    return true;
}

float DepthSensorManager::pressureToDepthCm(float pressureDeltaMbar) const {
    if (pressureDeltaMbar <= 0.0f) {
        return 0.0f;
    }
    return pressureDeltaMbar * CM_PER_MBAR;
}

void DepthSensorManager::commitDepth(float depthCm, float temperatureC, float pressureMbar) {
    const uint32_t now = millis();

    _rawDepthCm = depthCm;
    _pressureMbar = pressureMbar;

    if (!_filterInitialized) {
        _depthFilter.reset(depthCm, 0.0f);
        _filteredDepthCm = depthCm;
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

        _depthFilter.update(depthCm, dt);
        _filteredDepthCm = _depthFilter.getPosition();
        _depthSpeedCmS = _depthFilter.getVelocity();
    }

    _temperatureC = temperatureC;
    _lastUpdate = now;
    _lastFilterUpdate = now;
    _valid = true;
}

const __FlashStringHelper* DepthSensorManager::debugStatusString(DebugStatus status) {
    switch (status) {
        case DEBUG_NOT_STARTED:
            return F("not_started");
        case DEBUG_I2C_INIT_FAILED:
            return F("i2c_init_failed");
        case DEBUG_RESET_FAILED:
            return F("reset_failed");
        case DEBUG_PROM_READ_FAILED:
            return F("prom_read_failed");
        case DEBUG_PROM_CRC_FAILED:
            return F("prom_crc_failed");
        case DEBUG_ADC_READ_FAILED:
            return F("adc_read_failed");
        case DEBUG_VALUE_OUT_OF_RANGE:
            return F("value_out_of_range");
        case DEBUG_VALID_DATA:
            return F("valid");
        case DEBUG_STALE:
            return F("stale");
        default:
            return F("unknown");
    }
}

const __FlashStringHelper* DepthSensorManager::readFailureString(ReadFailure failure) {
    switch (failure) {
        case READ_OK:
            return F("ok");
        case READ_D1_TRIGGER_FAILED:
            return F("d1_trigger");
        case READ_D1_FETCH_FAILED:
            return F("d1_fetch");
        case READ_D2_TRIGGER_FAILED:
            return F("d2_trigger");
        case READ_D2_FETCH_FAILED:
            return F("d2_fetch");
        case READ_VALUE_OUT_OF_RANGE:
            return F("range");
        default:
            return F("unknown");
    }
}
