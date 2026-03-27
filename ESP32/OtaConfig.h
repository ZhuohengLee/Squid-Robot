/**********************************************************************
 * OtaConfig.h
 *
 * 这个文件提供 ESP32 OTA 的默认配置。
 * 如果需要保留本地私密配置，请新建 OtaConfig.local.h 覆盖这些值。
 *********************************************************************/

#ifndef ESP32_OTA_CONFIG_H
#define ESP32_OTA_CONFIG_H

#if __has_include("OtaConfig.local.h")
#include "OtaConfig.local.h"
#else

constexpr char OTA_HOSTNAME[] = "squid-receiver-esp32";
constexpr char OTA_PASSWORD[] = "receiver-ota";

// 为空时跳过 STA 连接，直接启用 SoftAP OTA。
constexpr char OTA_STA_SSID[] = "";
constexpr char OTA_STA_PASSWORD[] = "";

constexpr char OTA_AP_SSID[] = "SquidReceiver-OTA";
constexpr char OTA_AP_PASSWORD[] = "receiver-ota";

constexpr uint32_t OTA_STA_CONNECT_TIMEOUT_MS = 15000;

#endif

#endif // ESP32_OTA_CONFIG_H
