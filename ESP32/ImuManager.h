/**********************************************************************
 * ImuManager.h
 *
 * EBIMU-9DOFV6（E2BOX 9 轴 AHRS）读取与解析。
 * 接入 CH9434A UART3，115200 bps。
 * IMU 默认 ASCII 推送 Euler 角：`*<roll>,<pitch>,<yaw>\r\n`（每行一帧）。
 *
 * 上电时发送固化配置：sof1 / sor20 (50Hz) / 关闭 gyro/accel/mag/timestamp
 * / pons1，保证无论 IMU 之前处于什么状态都能得到一致数据。
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
    // 角速度（开启 <sog1> 后输出，单位 deg/s），用于训练特征
    float getGyroX() const { return _gyroX; }
    float getGyroY() const { return _gyroY; }
    float getGyroZ() const { return _gyroZ; }

    uint32_t getSampleCount() const { return _sampleCount; }
    uint32_t getParseErrorCount() const { return _parseErrorCount; }
    const __FlashStringHelper* getStatusText() const;

    void printDebug() const;

private:
    // IMU 启动时发送一串配置命令
    void sendConfigSequence();
    void sendCommand(const char* cmd);

    // 解析一行 `*<r>,<p>,<y>` 形式的数据
    bool parseLine(const char* line);

    CH9434A* _ch9434;

    char     _lineBuf[96];
    uint8_t  _lineLen;

    float    _roll;
    float    _pitch;
    float    _yaw;
    float    _gyroX;
    float    _gyroY;
    float    _gyroZ;

    bool     _valid;
    uint32_t _lastFrameMs;
    uint32_t _sampleCount;
    uint32_t _parseErrorCount;

    DebugStatus _debugStatus;
};

#endif // ESP32_IMU_MANAGER_H
