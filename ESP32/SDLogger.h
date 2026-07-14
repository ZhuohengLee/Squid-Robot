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

// sensors.csv 一行的全部字段，对齐 learning 残差训练管线的输入契约
// (lizhuoh9/learning sample_control_telemetry.csv)。
// 约定：所有字段始终写数值（不写 "--"），无效以 *_valid 标志位表示，
// 便于 Python 端 float() 解析与按 valid 过滤。
struct SensorLogRow {
    uint32_t timestampMs;      // = millis()
    uint32_t dtMs;             // 距上一条记录的毫秒数
    uint8_t  robotMode;        // 0=DEBUG 1=TEST
    uint8_t  controlMode;      // 0=MANUAL 1=AUTO
    bool     depthValid;
    bool     imuValid;
    float    batteryV;
    float    targetDepthCm;    // 未 hold 时写 0
    float    filteredDepthCm;
    float    depthSpeedCmS;
    float    depthAccelCmS2;
    float    roll, pitch, yaw;
    float    gyroX, gyroY, gyroZ;
    float    frontCm, leftCm, rightCm;
    float    depthErrCm;       // target - depth，未 hold 时写 0
    float    uBase;            // 手写基线控制器输出
    float    uResidual;        // NN 残差（暂为 0）
    float    uTotal;           // 实际施加 = base + residual
    uint8_t  buoyancyDir;      // STOP/ASCEND/DESCEND/BALANCE
    uint8_t  buoyancyPwm;      // 0-255
    bool     balancing;        // 气压平衡进行中
    bool     emergencyStop;    // 急停锁进行中
};

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

    // 传感器数据 → sensors.csv（session 未开启时静默忽略）。
    // 字段与 learning 训练管线契约对齐，详见 SensorLogRow。
    void logSensor(const SensorLogRow& row);

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
    char     _sessionId[36];    // 不含前导斜杠的 session 名，写入每行 session_id 列
    File     _sensorFile;       // 保持打开以避免高频下频繁 open/close
    uint8_t  _rowsSinceFlush;
};

#endif // ESP32_SD_LOGGER_H
