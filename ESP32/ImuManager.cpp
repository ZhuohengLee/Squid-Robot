/**********************************************************************
 * ImuManager.cpp
 *
 * EBIMU-9DOFV6 解析实现。
 *********************************************************************/

#include "ImuManager.h"
#include "Protocol.h"
#include "TeeStream.h"

namespace {
constexpr uint32_t FRAME_TIMEOUT_MS = 500;   // 500ms 无新帧视为 stale
constexpr uint32_t CONFIG_CMD_GAP_MS = 100;  // 命令之间的间隔，等 <ok> 回复
}  // namespace

ImuManager::ImuManager(CH9434A* ch9434)
    : _ch9434(ch9434),
      _lineLen(0),
      _roll(0.0f),
      _pitch(0.0f),
      _yaw(0.0f),
      _gyroX(0.0f),
      _gyroY(0.0f),
      _gyroZ(0.0f),
      _valid(false),
      _lastFrameMs(0),
      _sampleCount(0),
      _parseErrorCount(0),
      _debugStatus(DEBUG_NOT_STARTED) {
    _lineBuf[0] = '\0';
}

bool ImuManager::begin() {
    if (!_ch9434) return false;

    if (!_ch9434->config(IMU_UART, IMU_BAUDRATE, CH9434A_LCR_8N1)) {
        return false;
    }
    _ch9434->flush(IMU_UART);

    // 下发固化配置。命令格式见 EBIMU-9DOFV6 datasheet 5-3 节。
    sendConfigSequence();

    _ch9434->flush(IMU_UART);   // 清掉配置期间 IMU 回复的 <ok> 字节
    _lineLen = 0;
    _debugStatus = DEBUG_WAITING_FIRST_FRAME;
    return true;
}

void ImuManager::sendCommand(const char* cmd) {
    _ch9434->print(IMU_UART, cmd);
    delay(CONFIG_CMD_GAP_MS);
}

void ImuManager::sendConfigSequence() {
    // 停掉数据输出，进入命令配置
    sendCommand("<stop>");
    // Euler 角输出（roll/pitch/yaw）
    sendCommand("<sof1>");
    // 开启陀螺角速度输出（gyro x/y/z），作为残差控制训练特征
    sendCommand("<sog1>");
    sendCommand("<soa0>");
    sendCommand("<som0>");
    sendCommand("<sod0>");
    sendCommand("<sot0>");
    sendCommand("<sots0>");
    // 输出间隔 20ms = 50Hz（2x 过采样 SD 5Hz 记录）
    sendCommand("<sor20>");
    // 上电自动启动
    sendCommand("<pons1>");
    // 启动输出
    sendCommand("<start>");
}

void ImuManager::update() {
    if (!_ch9434) return;

    // 非阻塞逐字节读取：收到 '\n' 时完整解析一帧
    while (_ch9434->available(IMU_UART) > 0) {
        uint8_t c = _ch9434->read(IMU_UART);

        if (c == '\r') continue;  // 忽略 CR
        if (c == '\n') {
            _lineBuf[_lineLen] = '\0';
            if (_lineLen > 0) {
                if (parseLine(_lineBuf)) {
                    _lastFrameMs = millis();
                    _valid = true;
                    _sampleCount++;
                    _debugStatus = DEBUG_VALID;
                } else {
                    _parseErrorCount++;
                    _debugStatus = DEBUG_PARSE_ERROR;
                }
            }
            _lineLen = 0;
            continue;
        }

        if (_lineLen < sizeof(_lineBuf) - 1) {
            _lineBuf[_lineLen++] = (char)c;
        } else {
            // 超长行视为噪声，丢弃
            _lineLen = 0;
            _parseErrorCount++;
        }
    }

    // 超时降级为 stale
    const uint32_t now = millis();
    if (_valid && (now - _lastFrameMs) > FRAME_TIMEOUT_MS) {
        _valid = false;
        _debugStatus = DEBUG_STALE;
    }
}

bool ImuManager::parseLine(const char* line) {
    // 合法帧首字节是 '*'：`*-25.46,47.24,-35.77`
    if (line[0] != '*') return false;

    const char* p = line + 1;

    char* endptr = nullptr;
    float r = strtof(p, &endptr);
    if (endptr == p || *endptr != ',') return false;
    p = endptr + 1;

    float pi = strtof(p, &endptr);
    if (endptr == p || *endptr != ',') return false;
    p = endptr + 1;

    float y = strtof(p, &endptr);
    if (endptr == p) return false;

    // EBIMU Roll∈[-180,180], Pitch∈[-90,90], Yaw∈[-180,180]
    if (r < -180.0f || r > 180.0f) return false;
    if (pi < -90.0f || pi > 90.0f) return false;
    if (y < -180.0f || y > 180.0f) return false;

    // 可选的角速度 gyro x/y/z（<sog1> 开启后跟在 yaw 之后；旧固件无则保持 0）
    float gx = 0.0f, gy = 0.0f, gz = 0.0f;
    if (*endptr == ',') {
        p = endptr + 1; gx = strtof(p, &endptr);
        if (*endptr == ',') {
            p = endptr + 1; gy = strtof(p, &endptr);
            if (*endptr == ',') { p = endptr + 1; gz = strtof(p, &endptr); }
        }
    }

    _roll  = r;
    _pitch = pi;
    _yaw   = y;
    _gyroX = gx;
    _gyroY = gy;
    _gyroZ = gz;
    return true;
}

const __FlashStringHelper* ImuManager::getStatusText() const {
    switch (_debugStatus) {
        case DEBUG_NOT_STARTED:         return F("not_started");
        case DEBUG_WAITING_FIRST_FRAME: return F("waiting");
        case DEBUG_VALID:               return F("valid");
        case DEBUG_STALE:               return F("stale");
        case DEBUG_PARSE_ERROR:         return F("parse_error");
    }
    return F("unknown");
}

void ImuManager::printDebug() const {
    g_dbg->print(F("IMU UART3: samples="));
    g_dbg->print(_sampleCount);
    g_dbg->print(F(" | parse_err="));
    g_dbg->print(_parseErrorCount);
    g_dbg->print(F(" | status="));
    g_dbg->println(getStatusText());
    g_dbg->print(F("  roll="));  g_dbg->print(_roll, 2);
    g_dbg->print(F(" pitch=")); g_dbg->print(_pitch, 2);
    g_dbg->print(F(" yaw="));   g_dbg->print(_yaw, 2);
    g_dbg->println(F(" deg"));
}
