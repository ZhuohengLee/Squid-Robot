/**********************************************************************
 * SensorHub.h
 *
 * 杩欎釜鏂囦欢澹版槑 ESP32 渚х殑澶氫紶鎰熷櫒姹囨€绘帴鍙ｃ€? *********************************************************************/

// 闃叉澶存枃浠惰閲嶅鍖呭惈銆?#ifndef ESP32_SENSOR_HUB_H
// 瀹氫箟澶存枃浠朵繚鎶ゅ畯銆?#define ESP32_SENSOR_HUB_H

// 寮曞叆 Arduino 鍩虹绫诲瀷銆?#include <Arduino.h>
// 寮曞叆娣卞害浼犳劅鍣ㄧ鐞嗙被瀹氫箟銆?#include "DepthSensorManager.h"
// 寮曞叆瓒呭０娉㈢鐞嗙被瀹氫箟銆?#include "UltrasonicManager.h"

// 澹版槑浼犳劅鍣ㄦ眹鎬荤被銆?class SensorHub {
// 中文逐行说明：下面这一行保留原始代码 -> public:
public:
    // 澹版槑鏋勯€犲嚱鏁般€?    SensorHub();

    // 澹版槑娉ㄥ叆娣卞害浼犳劅鍣ㄧ鐞嗗櫒鐨勫嚱鏁般€?    void setDepthSensorManager(DepthSensorManager* manager);
    // 澹版槑娉ㄥ叆瓒呭０娉㈢鐞嗗櫒鐨勫嚱鏁般€?    void setUltrasonicManager(UltrasonicManager* manager);

    // 澹版槑鎵ц娣卞害闆剁偣鏍″噯鐨勫嚱鏁般€?    void calibrateDepthZero();
    // 澹版槑瀹屾暣鎵撳嵃鎵€鏈変紶鎰熷櫒鐘舵€佺殑鍑芥暟銆?    void displayAll();
    // 澹版槑绱у噾鎵撳嵃鎵€鏈変紶鎰熷櫒鐘舵€佺殑鍑芥暟銆?    void displayCompact();
    // 澹版槑鎵ц浼犳劅鍣ㄥ仴搴锋鏌ョ殑鍑芥暟銆?    bool isHealthy() const;
    // 澹版槑杩斿洖褰撳墠鏈夋晥浼犳劅鍣ㄦ暟閲忕殑鍑芥暟銆?    uint8_t getSensorCount() const;

// 中文逐行说明：下面这一行保留原始代码 -> private:
private:
    // 淇濆瓨娣卞害浼犳劅鍣ㄧ鐞嗗櫒鎸囬拡銆?    DepthSensorManager* _depthMgr;
    // 淇濆瓨瓒呭０娉㈢鐞嗗櫒鎸囬拡銆?    UltrasonicManager* _ultrasonicMgr;
    // 淇濆瓨涓婁竴娆″畬鏁存墦鍗扮殑鏃堕棿鎴炽€?    uint32_t _lastDisplay;
// 中文逐行说明：下面这一行保留原始代码 -> };
};

// 缁撴潫澶存枃浠朵繚鎶ゃ€?#endif // ESP32_SENSOR_HUB_H
