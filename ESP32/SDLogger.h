/**********************************************************************
 * SDLogger.h
 *
 * 基于 session 的 SD 卡日志记录器（SPI3/HSPI，IO6/7/8/14）。
 * 每次进入 TEST 模式时以 NTP 时间戳命名新文件夹，分文件记录：
 *   sensors.csv   —— 传感器读数（5 Hz）
 *   commands.csv  —— 收到的所有指令（来源：serial / hc12）
 *   events.log    —— 系统事件（模式切换、急停、mark 标记等）
 *********************************************************************/

#ifndef ESP32_SD_LOGGER_H
#define ESP32_SD_LOGGER_H

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

class SDLogger {
public:
    SDLogger();

    // 初始化 SD 硬件，不开启 session
    bool begin();
    bool isReady() const;
    bool hasSession() const;

    // 开始 / 结束一次测试 session
    // folderName 例如 "2025-04-10_14-30-00" 或 "no_time"
    bool startSession(const char* folderName);
    void endSession(uint32_t ms);

    // 传感器数据 → sensors.csv（session 未开启时静默忽略）
    void logSensor(uint32_t ms,
                   float depthCm, float vzCms, float azCms2,
                   float usFront, float usLeft, float usRight,
                   uint8_t motionStatus, float battV = -1.0f);

    // 指令记录 → commands.csv，source = "serial" 或 "hc12"
    void logCommand(uint32_t ms, const char* source, const char* cmd);

    // 系统事件 → events.log
    void logEvent(uint32_t ms, const char* msg);

    const char* getFolder() const;

    // 打印当前 session 状态和文件大小（用于验证记录是否正常）
    void printStats();

private:
    // 构造 "/<folder>/<leaf>" 路径，返回 dst
    char* buildPath(char* dst, size_t dstLen, const char* leaf) const;

    SPIClass _spi;
    bool     _sdReady;
    bool     _sessionActive;
    char     _folder[36];       // "/<YYYY-MM-DD_HH-MM-SS>"
    File     _sensorFile;       // 保持打开以避免 5 Hz 下频繁 open/close
    uint8_t  _rowsSinceFlush;
};

#endif // ESP32_SD_LOGGER_H
