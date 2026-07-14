/**********************************************************************
 * SDLogger.cpp
 *********************************************************************/

#include "SDLogger.h"
#include "Protocol.h"
#include "TeeStream.h"

namespace {
// sensors.csv 保持打开，写入 FLUSH_EVERY_N_ROWS 行后 flush。
// 20 Hz × 20 行 = 1 s flush 一次，最多丢 1 s 数据。
// SD 吞吐余量充足，即使 GC stall 也不会丢行。
constexpr uint8_t FLUSH_EVERY_N_ROWS = 20;
}  // namespace

SDLogger::SDLogger()
    : _spi(HSPI), _sdReady(false), _sessionActive(false), _rowsSinceFlush(0) {
    _folder[0] = '\0';
    _sessionId[0] = '\0';
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

    // 防止文件夹名碰撞导致 FILE_WRITE 静默截断历史 session：
    //   - NTP 不同步时，folderName == "no_time"，跨多次 mt 会撞车
    //   - NTP 同步但同一秒内连续 mt → md → mt，时间戳也会撞车
    // 若已存在则追加 _1 / _2 ... 直到找到可用名字。
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

    // session_id = 文件夹名去掉前导 '/'，写入每行首列供训练按 session 切分
    strncpy(_sessionId, _folder[0] == '/' ? _folder + 1 : _folder, sizeof(_sessionId));
    _sessionId[sizeof(_sessionId) - 1] = '\0';

    char path[52];

    // sensors.csv —— 保持打开的高频写入文件
    buildPath(path, sizeof(path), "sensors.csv");
    _sensorFile = SD.open(path, FILE_WRITE);
    if (!_sensorFile) {
        g_dbg->println(F("[SDLogger] Cannot create sensors.csv"));
        return false;
    }
    _sensorFile.println(F("session_id,timestamp_ms,dt_ms,robot_mode,control_mode,depth_valid,imu_valid,battery_v,target_depth_cm,filtered_depth_cm,depth_speed_cm_s,depth_accel_cm_s2,roll_deg,pitch_deg,yaw_deg,gyro_x_deg_s,gyro_y_deg_s,gyro_z_deg_s,front_distance_cm,left_distance_cm,right_distance_cm,depth_err_cm,u_base,u_residual,u_total,buoyancy_dir_applied,buoyancy_pwm_applied,balancing,emergency_stop"));
    _sensorFile.flush();
    _rowsSinceFlush = 0;

    // commands.csv / events.log —— 低频，每次 open/close 即可
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

void SDLogger::logSensor(const SensorLogRow& r) {
    if (!_sessionActive || !_sensorFile) return;

    // 训练数据一律写数值（不写 "--"），无效由 depth_valid/imu_valid 标志位区分，
    // 便于 Python 端 float() 解析；NaN 兜底为 0 防止写出 "nan"。
    auto f = [&](float v, uint8_t d) {
        _sensorFile.print(isnan(v) ? 0.0f : v, d);
        _sensorFile.print(',');
    };

    _sensorFile.print(_sessionId);          _sensorFile.print(',');
    _sensorFile.print(r.timestampMs);       _sensorFile.print(',');
    _sensorFile.print(r.dtMs);              _sensorFile.print(',');
    _sensorFile.print(r.robotMode);         _sensorFile.print(',');
    _sensorFile.print(r.controlMode);       _sensorFile.print(',');
    _sensorFile.print(r.depthValid ? 1 : 0);_sensorFile.print(',');
    _sensorFile.print(r.imuValid ? 1 : 0);  _sensorFile.print(',');
    f(r.batteryV, 2);
    f(r.targetDepthCm, 2);
    f(r.filteredDepthCm, 2);
    f(r.depthSpeedCmS, 2);
    f(r.depthAccelCmS2, 2);
    f(r.roll, 2); f(r.pitch, 2); f(r.yaw, 2);
    f(r.gyroX, 2); f(r.gyroY, 2); f(r.gyroZ, 2);
    f(r.frontCm, 1); f(r.leftCm, 1); f(r.rightCm, 1);
    f(r.depthErrCm, 2);
    f(r.uBase, 3); f(r.uResidual, 3); f(r.uTotal, 3);
    _sensorFile.print(r.buoyancyDir);       _sensorFile.print(',');
    _sensorFile.print(r.buoyancyPwm);       _sensorFile.print(',');
    _sensorFile.print(r.balancing ? 1 : 0); _sensorFile.print(',');
    _sensorFile.print(r.emergencyStop ? 1 : 0);
    _sensorFile.println();

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

    // sensors.csv 由 _sensorFile 保持打开。先 flush 确保 size() 反映最新写入，
    // 否则 vfs_fat 的 fstat 只报告已落盘字节，会少算未刷新的部分。
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

void SDLogger::logEvent(uint32_t ms, const char* msg) {
    if (!_sessionActive) return;

    char path[52];
    File f = SD.open(buildPath(path, sizeof(path), "events.log"), FILE_APPEND);
    if (!f) return;

    f.print(ms); f.print(',');
    f.println(msg);
    f.close();
}
