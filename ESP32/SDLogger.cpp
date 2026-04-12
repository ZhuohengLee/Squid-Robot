/**********************************************************************
 * SDLogger.cpp
 *********************************************************************/

#include "SDLogger.h"
#include "Protocol.h"
#include "TeeStream.h"

namespace {
// sensors.csv 保持打开，写入 FLUSH_EVERY_N_ROWS 行后 flush。
// 5 Hz × 5 行 = 1 s flush 一次，最多丢 1 s 数据。
// SD 吞吐余量约 75x，即使 GC stall 200ms 也不会丢行。
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

    char path[52];

    // sensors.csv —— 保持打开的高频写入文件
    buildPath(path, sizeof(path), "sensors.csv");
    _sensorFile = SD.open(path, FILE_WRITE);
    if (!_sensorFile) {
        g_dbg->println(F("[SDLogger] Cannot create sensors.csv"));
        return false;
    }
    _sensorFile.println(F("millis,depth_cm,vz_cms,az_cms2,us_front_cm,us_left_cm,us_right_cm,motion,batt_v"));
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

void SDLogger::logSensor(uint32_t ms,
                          float depthCm, float vzCms, float azCms2,
                          float usFront, float usLeft, float usRight,
                          uint8_t motionStatus, float battV) {
    if (!_sessionActive || !_sensorFile) return;

    _sensorFile.print(ms); _sensorFile.print(',');

    auto wf = [&](float v, uint8_t d) {
        if (v < 0.0f) _sensorFile.print(F("--")); else _sensorFile.print(v, d);
        _sensorFile.print(',');
    };

    wf(depthCm, 2); wf(vzCms, 2); wf(azCms2, 2);
    wf(usFront, 1); wf(usLeft, 1); wf(usRight, 1);
    _sensorFile.print(motionStatus); _sensorFile.print(',');
    if (battV < 0.0f) _sensorFile.print(F("--")); else _sensorFile.print(battV, 2);
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
