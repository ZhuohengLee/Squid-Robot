/**********************************************************************
 * UltrasonicManager.cpp
 *
 * 这个文件实现三路超声波传感器的轮询、触发和数据解析。
 *********************************************************************/

// 引入超声波管理类声明。
#include "UltrasonicManager.h"

// 声明匿名命名空间，用于保存当前文件内部常量。
namespace {
// 定义超声波轮询周期，单位毫秒。
constexpr uint32_t SCAN_INTERVAL_MS = 100;
// 定义超声波数据超时时间，单位毫秒。
constexpr uint32_t DATA_TIMEOUT_MS = 2000;
// 定义逻辑方向到 UART 通道的映射表。
const uint8_t UART_CHANNELS[NUM_ULTRASONIC] = {
    ULTRASONIC_FRONT_UART,
    ULTRASONIC_LEFT_UART,
    ULTRASONIC_RIGHT_UART,
};
}

// 实现构造函数。
UltrasonicManager::UltrasonicManager(CH9434A* ch9434)
    // 保存 CH9434A 驱动对象指针。
    : _ch9434(ch9434),
      // 初始化最近一次扫描时间为 0。
      _lastScanTime(0) {
    // 构造完成后先把全部状态清空。
    resetAll();
}

// 实现初始化函数。
bool UltrasonicManager::begin() {
    // 打印初始化提示。
    Serial.println(F("Initializing ultrasonic sensors..."));

    // 逐路配置三路超声波对应的 UART。
    for (uint8_t sensor = 0; sensor < NUM_ULTRASONIC; ++sensor) {
        // 根据逻辑编号获取 UART 通道号。
        const uint8_t uart = getUartChannel(sensor);
        // 打印 UART 配置提示前缀。
        Serial.print(F("  Configuring UART"));
        // 打印 UART 通道号。
        Serial.print(uart);
        // 打印提示后缀。
        Serial.print(F("... "));

        // 配置 CH9434A 对应 UART 的波特率和串口格式。
        if (!_ch9434->config(uart, ULTRASONIC_BAUDRATE, CH9434A_LCR_8N1)) {
            // 如果失败，就打印 FAILED。
            Serial.println(F("FAILED"));
            // 返回初始化失败。
            return false;
        }

        // 清空该 UART 上的旧缓存。
        _ch9434->flush(uart);
        // 打印初始化成功。
        Serial.println(F("OK"));
    }

    // 全部 UART 初始化成功后返回 true。
    return true;
}

// 实现更新函数。
void UltrasonicManager::update() {
    // 读取当前时间。
    const uint32_t now = millis();
    // 如果还没到下一次扫描时间，就直接返回。
    if (now - _lastScanTime < SCAN_INTERVAL_MS) {
        return;
    }

    // 更新最近一次扫描时间。
    _lastScanTime = now;

    // 逐路轮询三路超声波。
    for (uint8_t sensor = 0; sensor < NUM_ULTRASONIC; ++sensor) {
        // 尝试读取当前这一路的测量值。
        readSensor(sensor);
        // 如果这一路数据超时太久没有更新。
        if (now - _sensors[sensor].lastUpdate > DATA_TIMEOUT_MS) {
            // 就把这一路标记为无效。
            _sensors[sensor].valid = false;
        }
    }
}

// 实现读取单路距离值的函数。
uint16_t UltrasonicManager::getDistance(uint8_t sensor) const {
    // 如果编号合法，就返回对应距离，否则返回 0。
    return sensor < NUM_ULTRASONIC ? _sensors[sensor].distance_mm : 0;
}

// 实现读取单路有效性的函数。
bool UltrasonicManager::isValid(uint8_t sensor) const {
    // 只有编号合法且该路 valid 为 true 时才返回 true。
    return sensor < NUM_ULTRASONIC && _sensors[sensor].valid;
}

// 实现读取单路最后更新时间的函数。
uint32_t UltrasonicManager::getLastUpdate(uint8_t sensor) const {
    // 如果编号合法，就返回对应时间，否则返回 0。
    return sensor < NUM_ULTRASONIC ? _sensors[sensor].lastUpdate : 0;
}

// 实现批量读取全部距离值的函数。
void UltrasonicManager::getAllDistances(uint16_t* distances) const {
    // 逐路遍历全部超声波。
    for (uint8_t sensor = 0; sensor < NUM_ULTRASONIC; ++sensor) {
        // 如果当前这一路数据有效，就写入真实距离，否则写入 0。
        distances[sensor] = _sensors[sensor].valid ? _sensors[sensor].distance_mm : 0;
    }
}

// 实现重置单路状态的函数。
void UltrasonicManager::reset(uint8_t sensor) {
    // 如果编号越界，就直接返回。
    if (sensor >= NUM_ULTRASONIC) {
        return;
    }

    // 把距离值清零。
    _sensors[sensor].distance_mm = 0;
    // 把有效标记清零。
    _sensors[sensor].valid = false;
    // 把最后更新时间清零。
    _sensors[sensor].lastUpdate = 0;
    // 把错误计数清零。
    _sensors[sensor].errorCount = 0;
    // 把最近触发时间清零。
    _lastTrigger[sensor] = 0;
    // 清空该路 UART 的接收缓存。
    _ch9434->flush(getUartChannel(sensor));
}

// 实现重置全部状态的函数。
void UltrasonicManager::resetAll() {
    // 逐路遍历全部超声波。
    for (uint8_t sensor = 0; sensor < NUM_ULTRASONIC; ++sensor) {
        // 把距离值清零。
        _sensors[sensor].distance_mm = 0;
        // 把有效标记清零。
        _sensors[sensor].valid = false;
        // 把最后更新时间清零。
        _sensors[sensor].lastUpdate = 0;
        // 把错误计数清零。
        _sensors[sensor].errorCount = 0;
        // 把最近触发时间清零。
        _lastTrigger[sensor] = 0;
    }
}

// 实现读取单路超声波的函数。
bool UltrasonicManager::readSensor(uint8_t sensor) {
    // 如果编号越界，就直接返回 false。
    if (sensor >= NUM_ULTRASONIC) {
        return false;
    }

    // 获取当前逻辑方向对应的 UART 通道号。
    const uint8_t uart = getUartChannel(sensor);
    // 读取当前时间。
    const uint32_t now = millis();

    // 如果距离上次触发已经超过 50 毫秒。
    if (now - _lastTrigger[sensor] > 50) {
        // 先触发一次新的超声波测量。
        triggerMeasurement(sensor);
        // 记录本次触发时间。
        _lastTrigger[sensor] = now;
        // 等待模块准备返回数据。
        delay(20);
    }

    // 如果当前 UART 没有任何可读数据。
    if (_ch9434->available(uart) == 0) {
        // 给该路错误计数加一。
        if (++_sensors[sensor].errorCount > 3) {
            // 连续错误太多时，把该路标记为无效。
            _sensors[sensor].valid = false;
        }
        // 返回读取失败。
        return false;
    }

    // 创建读帧缓冲区。
    uint8_t buffer[10];
    // 初始化已读取字节数为 0。
    uint8_t bytesRead = 0;
    // 记录读帧起始时间。
    const uint32_t start = millis();

    // 在缓冲区未满且未超时时持续读取。
    while (bytesRead < sizeof(buffer) && (millis() - start) < ULTRASONIC_TIMEOUT) {
        // 只有 UART 确实有数据时才读取。
        if (_ch9434->available(uart)) {
            // 读取一个字节写入缓冲区，并递增读取计数。
            buffer[bytesRead++] = _ch9434->read(uart);
        }
        // 给串口数据到达留一点时间。
        delayMicroseconds(100);
    }

    // 如果读取到的字节数少于最小帧长 4。
    if (bytesRead < 4) {
        // 错误计数加一。
        ++_sensors[sensor].errorCount;
        // 返回读取失败。
        return false;
    }

    // 初始化帧头位置为未找到。
    int frameStart = -1;
    // 在已读取数据里查找 0xFF 帧头。
    for (uint8_t i = 0; i <= bytesRead - 4; ++i) {
        // 如果当前位置是帧头。
        if (buffer[i] == 0xFF) {
            // 记录帧起始位置。
            frameStart = i;
            // 找到第一个帧头后立即退出。
            break;
        }
    }

    // 如果没找到帧头，或者帧校验失败。
    if (frameStart < 0 || !validateFrame(&buffer[frameStart])) {
        // 错误计数加一。
        ++_sensors[sensor].errorCount;
        // 返回读取失败。
        return false;
    }

    // 解析该帧中的距离值。
    const uint16_t distance = parseDistance(&buffer[frameStart]);
    // 如果距离超出当前允许范围。
    if (distance < 50 || distance > 3000) {
        // 错误计数加一。
        ++_sensors[sensor].errorCount;
        // 返回读取失败。
        return false;
    }

    // 保存本次有效距离值。
    _sensors[sensor].distance_mm = distance;
    // 标记该路当前有效。
    _sensors[sensor].valid = true;
    // 更新最近一次有效时间。
    _sensors[sensor].lastUpdate = now;
    // 清零连续错误计数。
    _sensors[sensor].errorCount = 0;
    // 返回读取成功。
    return true;
}

// 实现触发超声波测量的函数。
void UltrasonicManager::triggerMeasurement(uint8_t sensor) {
    // 根据逻辑方向取得 UART 通道号。
    const uint8_t uart = getUartChannel(sensor);
    // 先清空该 UART 上残留的旧数据。
    _ch9434->flush(uart);
    // 向模块发送一个 0x00 作为触发字节。
    _ch9434->write(uart, 0x00);
}

// 实现超声波帧校验函数。
bool UltrasonicManager::validateFrame(const uint8_t* buffer) const {
    // 要求帧头必须是 0xFF。
    return buffer[0] == 0xFF &&
           // 并且前三个字节之和的低 8 位必须等于第 4 个字节。
           static_cast<uint8_t>(buffer[0] + buffer[1] + buffer[2]) == buffer[3];
}

// 实现超声波距离解析函数。
uint16_t UltrasonicManager::parseDistance(const uint8_t* buffer) const {
    // 把高低字节拼成毫米单位距离值。
    return static_cast<uint16_t>(buffer[1] << 8) | buffer[2];
}

// 实现 UART 通道映射函数。
uint8_t UltrasonicManager::getUartChannel(uint8_t sensor) const {
    // 直接从映射表返回对应 UART 编号。
    return UART_CHANNELS[sensor];
}
