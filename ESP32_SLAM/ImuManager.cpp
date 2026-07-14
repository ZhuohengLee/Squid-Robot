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
constexpr uint32_t CONFIG_CMD_GAP_MS = 500;  // 命令间隔 500ms，EBIMU 处理一条约需 500ms
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
      _hasGyro(false),
      _hasNativeGyro(false),
      _prevYaw(0.0f),
      _prevYawMs(0),
      _valid(false),
      _lastFrameMs(0),
      _sampleCount(0),
      _parseErrorCount(0),
      _debugStatus(DEBUG_NOT_STARTED) {
    _lineBuf[0] = '\0';
    _lastRawLine[0] = '\0';
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
    while (_ch9434->available(IMU_UART) > 0) {
        _ch9434->read(IMU_UART);
    }
}

// 读响应直到 '>' 或超时
static int readResponse(CH9434A* ch, uint8_t uart, char* resp, int respLen, uint32_t timeoutMs) {
    const uint32_t t0 = millis();
    int idx = 0;
    while (millis() - t0 < timeoutMs && idx < respLen - 1) {
        if (ch->available(uart) > 0) {
            char c = ch->read(uart);
            if (idx < respLen - 1) resp[idx++] = c;
            if (c == '>') break;
        }
    }
    resp[idx] = '\0';
    return idx;
}

// 发送一条命令并等响应。如果响应里没有 '<ok>' 或 '<er>'，说明 EBIMU 没进入命令模式
static bool sendCmdAndVerify(CH9434A* ch, uint8_t uart, const char* cmd, const char* desc) {
    // 先单独发 '<' 触发 EBIMU 进入命令模式（手册 3-2 节），
    // 等它停止数据流，再读干净缓冲
    ch->print(uart, "<");
    delay(150);
    while (ch->available(uart) > 0) ch->read(uart);
    delay(50);

    // 然后发命令主体（跳过开头的 '<'），末尾 '>' 触发执行
    ch->print(uart, cmd + 1);   // 跳过 '<'

    char resp[64];
    const int n = readResponse(ch, uart, resp, sizeof(resp), 800);
    const bool ok = (strstr(resp, "<ok>") != nullptr);

    g_dbg->print(F("[IMU cfg] "));
    g_dbg->print(cmd);
    g_dbg->print(F(" ("));
    g_dbg->print(desc);
    g_dbg->print(F(") → "));
    if (n == 0) {
        g_dbg->println(F("(无响应)"));
    } else if (ok) {
        g_dbg->println(F("<ok>"));
    } else {
        g_dbg->print(F("未确认 raw='"));
        g_dbg->print(resp);
        g_dbg->println(F("'"));
    }
    return ok;
}

void ImuManager::sendConfigSequence() {
    // 充分等上电自启的数据流稳定
    delay(1000);
    while (_ch9434->available(IMU_UART) > 0) _ch9434->read(IMU_UART);

    // 先发 <reset> 彻底重置，相当于重新上电（手册 6-4-6）
    _ch9434->print(IMU_UART, "<reset>");
    g_dbg->println(F("[IMU cfg] <reset> 已发送，等 2s"));
    delay(2000);
    while (_ch9434->available(IMU_UART) > 0) _ch9434->read(IMU_UART);

    // 字符分开发 '<' 和命令，给 EBIMU 切换到命令模式的时间
    struct Cmd { const char* cmd; const char* desc; };
    const Cmd cmds[] = {
        {"<sof1>",  "Euler"},
        {"<sog1>",  "Gyro"},      // SLAM 关键
        {"<soa0>",  "Acc off"},
        {"<som0>",  "Mag off"},
        {"<sod0>",  "Dist off"},
        {"<sot0>",  "Temp off"},
        {"<sots0>", "TS off"},
        {"<sor20>", "50Hz"},
        {"<pons1>", "AutoStart"},
    };
    for (const auto& c : cmds) {
        sendCmdAndVerify(_ch9434, IMU_UART, c.cmd, c.desc);
    }

    // start 数据输出
    sendCmdAndVerify(_ch9434, IMU_UART, "<start>", "Start");
    delay(200);
    while (_ch9434->available(IMU_UART) > 0) _ch9434->read(IMU_UART);
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
                // 诊断：缓存最后一次收到的行，供显示时查看
                strncpy(_lastRawLine, _lineBuf, sizeof(_lastRawLine) - 1);
                _lastRawLine[sizeof(_lastRawLine) - 1] = '\0';
                // 启动后前 5 帧也直接打印一份（方便看初始化状态）
                if (_sampleCount < 5) {
                    g_dbg->print(F("[IMU raw] '"));
                    g_dbg->print(_lineBuf);
                    g_dbg->println(F("'"));
                }
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

    // update() 结束时，若没有 EBIMU 原生 gyro，用 yaw 差分算 gyroZ。
    // 关键修正：每次都要重新计算！基于 _hasNativeGyro（不是 _hasGyro），
    // 否则 _hasGyro 被首次算出后置 true，以后再不进入这个分支，值冻结。
    if (_valid && !_hasNativeGyro) {
        const uint32_t nowMs = millis();
        if (_prevYawMs == 0) {
            // 首次，记录基线
            _prevYaw = _yaw;
            _prevYawMs = nowMs;
        } else if (nowMs > _prevYawMs + 20) {  // 至少 20ms 间隔才更新
            const float dt = (nowMs - _prevYawMs) * 0.001f;
            if (dt < 0.5f) {
                float dyaw = _yaw - _prevYaw;
                while (dyaw > 180.0f)  dyaw -= 360.0f;
                while (dyaw < -180.0f) dyaw += 360.0f;
                _gyroZ = dyaw / dt;
                _gyroX = 0.0f;
                _gyroY = 0.0f;
                _hasGyro = true;
            }
            // 无论 dt 是否超标都更新基线
            _prevYaw = _yaw;
            _prevYawMs = nowMs;
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
    // 帧格式：`*roll,pitch,yaw[,gx,gy,gz]`
    // 开启 sog1 后为 6 字段，否则 3 字段，两种都能解析。
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

    _roll  = r;
    _pitch = pi;
    _yaw   = y;

    // 尝试解析 EBIMU 原生陀螺仪字段（sog1 开启后才有）
    if (*endptr == ',') {
        p = endptr + 1;
        float gx = strtof(p, &endptr);
        if (endptr != p && *endptr == ',') {
            p = endptr + 1;
            float gy = strtof(p, &endptr);
            if (endptr != p && *endptr == ',') {
                p = endptr + 1;
                float gz = strtof(p, &endptr);
                if (endptr != p) {
                    _gyroX = gx;
                    _gyroY = gy;
                    _gyroZ = gz;
                    _hasGyro = true;
                    _hasNativeGyro = true;
                    return true;
                }
            }
        }
    }

    // 没有原生 gyro，gyroZ 由 update() 差分算
    _hasNativeGyro = false;
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
    if (_hasGyro) {
        g_dbg->print(F("  gyro="));
        g_dbg->print(_gyroX, 2); g_dbg->print(F(", "));
        g_dbg->print(_gyroY, 2); g_dbg->print(F(", "));
        g_dbg->print(_gyroZ, 2); g_dbg->println(F(" deg/s"));
    }
}
