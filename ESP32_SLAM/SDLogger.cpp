/**********************************************************************
 * SDLogger.cpp
 *
 * 全量传感器 + 控制状态记录。
 * 列名保持与 python_local_mapper.py 兼容（us_front_cm / motion_mode /
 * gyro_z_deg_s 等），同时扩展记录 IMU 全部轴、深度控制、执行器状态。
 *********************************************************************/

#include "SDLogger.h"
#include "Protocol.h"
#include "TeeStream.h"

namespace {
constexpr uint8_t FLUSH_EVERY_N_ROWS = 5;
}  // namespace

SDLogger::SDLogger()
    : _spi(HSPI), _sdReady(false), _sessionActive(false), _rowsSinceFlush(0) {
    _folder[0] = '\0';
}

bool SDLogger::begin() {
    _sdReady = false;
    _spi.begin(SD_SPI_SCK, SD_SPI_MISO, SD_SPI_MOSI, SD_SPI_CS);
    if (!SD.begin(SD_SPI_CS, _spi)) {
        g_dbg->println(F("[SDLogger] Mount failed. Check wiring/FAT32."));
        return false;
    }
    const uint32_t mb = (uint32_t)(SD.cardSize() / (1024ULL * 1024ULL));
    g_dbg->print(F("[SDLogger] SD ready, "));
    g_dbg->print(mb);
    g_dbg->println(F(" MB"));
    _sdReady = true;
    return true;
}

bool SDLogger::isReady()   const { return _sdReady; }
bool SDLogger::hasSession() const { return _sessionActive; }
const char* SDLogger::getFolder() const { return _folder; }

char* SDLogger::buildPath(char* dst, size_t dstLen, const char* leaf) const {
    snprintf(dst, dstLen, "%s/%s", _folder, leaf);
    return dst;
}

bool SDLogger::startSession(const char* folderName) {
    if (!_sdReady) return false;
    if (_sessionActive) endSession(millis());

    // 防止文件夹名碰撞
    char base[32];
    snprintf(base, sizeof(base), "/%s", folderName);
    strncpy(_folder, base, sizeof(_folder));
    _folder[sizeof(_folder) - 1] = '\0';
    for (uint16_t suffix = 1; suffix < 1000 && SD.exists(_folder); ++suffix) {
        snprintf(_folder, sizeof(_folder), "%s_%u", base, suffix);
    }
    if (SD.exists(_folder)) {
        g_dbg->println(F("[SDLogger] 文件夹名冲突上限，放弃 session"));
        return false;
    }
    if (!SD.mkdir(_folder)) {
        g_dbg->print(F("[SDLogger] mkdir 失败: "));
        g_dbg->println(_folder);
        return false;
    }

    char path[52];

    // sensors.csv —— 全量数据，SLAM mapper 兼容列名
    // python_local_mapper.py 读取的列：
    //   timestamp_ms, motion_mode, imu_valid, roll_deg, pitch_deg,
    //   gyro_z_deg_s, us_front_cm, us_left_cm, us_right_cm
    // 其余列供训练 / 调试 / 后续扩展使用，mapper 自动忽略
    buildPath(path, sizeof(path), "sensors.csv");
    _sensorFile = SD.open(path, FILE_WRITE);
    if (!_sensorFile) {
        g_dbg->println(F("[SDLogger] Cannot create sensors.csv"));
        return false;
    }
    // 36 列格式，对齐 learning 仓库训练 CSV 规范
    // https://github.com/ZhuohengLee/learning/blob/main/README.md
    _sensorFile.println(
        F("session_id,timestamp_ms,dt_ms,robot_mode,control_mode,"
          "depth_valid,imu_valid,battery_v,"
          "target_depth_cm,filtered_depth_cm,depth_speed_cm_s,depth_accel_cm_s2,"
          "roll_deg,pitch_deg,gyro_x_deg_s,gyro_y_deg_s,gyro_z_deg_s,"
          "front_distance_cm,left_distance_cm,right_distance_cm,"
          "depth_err_cm,u_base,u_residual,u_total,"
          "forward_cmd_base,forward_cmd_residual,forward_cmd_total,forward_phase_interval_ms,"
          "yaw_cmd_base,yaw_cmd_residual,yaw_cmd_total,"
          "buoyancy_dir_applied,buoyancy_pwm_applied,actuator_mask,balancing,emergency_stop")
    );
    _sensorFile.flush();
    _rowsSinceFlush = 0;

    // commands.csv / events.log
    auto writeHeader = [&](const char* leaf, const __FlashStringHelper* header) {
        File f = SD.open(buildPath(path, sizeof(path), leaf), FILE_WRITE);
        if (f) { f.println(header); f.close(); }
    };
    writeHeader("commands.csv", F("millis,source,command"));
    writeHeader("events.log",   F("millis,event"));

    _sessionActive = true;
    g_dbg->print(F("[SDLogger] Session: "));
    g_dbg->println(_folder);
    return true;
}

void SDLogger::endSession(uint32_t ms) {
    if (!_sessionActive) return;
    logEvent(ms, "session_end");
    if (_sensorFile) {
        _sensorFile.flush();
        _sensorFile.close();
    }
    _sessionActive = false;
    _rowsSinceFlush = 0;
    g_dbg->println(F("[SDLogger] Session closed."));
}

void SDLogger::logControlFrame(
    uint32_t timestampMs,
    uint32_t dtMs,
    uint8_t  robotMode,
    uint8_t  controlMode,
    bool     depthValid,
    bool     imuValid,
    float    batteryV,
    float    targetDepthCm,
    float    filteredDepthCm,
    float    depthSpeedCmS,
    float    depthAccelCmS2,
    float    rollDeg,
    float    pitchDeg,
    float    gyroXDegS,
    float    gyroYDegS,
    float    gyroZDegS,
    float    frontDistanceCm,
    float    leftDistanceCm,
    float    rightDistanceCm,
    float    depthErrCm,
    float    uBase,
    float    uResidual,
    float    uTotal,
    float    forwardBase,
    float    forwardResidual,
    float    forwardTotal,
    uint32_t forwardPhaseIntervalMs,
    float    yawBase,
    float    yawResidual,
    float    yawTotal,
    uint8_t  buoyancyDirApplied,
    uint8_t  buoyancyPwmApplied,
    uint16_t actuatorMask,
    bool     balancing,
    bool     emergencyStop
) {
    if (!_sessionActive || !_sensorFile) return;

    // 无效传感器值写空字段（pandas 默认读为 NaN，兼容 learning 训练管线）
    auto wf = [&](float v, uint8_t d) {
        if (v < -9000.0f) { /* 空字段 */ }
        else _sensorFile.print(v, d);
    };

    // session_id 使用 folder 名去掉开头 '/'
    const char* sessionId = (_folder[0] == '/') ? (_folder + 1) : _folder;

    // 时间 & 状态
    _sensorFile.print(sessionId);             _sensorFile.print(',');
    _sensorFile.print(timestampMs);           _sensorFile.print(',');
    _sensorFile.print(dtMs);                  _sensorFile.print(',');
    _sensorFile.print(robotMode);             _sensorFile.print(',');
    _sensorFile.print(controlMode);           _sensorFile.print(',');
    _sensorFile.print(depthValid ? 1 : 0);    _sensorFile.print(',');
    _sensorFile.print(imuValid ? 1 : 0);      _sensorFile.print(',');
    wf(batteryV, 2);                          _sensorFile.print(',');

    // 深度
    wf(targetDepthCm, 2);                     _sensorFile.print(',');
    wf(filteredDepthCm, 2);                   _sensorFile.print(',');
    wf(depthSpeedCmS, 2);                     _sensorFile.print(',');
    wf(depthAccelCmS2, 2);                    _sensorFile.print(',');

    // IMU: roll/pitch + gyro x/y/z（不含 yaw 绝对值，训练用 gyro 积分）
    wf(rollDeg, 2);                           _sensorFile.print(',');
    wf(pitchDeg, 2);                          _sensorFile.print(',');
    wf(gyroXDegS, 3);                         _sensorFile.print(',');
    wf(gyroYDegS, 3);                         _sensorFile.print(',');
    wf(gyroZDegS, 3);                         _sensorFile.print(',');

    // 超声波
    wf(frontDistanceCm, 1);                   _sensorFile.print(',');
    wf(leftDistanceCm, 1);                    _sensorFile.print(',');
    wf(rightDistanceCm, 1);                   _sensorFile.print(',');

    // 深度控制拆分（base + residual = total）
    wf(depthErrCm, 2);                        _sensorFile.print(',');
    wf(uBase, 2);                             _sensorFile.print(',');
    wf(uResidual, 2);                         _sensorFile.print(',');
    wf(uTotal, 2);                            _sensorFile.print(',');

    // 前进指令拆分
    wf(forwardBase, 2);                       _sensorFile.print(',');
    wf(forwardResidual, 2);                   _sensorFile.print(',');
    wf(forwardTotal, 2);                      _sensorFile.print(',');
    _sensorFile.print(forwardPhaseIntervalMs);_sensorFile.print(',');

    // 偏航指令拆分
    wf(yawBase, 2);                           _sensorFile.print(',');
    wf(yawResidual, 2);                       _sensorFile.print(',');
    wf(yawTotal, 2);                          _sensorFile.print(',');

    // 执行器 & 标志
    _sensorFile.print(buoyancyDirApplied);    _sensorFile.print(',');
    _sensorFile.print(buoyancyPwmApplied);    _sensorFile.print(',');
    _sensorFile.print(actuatorMask);          _sensorFile.print(',');
    _sensorFile.print(balancing ? 1 : 0);     _sensorFile.print(',');
    _sensorFile.println(emergencyStop ? 1 : 0);

    if (++_rowsSinceFlush >= FLUSH_EVERY_N_ROWS) {
        _sensorFile.flush();
        _rowsSinceFlush = 0;
    }
}

void SDLogger::logCommand(uint32_t ms, const char* source, const char* cmd) {
    if (!_sessionActive) return;

    char path[52];
    File f = SD.open(buildPath(path, sizeof(path), "commands.csv"), FILE_APPEND);
    if (!f) return;

    f.print(ms);     f.print(',');
    f.print(source); f.print(',');
    f.println(cmd);
    f.close();
}

void SDLogger::logEvent(uint32_t ms, const char* msg) {
    if (!_sessionActive) return;

    char path[52];
    File f = SD.open(buildPath(path, sizeof(path), "events.log"), FILE_APPEND);
    if (!f) return;

    f.print(ms); f.print(',');
    f.println(msg);
    f.close();
}

void SDLogger::printStats() {
    if (!_sdReady) {
        g_dbg->println(F("[SD] 未就绪"));
        return;
    }
    if (!_sessionActive) {
        g_dbg->println(F("[SD] 无活跃 session（需先进入 MT 模式）"));
        return;
    }
    g_dbg->print(F("[SD] session: "));
    g_dbg->println(_folder);

    g_dbg->print(F("  sensors.csv: "));
    if (_sensorFile) {
        _sensorFile.flush();
        g_dbg->print(_sensorFile.size());
        g_dbg->println(F(" 字节"));
    } else {
        g_dbg->println(F("无法打开"));
    }

    char path[52];
    const char* files[] = {"commands.csv", "events.log"};
    for (uint8_t i = 0; i < 2; i++) {
        File f = SD.open(buildPath(path, sizeof(path), files[i]), FILE_READ);
        g_dbg->print(F("  "));
        g_dbg->print(files[i]);
        g_dbg->print(F(": "));
        if (f) {
            g_dbg->print(f.size());
            g_dbg->println(F(" 字节"));
            f.close();
        } else {
            g_dbg->println(F("无法打开"));
        }
    }
    const uint32_t mb = (uint32_t)(SD.cardSize() / (1024ULL * 1024ULL));
    g_dbg->print(F("  SD 卡容量: "));
    g_dbg->print(mb);
    g_dbg->println(F(" MB"));
}
