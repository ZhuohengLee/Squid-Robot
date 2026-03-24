/**********************************************************************
 * DepthSensorManager.cpp
 *
 * 这个文件实现通过 CH9434A 串口读取深度传感器的逻辑。
 *********************************************************************/

// 引入深度传感器管理类声明。
#include "DepthSensorManager.h"
// 引入协议常量定义。
#include "Protocol.h"

// 实现构造函数。
DepthSensorManager::DepthSensorManager(CH9434A* ch9434, uint8_t uartNum)
    // 保存桥接芯片对象指针。
    : _ch9434(ch9434),
      // 保存深度传感器所在 UART 号。
      _uartNum(uartNum),
      // 初始化深度值为 0。
      _depthCm(0.0f),
      // 初始化温度值为 0。
      _temperatureC(0.0f),
      // 初始化零点偏移为 0。
      _depthOffsetCm(0.0f),
      // 初始化数据有效标记为 false。
      _valid(false),
      // 初始化最后更新时间戳为 0。
      _lastUpdate(0),
      // 初始化文本缓冲区长度为 0。
      _lineLength(0),
      // 初始化二进制缓冲区长度为 0。
      _binaryLength(0) {}

// 实现初始化函数。
bool DepthSensorManager::begin() {
    // 先配置深度传感器所在 UART 的波特率和格式。
    if (!_ch9434->config(_uartNum, DEPTH_UART_BAUDRATE, CH9434A_LCR_8N1)) {
        // 如果配置失败，就返回 false。
        return false;
    }
    // 清空该 UART 上可能残留的旧数据。
    _ch9434->flush(_uartNum);
    // 返回初始化成功。
    return true;
}

// 实现轮询更新函数。
void DepthSensorManager::update() {
    // 只要串口还有数据，就持续读取。
    while (_ch9434->available(_uartNum)) {
        // 读取一个字节。
        const uint8_t byte = _ch9434->read(_uartNum);

        // 优先尝试把这个字节交给二进制协议解析器。
        if (parseBinaryByte(byte)) {
            // 如果二进制解析器已经消费了这个字节，就继续下一个字节。
            continue;
        }

        // 如果收到的是换行符或回车符，说明一行文本结束。
        if (byte == '\r' || byte == '\n') {
            // 只有文本缓冲区非空时才尝试解析。
            if (_lineLength > 0) {
                // 在当前文本末尾补上字符串结束符。
                _lineBuffer[_lineLength] = '\0';
                // 把整行文本交给文本解析器。
                parseAsciiLine(_lineBuffer);
                // 解析完成后清空文本长度。
                _lineLength = 0;
            }
            // 继续处理下一个字节。
            continue;
        }

        // 如果文本缓冲区已经放不下新字符，就直接丢弃这一行。
        if (_lineLength + 1 >= sizeof(_lineBuffer)) {
            // 把文本缓冲区长度重置为 0。
            _lineLength = 0;
            // 继续处理下一个字节。
            continue;
        }

        // 只有可打印字符才会被加入文本缓冲区。
        if (isPrintable(byte)) {
            // 把当前字节转换成 char 后写入缓冲区。
            _lineBuffer[_lineLength++] = static_cast<char>(byte);
        }
    }

    // 如果数据长时间未更新，就把有效标记清掉。
    if (_valid && millis() - _lastUpdate > 2000) {
        // 标记当前深度数据无效。
        _valid = false;
    }
}

// 实现零点校准函数。
void DepthSensorManager::calibrateZero() {
    // 把当前深度值累加进偏移量，之后输出会以此为零点。
    _depthOffsetCm = _depthCm + _depthOffsetCm;
}

// 实现读取“是否有效”的函数。
bool DepthSensorManager::isValid() const {
    // 返回当前有效标记。
    return _valid;
}

// 实现读取深度值的函数。
float DepthSensorManager::getDepthCm() const {
    // 返回当前深度值。
    return _depthCm;
}

// 实现读取温度值的函数。
float DepthSensorManager::getTemperatureC() const {
    // 返回当前温度值。
    return _temperatureC;
}

// 实现读取最后更新时间的函数。
uint32_t DepthSensorManager::getLastUpdate() const {
    // 返回最后更新时间戳。
    return _lastUpdate;
}

// 实现文本深度行解析函数。
bool DepthSensorManager::parseAsciiLine(const char* line) {
    // 创建本地可修改缓冲区。
    char buffer[64];
    // 复制输入文本到本地缓冲区。
    strncpy(buffer, line, sizeof(buffer) - 1);
    // 强制补上字符串结束符。
    buffer[sizeof(buffer) - 1] = '\0';

    // 创建第一个数字的结束指针。
    char* endPtr = nullptr;
    // 解析第一个浮点数。
    const float first = strtof(buffer, &endPtr);
    // 如果没有解析出任何数字，就返回失败。
    if (endPtr == buffer) {
        // 返回解析失败。
        return false;
    }

    // 跳过第一个数字后的分隔符。
    while (*endPtr == ' ' || *endPtr == ',' || *endPtr == '\t' || *endPtr == ':') {
        // 移动到下一个字符。
        ++endPtr;
    }

    // 如果后面没有第二个数字，就只提交深度值。
    if (*endPtr == '\0') {
        // 提交深度值，并保留旧温度。
        commitDepth(first, _temperatureC, false);
        // 返回解析成功。
        return true;
    }

    // 创建第二个数字的结束指针。
    char* secondEndPtr = nullptr;
    // 解析第二个浮点数。
    const float second = strtof(endPtr, &secondEndPtr);
    // 如果第二个数字解析失败，就仍然只提交深度值。
    if (secondEndPtr == endPtr) {
        // 提交深度值，并保留旧温度。
        commitDepth(first, _temperatureC, false);
        // 返回解析成功。
        return true;
    }

    // 提交深度和温度两个值。
    commitDepth(first, second, true);
    // 返回解析成功。
    return true;
}

// 实现二进制深度帧逐字节解析函数。
bool DepthSensorManager::parseBinaryByte(uint8_t byte) {
    // 如果当前还没有进入二进制帧状态。
    if (_binaryLength == 0) {
        // 只有遇到帧头 0xFF 才开始收帧。
        if (byte != 0xFF) {
            // 不是二进制帧头，返回 false。
            return false;
        }
        // 把帧头写入二进制缓冲区。
        _binaryBuffer[_binaryLength++] = byte;
        // 返回 true，表示这个字节已被消费。
        return true;
    }

    // 把当前字节追加到二进制缓冲区。
    _binaryBuffer[_binaryLength++] = byte;
    // 如果二进制帧还没收满 4 字节，就继续等待后续字节。
    if (_binaryLength < sizeof(_binaryBuffer)) {
        // 返回 true，表示当前字节已被消费。
        return true;
    }

    // 计算前三个字节的校验和。
    const uint8_t checksum = static_cast<uint8_t>(_binaryBuffer[0] + _binaryBuffer[1] + _binaryBuffer[2]);
    // 判断校验值是否与第 4 个字节一致。
    const bool valid = checksum == _binaryBuffer[3];
    // 无论成功失败，都把二进制长度重置为 0。
    _binaryLength = 0;

    // 如果校验失败，就返回 false。
    if (!valid) {
        // 返回校验失败。
        return false;
    }

    // 从高低字节拼出毫米单位的深度值。
    const uint16_t depthMm = static_cast<uint16_t>(_binaryBuffer[1] << 8) | _binaryBuffer[2];
    // 把毫米值换算成厘米并提交。
    commitDepth(depthMm / 10.0f, _temperatureC, false);
    // 返回 true，表示二进制帧解析成功。
    return true;
}

// 实现提交有效深度结果的函数。
void DepthSensorManager::commitDepth(float depthCm, float temperatureC, bool hasTemperature) {
    // 写入经零点偏移修正后的深度值。
    _depthCm = depthCm - _depthOffsetCm;
    // 如果这一帧带温度，就更新当前温度值。
    if (hasTemperature) {
        // 保存温度值。
        _temperatureC = temperatureC;
    }
    // 更新时间戳为当前毫秒数。
    _lastUpdate = millis();
    // 标记当前数据有效。
    _valid = true;
}
