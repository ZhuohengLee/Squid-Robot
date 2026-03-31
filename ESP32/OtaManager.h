/**********************************************************************
 * OtaManager.h
 *
 * OTA 管理器，集成网页配网（Captive Portal）。
 *
 * 启动流程：
 *   1. 尝试连接 NVS 已保存的 WiFi（5s）
 *   2. 若失败，尝试 OtaConfig.h 中的硬编码 SSID（15s）
 *   3. 若仍失败，启动配网热点 + 强制门户网页，阻塞直到配网成功
 *   4. OTA 服务启动
 *********************************************************************/

#ifndef ESP32_OTA_MANAGER_H
#define ESP32_OTA_MANAGER_H

#include <Arduino.h>
#include <DNSServer.h>
#include <IPAddress.h>
#include <WebServer.h>

class OtaManager {
public:
    OtaManager();

    // underwaterMode=true 时跳过所有 WiFi 初始化
    void begin(bool underwaterMode = false);
    void handle();

    bool isReady() const;
    bool isUsingSoftAP() const;
    IPAddress getLocalIP() const;

private:
    bool      _ready;
    bool      _usingSoftAP;
    bool      _provisioningDone;
    DNSServer _dnsServer;
    WebServer _webServer;

    // 尝试用 NVS 保存的凭据连接（无参数 WiFi.begin）
    bool beginNvsMode(uint32_t deadline);
    // 尝试用 OtaConfig 中硬编码的 SSID 连接
    bool beginStationMode(uint32_t deadline);
    // 启动配网热点并阻塞直到配网完成
    bool beginProvisioningPortal();
    // 启动 SoftAP 兜底 OTA（配网也失败时）
    bool beginSoftAPMode();

    void configureArduinoOta();

    // 配网网页处理
    void handlePortalRoot();
    void handlePortalSave();
    void handlePortalScan();       // 手动触发重新扫描并返回 JSON 片段
    void handlePortalNotFound();
    String buildScanResults();

    bool connectSta(const char* ssid, const char* pass, uint32_t timeoutMs);
};

#endif // ESP32_OTA_MANAGER_H
