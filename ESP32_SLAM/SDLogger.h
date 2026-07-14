/**********************************************************************
 * SDLogger.h
 *
 * 基于 session 的 SD 卡日志记录器（SPI3/HSPI，IO6/7/8/14）。
 * 每次进入 TEST 模式时以 NTP 时间戳命名新文件夹，分文件记录：
 *   sensors.csv   —— 36 列全量数据，匹配 learning 仓库训练 CSV 规范
 *   commands.csv  —— 收到的所有指令
 *   events.log    —— 系统事件
 *
 * CSV 格式对齐：
 *   https://github.com/ZhuohengLee/learning/blob/main/README.md
 * 同时 SLAM mapper 通过列名别名兼容：front_distance_cm ↔ us_front_cm 等
 *********************************************************************/

#ifndef ESP32_SD_LOGGER_H
#define ESP32_SD_LOGGER_H

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

class SDLogger {
public:
    SDLogger();

    bool begin();
    bool isReady() const;
    bool hasSession() const;

    bool startSession(const char* folderName);
    void endSession(uint32_t ms);

    // 36 列训练格式日志。session_id 自动使用 folder 名。
    // 当前控制器没有 base/residual 拆分，所有 residual 字段传 0，total = base。
    // 无效传感器值用 MISSING_VALUE (-9999.0f) 表示，CSV 写 "--"。
    void logControlFrame(
        uint32_t timestampMs,
        uint32_t dtMs,
        uint8_t  robotMode,          // 0=DEBUG, 1=TEST
        uint8_t  controlMode,        // cmdHandler.getMode()
        bool     depthValid,
        bool     imuValid,
        float    batteryV,
        // 深度
        float    targetDepthCm,
        float    filteredDepthCm,
        float    depthSpeedCmS,
        float    depthAccelCmS2,
        // IMU（Euler + Gyro）
        float    rollDeg,
        float    pitchDeg,
        float    gyroXDegS,
        float    gyroYDegS,
        float    gyroZDegS,
        // 超声波
        float    frontDistanceCm,
        float    leftDistanceCm,
        float    rightDistanceCm,
        // 深度控制（base + residual = total）
        float    depthErrCm,
        float    uBase,
        float    uResidual,
        float    uTotal,
        // 前进指令拆分
        float    forwardBase,
        float    forwardResidual,
        float    forwardTotal,
        uint32_t forwardPhaseIntervalMs,
        // 偏航指令拆分
        float    yawBase,
        float    yawResidual,
        float    yawTotal,
        // 执行器 & 状态
        uint8_t  buoyancyDirApplied,
        uint8_t  buoyancyPwmApplied,
        uint16_t actuatorMask,
        bool     balancing,
        bool     emergencyStop
    );

    void logCommand(uint32_t ms, const char* source, const char* cmd);
    void logEvent(uint32_t ms, const char* msg);

    const char* getFolder() const;
    void printStats();

    static constexpr float MISSING_VALUE = -9999.0f;

private:
    char* buildPath(char* dst, size_t dstLen, const char* leaf) const;

    SPIClass _spi;
    bool     _sdReady;
    bool     _sessionActive;
    char     _folder[36];
    File     _sensorFile;
    uint8_t  _rowsSinceFlush;
};

#endif // ESP32_SD_LOGGER_H
