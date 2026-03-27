/**********************************************************************
 * OtaManager.cpp
 *
 * 这个文件实现 ESP32 OTA 管理器。
 *********************************************************************/

#include "OtaManager.h"

#include <ArduinoOTA.h>
#include <WiFi.h>

#include "OtaConfig.h"

namespace {
bool hasValue(const char* value) {
    return value != nullptr && value[0] != '\0';
}
} // namespace

OtaManager::OtaManager()
    : _ready(false),
      _usingSoftAP(false) {}

void OtaManager::begin() {
    _ready = false;
    _usingSoftAP = false;

    WiFi.persistent(false);
    WiFi.setAutoReconnect(true);

    if (!beginStationMode() && !beginSoftAPMode()) {
        Serial.println(F("OTA network init failed."));
        return;
    }

    configureArduinoOta();
    ArduinoOTA.begin();
    _ready = true;

    Serial.print(F("OTA ready: "));
    Serial.print(OTA_HOSTNAME);
    Serial.print(F(" @ "));
    Serial.println(getLocalIP());

    if (_usingSoftAP) {
        Serial.print(F("OTA mode: SoftAP (SSID="));
        Serial.print(OTA_AP_SSID);
        Serial.println(F(")"));
    } else {
        Serial.print(F("OTA mode: STA (SSID="));
        Serial.print(OTA_STA_SSID);
        Serial.println(F(")"));
    }
}

void OtaManager::handle() {
    if (_ready) {
        ArduinoOTA.handle();
    }
}

bool OtaManager::isReady() const {
    return _ready;
}

bool OtaManager::isUsingSoftAP() const {
    return _usingSoftAP;
}

IPAddress OtaManager::getLocalIP() const {
    return _usingSoftAP ? WiFi.softAPIP() : WiFi.localIP();
}

bool OtaManager::beginStationMode() {
    if (!hasValue(OTA_STA_SSID)) {
        return false;
    }

    Serial.print(F("Connecting OTA Wi-Fi: "));
    Serial.println(OTA_STA_SSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(OTA_STA_SSID, OTA_STA_PASSWORD);

    const uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < OTA_STA_CONNECT_TIMEOUT_MS) {
        delay(250);
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println(F("OTA STA connect timeout, falling back to SoftAP."));
        WiFi.disconnect(true, true);
        return false;
    }

    return true;
}

bool OtaManager::beginSoftAPMode() {
    if (!hasValue(OTA_AP_SSID) || !hasValue(OTA_AP_PASSWORD)) {
        return false;
    }

    Serial.print(F("Starting OTA SoftAP: "));
    Serial.println(OTA_AP_SSID);

    WiFi.mode(WIFI_AP);
    if (!WiFi.softAP(OTA_AP_SSID, OTA_AP_PASSWORD)) {
        Serial.println(F("OTA SoftAP start failed."));
        return false;
    }

    _usingSoftAP = true;
    return true;
}

void OtaManager::configureArduinoOta() {
    ArduinoOTA.setHostname(OTA_HOSTNAME);

    if (hasValue(OTA_PASSWORD)) {
        ArduinoOTA.setPassword(OTA_PASSWORD);
    }

    ArduinoOTA.onStart([]() {
        Serial.println(F("\nOTA start"));
    });

    ArduinoOTA.onEnd([]() {
        Serial.println(F("\nOTA end"));
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        static uint8_t lastPercent = 255;
        const uint8_t percent = total == 0 ? 0 : static_cast<uint8_t>((progress * 100U) / total);
        if (percent != lastPercent && percent % 10 == 0) {
            lastPercent = percent;
            Serial.printf("OTA progress: %u%%\n", percent);
        }
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("OTA error[%u]\n", static_cast<unsigned>(error));
    });
}
