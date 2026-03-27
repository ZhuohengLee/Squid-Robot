/**********************************************************************
 * OtaManager.h
 *
 * 这个文件声明 ESP32 OTA 管理器。
 * 它优先尝试连接既有 Wi-Fi，失败后退回 SoftAP，确保现场仍可 OTA。
 *********************************************************************/

#ifndef ESP32_OTA_MANAGER_H
#define ESP32_OTA_MANAGER_H

#include <Arduino.h>
#include <IPAddress.h>

class OtaManager {
public:
    OtaManager();

    void begin();
    void handle();

    bool isReady() const;
    bool isUsingSoftAP() const;
    IPAddress getLocalIP() const;

private:
    bool _ready;
    bool _usingSoftAP;

    bool beginStationMode();
    bool beginSoftAPMode();
    void configureArduinoOta();
};

#endif // ESP32_OTA_MANAGER_H
