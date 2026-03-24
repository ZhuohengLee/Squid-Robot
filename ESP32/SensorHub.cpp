/**********************************************************************
 * SensorHub.cpp
 *
 * 这个文件实现 ESP32 侧的多传感器汇总显示与健康检查。
 *********************************************************************/

// 引入传感器汇总类声明。
#include "SensorHub.h"

// 声明匿名命名空间，用于保存当前文件内部常量。
namespace {
// 定义完整显示输出的最小刷新周期。
constexpr uint32_t DISPLAY_INTERVAL_MS = 1000;
// 定义三路超声波对应的人类可读名称。
const char* SENSOR_NAMES[NUM_ULTRASONIC] = {"Front", "Left ", "Right"};
}

// 实现构造函数。
SensorHub::SensorHub()
    // 初始化深度管理器指针为空。
    : _depthMgr(nullptr),
      // 初始化超声波管理器指针为空。
      _ultrasonicMgr(nullptr),
      // 初始化上次显示时间为 0。
      _lastDisplay(0) {}

// 实现设置深度管理器的函数。
void SensorHub::setDepthSensorManager(DepthSensorManager* manager) {
    // 保存深度管理器指针。
    _depthMgr = manager;
}

// 实现设置超声波管理器的函数。
void SensorHub::setUltrasonicManager(UltrasonicManager* manager) {
    // 保存超声波管理器指针。
    _ultrasonicMgr = manager;
}

// 实现深度零点校准函数。
void SensorHub::calibrateDepthZero() {
    // 只有在深度管理器已绑定时才执行校准。
    if (_depthMgr) {
        // 调用深度管理器的零点校准接口。
        _depthMgr->calibrateZero();
    }
}

// 实现完整打印全部传感器状态的函数。
void SensorHub::displayAll() {
    // 读取当前时间。
    const uint32_t now = millis();
    // 如果距离上次显示太近，就直接返回。
    if (now - _lastDisplay < DISPLAY_INTERVAL_MS) {
        return;
    }
    // 更新最近一次显示时间。
    _lastDisplay = now;

    // 打印显示头部。
    Serial.println(F("\n================ ALL SENSORS ================"));

    // 打印深度标签。
    Serial.print(F("Depth: "));
    // 如果深度管理器存在且当前数据有效。
    if (_depthMgr && _depthMgr->isValid()) {
        // 打印深度值。
        Serial.print(_depthMgr->getDepthCm(), 2);
        // 打印温度标签。
        Serial.print(F(" cm | Temp: "));
        // 打印温度值。
        Serial.print(_depthMgr->getTemperatureC(), 1);
        // 打印温度单位。
        Serial.println(F(" C"));
    } else {
        // 如果深度数据无效，就打印离线状态。
        Serial.println(F("offline"));
    }

    // 只有超声波管理器存在时才打印超声波状态。
    if (_ultrasonicMgr) {
        // 逐路遍历全部超声波。
        for (uint8_t sensor = 0; sensor < NUM_ULTRASONIC; ++sensor) {
            // 打印超声波前缀。
            Serial.print(F("Ultrasonic "));
            // 打印方向名称。
            Serial.print(SENSOR_NAMES[sensor]);
            // 打印冒号和空格。
            Serial.print(F(": "));

            // 如果该路超声波数据有效。
            if (_ultrasonicMgr->isValid(sensor)) {
                // 打印厘米单位距离值。
                Serial.print(_ultrasonicMgr->getDistance(sensor) / 10.0f, 1);
                // 打印单位。
                Serial.println(F(" cm"));
            } else {
                // 否则打印离线状态。
                Serial.println(F("offline"));
            }
        }
    }

    // 打印显示尾部。
    Serial.println(F("=============================================\n"));
}

// 实现紧凑显示函数。
void SensorHub::displayCompact() {
    // 打印深度字段前缀。
    Serial.print(F("Sensors: depth="));
    // 如果深度管理器存在且数据有效。
    if (_depthMgr && _depthMgr->isValid()) {
        // 打印一位小数的深度值。
        Serial.print(_depthMgr->getDepthCm(), 1);
        // 打印深度单位。
        Serial.print(F("cm"));
    } else {
        // 如果无效，就打印占位符。
        Serial.print(F("--"));
    }

    // 如果超声波管理器存在。
    if (_ultrasonicMgr) {
        // 打印超声波字段前缀。
        Serial.print(F(" | us="));
        // 逐路遍历全部超声波。
        for (uint8_t sensor = 0; sensor < NUM_ULTRASONIC; ++sensor) {
            // 如果该路超声波数据有效。
            if (_ultrasonicMgr->isValid(sensor)) {
                // 打印无小数厘米值。
                Serial.print(_ultrasonicMgr->getDistance(sensor) / 10.0f, 0);
            } else {
                // 如果无效，就打印占位符。
                Serial.print(F("--"));
            }
            // 如果还没到最后一路，就打印空格分隔。
            if (sensor + 1 < NUM_ULTRASONIC) {
                // 打印单个空格字符。
                Serial.print(' ');
            }
        }
    }

    // 打印行结束。
    Serial.println();
}

// 实现健康检查函数。
bool SensorHub::isHealthy() const {
    // 判断深度传感器是否在线。
    const bool depthOk = _depthMgr && _depthMgr->isValid();
    // 初始化有效超声波数量计数器。
    uint8_t ultrasonicOk = 0;

    // 如果超声波管理器存在。
    if (_ultrasonicMgr) {
        // 逐路检查全部超声波。
        for (uint8_t sensor = 0; sensor < NUM_ULTRASONIC; ++sensor) {
            // 如果这一 路超声波有效。
            if (_ultrasonicMgr->isValid(sensor)) {
                // 把有效计数加一。
                ++ultrasonicOk;
            }
        }
    }

    // 只有深度在线且至少 2 路超声波有效，才认为系统健康。
    return depthOk && ultrasonicOk >= 2;
}

// 实现有效传感器数量统计函数。
uint8_t SensorHub::getSensorCount() const {
    // 初始化有效传感器数量为 0。
    uint8_t count = 0;

    // 如果深度管理器存在且数据有效。
    if (_depthMgr && _depthMgr->isValid()) {
        // 把有效数量加一。
        ++count;
    }

    // 如果超声波管理器存在。
    if (_ultrasonicMgr) {
        // 逐路检查全部超声波。
        for (uint8_t sensor = 0; sensor < NUM_ULTRASONIC; ++sensor) {
            // 如果这一 路超声波有效。
            if (_ultrasonicMgr->isValid(sensor)) {
                // 把有效数量加一。
                ++count;
            }
        }
    }

    // 返回最终有效传感器数量。
    return count;
}
