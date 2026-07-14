/**********************************************************************
 * ImuManager.h
 *
 * EBIMU-9DOFV6（E2BOX 9 轴 AHRS）读取与解析。
 * 接入 CH9434A UART3，115200 bps。
 * 输出 Euler + Gyro：`*<roll>,<pitch>,<yaw>,<gx>,<gy>,<gz>\r\n`
 *
 * 上电时发送固化配置：sof1 / sog1 / sor20 (50Hz) / 关闭 accel/mag/timestamp
 * / pons1，保证无论 IMU 之前处于什么状态都能得到一致数据。
 *
 * SLAM 集成：gyro_z 用于航向积分，roll/pitch 用于倾斜门控。
 *********************************************************************/

#ifndef ESP32_IMU_MANAGER_H
#define ESP32_IMU_MANAGER_H

#include <Arduino.h>
#include "CH9434A.h"

class ImuManager {
public:
    enum DebugStatus : uint8_t {
        DEBUG_NOT_STARTED,
        DEBUG_WAITING_FIRST_FRAME,
        DEBUG_VALID,
        DEBUG_STALE,
        DEBUG_PARSE_ERROR,
    };

    explicit ImuManager(CH9434A* ch9434);

    bool begin();
    void update();

    bool isValid() const { return _valid; }
    float getRoll()  const { return _roll;  }
    float getPitch() const { return _pitch; }
    float getYaw()   const { return _yaw;   }
    float getGyroX() const { return _gyroX; }
    float getGyroY() const { return _gyroY; }
    float getGyroZ() const { return _gyroZ; }
    bool  hasGyro()  const { return _hasGyro; }
    const char* getLastRawLine() const { return _lastRawLine; }

    uint32_t getSampleCount() const { return _sampleCount; }
    uint32_t getParseErrorCount() const { return _parseErrorCount; }
    const __FlashStringHelper* getStatusText() const;

    void printDebug() const;

private:
    // IMU 启动时发送一串配置命令
    void sendConfigSequence();
    void sendCommand(const char* cmd);

    // 解析一行 `*<r>,<p>,<y>[,<gx>,<gy>,<gz>]` 形式的数据
    bool parseLine(const char* line);

    CH9434A* _ch9434;

    char     _lineBuf[128];
    char     _lastRawLine[128];  // 最近一次完整解析成功的原始行，供诊断用
    uint8_t  _lineLen;

    float    _roll;
    float    _pitch;
    float    _yaw;
    float    _gyroX;          // deg/s（EBIMU 原生输出，开启 sog1 时）
    float    _gyroY;          // deg/s
    float    _gyroZ;          // deg/s（若 EBIMU 不输出，由 yaw 差分计算）
    bool     _hasGyro;        // 是否有任何来源的 gyro 数据（原生 OR 差分）
    bool     _hasNativeGyro;  // parseLine 解析到 6 字段（EBIMU 原生 gyro 开启）
    float    _prevYaw;        // 差分基线 yaw
    uint32_t _prevYawMs;      // 差分基线时间戳

    bool     _valid;
    uint32_t _lastFrameMs;
    uint32_t _sampleCount;
    uint32_t _parseErrorCount;

    DebugStatus _debugStatus;
};

#endif // ESP32_IMU_MANAGER_H
