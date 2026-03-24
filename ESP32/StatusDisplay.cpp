/**********************************************************************
 * StatusDisplay.cpp
 *
 * 这个文件实现 Minima 回传状态帧的接收和串口显示。
 *********************************************************************/

// 引入状态显示类声明。
#include "StatusDisplay.h"

// 实现构造函数。
StatusDisplay::StatusDisplay()
    // 默认开启详细输出模式。
    : _verboseMode(true),
      // 初始化最近一次心跳时间为 0。
      _lastHeartbeat(0),
      // 初始化接收索引为 0。
      _rxIndex(0) {}

// 实现 Minima 回传处理函数。
void StatusDisplay::processMinimaFeedback() {
    // 只要 Minima 串口里还有数据，就持续读取。
    while (UART_TO_MINIMA.available()) {
        // 读取一个字节。
        const uint8_t byte = static_cast<uint8_t>(UART_TO_MINIMA.read());

        // 如果当前期待帧头，而收到的不是帧头，就丢弃这个字节。
        if (_rxIndex == 0 && byte != FRAME_HEADER) {
            continue;
        }

        // 把当前字节写入接收缓冲区。
        _rxBuffer[_rxIndex++] = byte;
        // 如果一帧还没收满，就继续读后续字节。
        if (_rxIndex < FRAME_LENGTH) {
            continue;
        }

        // 检查帧尾是否正确。
        const bool validTail = _rxBuffer[7] == FRAME_TAIL;
        // 重新计算 CRC8。
        const uint8_t crc = calculateCRC8(&_rxBuffer[2], 4);
        // 比较 CRC 是否匹配。
        const bool validCrc = crc == _rxBuffer[6];
        // 一帧处理结束后，把接收索引重置为 0。
        _rxIndex = 0;

        // 如果帧尾或 CRC 任一错误，就丢弃整帧。
        if (!validTail || !validCrc) {
            continue;
        }

        // 根据命令字节分发不同的状态处理逻辑。
        switch (_rxBuffer[2]) {
            // 如果是运动状态帧。
            case STATUS_MOTION:
                // 处理运动状态字节。
                processMotionStatus(_rxBuffer[3]);
                break;
            // 如果是心跳帧。
            case STATUS_HEARTBEAT:
                // 更新心跳时间戳。
                processHeartbeat();
                break;
            // 如果是旧版传感器状态帧。
            case STATUS_SENSOR_DATA:
                // 当前架构下深度和超声波都由 ESP32 本地读取，所以这里忽略。
                break;
            // 其它未知命令统一忽略。
            default:
                break;
        }
    }
}

// 实现开启详细输出函数。
void StatusDisplay::enableVerbose() {
    // 打开详细输出标记。
    _verboseMode = true;
    // 打印提示信息。
    Serial.println(F("\nVerbose mode enabled\n"));
}

// 实现关闭详细输出函数。
void StatusDisplay::disableVerbose() {
    // 关闭详细输出标记。
    _verboseMode = false;
    // 打印提示信息。
    Serial.println(F("\nVerbose mode disabled\n"));
}

// 实现详细输出切换函数。
void StatusDisplay::toggleVerbose() {
    // 如果当前已经是详细模式。
    if (_verboseMode) {
        // 就关闭详细模式。
        disableVerbose();
    } else {
        // 否则打开详细模式。
        enableVerbose();
    }
}

// 实现读取详细模式开关状态的函数。
bool StatusDisplay::isVerboseEnabled() const {
    // 返回当前详细模式标记。
    return _verboseMode;
}

// 实现运动状态解析函数。
void StatusDisplay::processMotionStatus(uint8_t status) {
    // 如果当前不允许详细输出，就不打印状态细节。
    if (!_verboseMode) {
        return;
    }

    // 打印运动状态前缀。
    Serial.print(F("<- Motion: "));
    // 如果前进位被置位，就打印 FWD。
    if (status & 0x01) Serial.print(F("FWD "));
    // 如果转向位被置位，就打印 TURN。
    if (status & 0x02) Serial.print(F("TURN "));
    // 如果浮沉位被置位，就打印 BUOY。
    if (status & 0x04) Serial.print(F("BUOY "));
    // 如果状态字节为 0，就说明当前空闲。
    if (status == 0) Serial.print(F("IDLE"));
    // 打印行结束。
    Serial.println();
}

// 实现心跳处理函数。
void StatusDisplay::processHeartbeat() {
    // 把最近一次心跳时间更新为当前时间。
    _lastHeartbeat = millis();
}
