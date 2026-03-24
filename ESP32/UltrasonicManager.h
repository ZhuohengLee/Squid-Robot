/**********************************************************************
 * UltrasonicManager.h
 *
 * 杩欎釜鏂囦欢澹版槑涓夎矾瓒呭０娉紶鎰熷櫒绠＄悊鍣ㄣ€? * 绠＄悊鍣ㄤ細鍏堣鍙栧師濮嬪洖娉㈣窛绂伙紝鍐嶅仛鍗″皵鏇兼护娉紝鏈€鍚庢妸婊ゆ尝缁撴灉鎻愪緵缁欒嚜鍔ㄥ璺€? *********************************************************************/

// 中文逐行说明：下面这一行保留原始代码 -> #ifndef ESP32_ULTRASONIC_MANAGER_H
#ifndef ESP32_ULTRASONIC_MANAGER_H
// 中文逐行说明：下面这一行保留原始代码 -> #define ESP32_ULTRASONIC_MANAGER_H
#define ESP32_ULTRASONIC_MANAGER_H

// 中文逐行说明：下面这一行保留原始代码 -> #include <Arduino.h>
#include <Arduino.h>
// 中文逐行说明：下面这一行保留原始代码 -> #include "CH9434A.h"
#include "CH9434A.h"
// 中文逐行说明：下面这一行保留原始代码 -> #include "KalmanFilter.h"
#include "KalmanFilter.h"
// 中文逐行说明：下面这一行保留原始代码 -> #include "Protocol.h"
#include "Protocol.h"

// 中文逐行说明：下面这一行保留原始代码 -> struct UltrasonicData {
struct UltrasonicData {
    // 中文逐行说明：下面这一行保留原始代码 -> uint16_t rawDistanceMm;
    uint16_t rawDistanceMm;
    // 中文逐行说明：下面这一行保留原始代码 -> uint16_t filteredDistanceMm;
    uint16_t filteredDistanceMm;
    // 中文逐行说明：下面这一行保留原始代码 -> bool valid;
    bool valid;
    // 中文逐行说明：下面这一行保留原始代码 -> uint32_t lastUpdate;
    uint32_t lastUpdate;
    // 中文逐行说明：下面这一行保留原始代码 -> uint8_t errorCount;
    uint8_t errorCount;
// 中文逐行说明：下面这一行保留原始代码 -> };
};

// 中文逐行说明：下面这一行保留原始代码 -> class UltrasonicManager {
class UltrasonicManager {
// 中文逐行说明：下面这一行保留原始代码 -> public:
public:
    // 中文逐行说明：下面这一行保留原始代码 -> explicit UltrasonicManager(CH9434A* ch9434);
    explicit UltrasonicManager(CH9434A* ch9434);

    // 中文逐行说明：下面这一行保留原始代码 -> bool begin();
    bool begin();
    // 中文逐行说明：下面这一行保留原始代码 -> void update();
    void update();

    // 中文逐行说明：下面这一行保留原始代码 -> uint16_t getDistance(uint8_t sensor) const;
    uint16_t getDistance(uint8_t sensor) const;
    // 中文逐行说明：下面这一行保留原始代码 -> uint16_t getRawDistance(uint8_t sensor) const;
    uint16_t getRawDistance(uint8_t sensor) const;
    // 中文逐行说明：下面这一行保留原始代码 -> bool isValid(uint8_t sensor) const;
    bool isValid(uint8_t sensor) const;
    // 中文逐行说明：下面这一行保留原始代码 -> uint32_t getLastUpdate(uint8_t sensor) const;
    uint32_t getLastUpdate(uint8_t sensor) const;
    // 中文逐行说明：下面这一行保留原始代码 -> void getAllDistances(uint16_t* distances) const;
    void getAllDistances(uint16_t* distances) const;

    // 中文逐行说明：下面这一行保留原始代码 -> void reset(uint8_t sensor);
    void reset(uint8_t sensor);
    // 中文逐行说明：下面这一行保留原始代码 -> void resetAll();
    void resetAll();

// 中文逐行说明：下面这一行保留原始代码 -> private:
private:
    // 中文逐行说明：下面这一行保留原始代码 -> CH9434A* _ch9434;
    CH9434A* _ch9434;
    // 中文逐行说明：下面这一行保留原始代码 -> UltrasonicData _sensors[NUM_ULTRASONIC];
    UltrasonicData _sensors[NUM_ULTRASONIC];
    // 中文逐行说明：下面这一行保留原始代码 -> KalmanFilter _filters[NUM_ULTRASONIC];
    KalmanFilter _filters[NUM_ULTRASONIC];
    // 中文逐行说明：下面这一行保留原始代码 -> bool _filterInitialized[NUM_ULTRASONIC];
    bool _filterInitialized[NUM_ULTRASONIC];
    // 中文逐行说明：下面这一行保留原始代码 -> uint32_t _lastTrigger[NUM_ULTRASONIC];
    uint32_t _lastTrigger[NUM_ULTRASONIC];
    // 中文逐行说明：下面这一行保留原始代码 -> uint32_t _lastFilterUpdate[NUM_ULTRASONIC];
    uint32_t _lastFilterUpdate[NUM_ULTRASONIC];
    // 中文逐行说明：下面这一行保留原始代码 -> uint32_t _lastScanTime;
    uint32_t _lastScanTime;

    // 中文逐行说明：下面这一行保留原始代码 -> bool readSensor(uint8_t sensor);
    bool readSensor(uint8_t sensor);
    // 中文逐行说明：下面这一行保留原始代码 -> void triggerMeasurement(uint8_t sensor);
    void triggerMeasurement(uint8_t sensor);
    // 中文逐行说明：下面这一行保留原始代码 -> bool validateFrame(const uint8_t* buffer) const;
    bool validateFrame(const uint8_t* buffer) const;
    // 中文逐行说明：下面这一行保留原始代码 -> uint16_t parseDistance(const uint8_t* buffer) const;
    uint16_t parseDistance(const uint8_t* buffer) const;
    // 中文逐行说明：下面这一行保留原始代码 -> uint8_t getUartChannel(uint8_t sensor) const;
    uint8_t getUartChannel(uint8_t sensor) const;
// 中文逐行说明：下面这一行保留原始代码 -> };
};

// 中文逐行说明：下面这一行保留原始代码 -> #endif // ESP32_ULTRASONIC_MANAGER_H
#endif // ESP32_ULTRASONIC_MANAGER_H
