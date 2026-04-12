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

constexpr char OTA_HOSTNAME[] = "squid-robot";
constexpr char OTA_PASSWORD[] = "12345678";

constexpr uint32_t OTA_CONNECT_TIMEOUT_MS = 15000;

// HTTP SD 文件接口无需账号密码（与 OTA 同一局域网内可访问）。

#endif

#endif // ESP32_OTA_CONFIG_H
