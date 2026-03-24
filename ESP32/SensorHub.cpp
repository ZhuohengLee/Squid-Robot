/**********************************************************************
 * SensorHub.cpp
 *
 * 杩欎釜鏂囦欢瀹炵幇 ESP32 渚х殑澶氫紶鎰熷櫒姹囨€绘樉绀轰笌鍋ュ悍妫€鏌ャ€? *********************************************************************/

// 寮曞叆浼犳劅鍣ㄦ眹鎬荤被澹版槑銆?#include "SensorHub.h"

// 澹版槑鍖垮悕鍛藉悕绌洪棿锛岀敤浜庝繚瀛樺綋鍓嶆枃浠跺唴閮ㄥ父閲忋€?namespace {
// 瀹氫箟瀹屾暣鏄剧ず杈撳嚭鐨勬渶灏忓埛鏂板懆鏈熴€?constexpr uint32_t DISPLAY_INTERVAL_MS = 1000;
// 瀹氫箟涓夎矾瓒呭０娉㈠搴旂殑浜虹被鍙鍚嶇О銆?const char* SENSOR_NAMES[NUM_ULTRASONIC] = {"Front", "Left ", "Right"};
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 瀹炵幇鏋勯€犲嚱鏁般€?SensorHub::SensorHub()
    // 鍒濆鍖栨繁搴︾鐞嗗櫒鎸囬拡涓虹┖銆?    : _depthMgr(nullptr),
      // 鍒濆鍖栬秴澹版尝绠＄悊鍣ㄦ寚閽堜负绌恒€?      _ultrasonicMgr(nullptr),
      // 鍒濆鍖栦笂娆℃樉绀烘椂闂翠负 0銆?      _lastDisplay(0) {}

// 瀹炵幇璁剧疆娣卞害绠＄悊鍣ㄧ殑鍑芥暟銆?void SensorHub::setDepthSensorManager(DepthSensorManager* manager) {
    // 淇濆瓨娣卞害绠＄悊鍣ㄦ寚閽堛€?    _depthMgr = manager;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 瀹炵幇璁剧疆瓒呭０娉㈢鐞嗗櫒鐨勫嚱鏁般€?void SensorHub::setUltrasonicManager(UltrasonicManager* manager) {
    // 淇濆瓨瓒呭０娉㈢鐞嗗櫒鎸囬拡銆?    _ultrasonicMgr = manager;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 瀹炵幇娣卞害闆剁偣鏍″噯鍑芥暟銆?void SensorHub::calibrateDepthZero() {
    // 鍙湁鍦ㄦ繁搴︾鐞嗗櫒宸茬粦瀹氭椂鎵嶆墽琛屾牎鍑嗐€?    if (_depthMgr) {
        // 璋冪敤娣卞害绠＄悊鍣ㄧ殑闆剁偣鏍″噯鎺ュ彛銆?        _depthMgr->calibrateZero();
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 瀹炵幇瀹屾暣鎵撳嵃鍏ㄩ儴浼犳劅鍣ㄧ姸鎬佺殑鍑芥暟銆?void SensorHub::displayAll() {
    // 璇诲彇褰撳墠鏃堕棿銆?    const uint32_t now = millis();
    // 濡傛灉璺濈涓婃鏄剧ず澶繎锛屽氨鐩存帴杩斿洖銆?    if (now - _lastDisplay < DISPLAY_INTERVAL_MS) {
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
    // 鏇存柊鏈€杩戜竴娆℃樉绀烘椂闂淬€?    _lastDisplay = now;

    // 鎵撳嵃鏄剧ず澶撮儴銆?    Serial.println(F("\n================ ALL SENSORS ================"));

    // 鎵撳嵃娣卞害鏍囩銆?    Serial.print(F("Depth: "));
    // 濡傛灉娣卞害绠＄悊鍣ㄥ瓨鍦ㄤ笖褰撳墠鏁版嵁鏈夋晥銆?    if (_depthMgr && _depthMgr->isValid()) {
        // 鎵撳嵃娣卞害鍊笺€?        Serial.print(_depthMgr->getDepthCm(), 2);
        // 鎵撳嵃娓╁害鏍囩銆?        Serial.print(F(" cm | Temp: "));
        // 鎵撳嵃娓╁害鍊笺€?        Serial.print(_depthMgr->getTemperatureC(), 1);
        // 鎵撳嵃娓╁害鍗曚綅銆?        Serial.println(F(" C"));
    // 中文逐行说明：下面这一行保留原始代码 -> } else {
    } else {
        // 濡傛灉娣卞害鏁版嵁鏃犳晥锛屽氨鎵撳嵃绂荤嚎鐘舵€併€?        Serial.println(F("offline"));
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 鍙湁瓒呭０娉㈢鐞嗗櫒瀛樺湪鏃舵墠鎵撳嵃瓒呭０娉㈢姸鎬併€?    if (_ultrasonicMgr) {
        // 閫愯矾閬嶅巻鍏ㄩ儴瓒呭０娉€?        for (uint8_t sensor = 0; sensor < NUM_ULTRASONIC; ++sensor) {
            // 鎵撳嵃瓒呭０娉㈠墠缂€銆?            Serial.print(F("Ultrasonic "));
            // 鎵撳嵃鏂瑰悜鍚嶇О銆?            Serial.print(SENSOR_NAMES[sensor]);
            // 鎵撳嵃鍐掑彿鍜岀┖鏍笺€?            Serial.print(F(": "));

            // 濡傛灉璇ヨ矾瓒呭０娉㈡暟鎹湁鏁堛€?            if (_ultrasonicMgr->isValid(sensor)) {
                // 鎵撳嵃鍘樼背鍗曚綅璺濈鍊笺€?                Serial.print(_ultrasonicMgr->getDistance(sensor) / 10.0f, 1);
                // 鎵撳嵃鍗曚綅銆?                Serial.println(F(" cm"));
            // 中文逐行说明：下面这一行保留原始代码 -> } else {
            } else {
                // 鍚﹀垯鎵撳嵃绂荤嚎鐘舵€併€?                Serial.println(F("offline"));
            // 中文逐行说明：下面这一行保留原始代码 -> }
            }
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 鎵撳嵃鏄剧ず灏鹃儴銆?    Serial.println(F("=============================================\n"));
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 瀹炵幇绱у噾鏄剧ず鍑芥暟銆?void SensorHub::displayCompact() {
    // 鎵撳嵃娣卞害瀛楁鍓嶇紑銆?    Serial.print(F("Sensors: depth="));
    // 濡傛灉娣卞害绠＄悊鍣ㄥ瓨鍦ㄤ笖鏁版嵁鏈夋晥銆?    if (_depthMgr && _depthMgr->isValid()) {
        // 鎵撳嵃涓€浣嶅皬鏁扮殑娣卞害鍊笺€?        Serial.print(_depthMgr->getDepthCm(), 1);
        // 鎵撳嵃娣卞害鍗曚綅銆?        Serial.print(F("cm"));
    // 中文逐行说明：下面这一行保留原始代码 -> } else {
    } else {
        // 濡傛灉鏃犳晥锛屽氨鎵撳嵃鍗犱綅绗︺€?        Serial.print(F("--"));
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 濡傛灉瓒呭０娉㈢鐞嗗櫒瀛樺湪銆?    if (_ultrasonicMgr) {
        // 鎵撳嵃瓒呭０娉㈠瓧娈靛墠缂€銆?        Serial.print(F(" | us="));
        // 閫愯矾閬嶅巻鍏ㄩ儴瓒呭０娉€?        for (uint8_t sensor = 0; sensor < NUM_ULTRASONIC; ++sensor) {
            // 濡傛灉璇ヨ矾瓒呭０娉㈡暟鎹湁鏁堛€?            if (_ultrasonicMgr->isValid(sensor)) {
                // 鎵撳嵃鏃犲皬鏁板帢绫冲€笺€?                Serial.print(_ultrasonicMgr->getDistance(sensor) / 10.0f, 0);
            // 中文逐行说明：下面这一行保留原始代码 -> } else {
            } else {
                // 濡傛灉鏃犳晥锛屽氨鎵撳嵃鍗犱綅绗︺€?                Serial.print(F("--"));
            // 中文逐行说明：下面这一行保留原始代码 -> }
            }
            // 濡傛灉杩樻病鍒版渶鍚庝竴璺紝灏辨墦鍗扮┖鏍煎垎闅斻€?            if (sensor + 1 < NUM_ULTRASONIC) {
                // 鎵撳嵃鍗曚釜绌烘牸瀛楃銆?                Serial.print(' ');
            // 中文逐行说明：下面这一行保留原始代码 -> }
            }
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 鎵撳嵃琛岀粨鏉熴€?    Serial.println();
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 瀹炵幇鍋ュ悍妫€鏌ュ嚱鏁般€?bool SensorHub::isHealthy() const {
    // 鍒ゆ柇娣卞害浼犳劅鍣ㄦ槸鍚﹀湪绾裤€?    const bool depthOk = _depthMgr && _depthMgr->isValid();
    // 鍒濆鍖栨湁鏁堣秴澹版尝鏁伴噺璁℃暟鍣ㄣ€?    uint8_t ultrasonicOk = 0;

    // 濡傛灉瓒呭０娉㈢鐞嗗櫒瀛樺湪銆?    if (_ultrasonicMgr) {
        // 閫愯矾妫€鏌ュ叏閮ㄨ秴澹版尝銆?        for (uint8_t sensor = 0; sensor < NUM_ULTRASONIC; ++sensor) {
            // 濡傛灉杩欎竴 璺秴澹版尝鏈夋晥銆?            if (_ultrasonicMgr->isValid(sensor)) {
                // 鎶婃湁鏁堣鏁板姞涓€銆?                ++ultrasonicOk;
            // 中文逐行说明：下面这一行保留原始代码 -> }
            }
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 鍙湁娣卞害鍦ㄧ嚎涓旇嚦灏?2 璺秴澹版尝鏈夋晥锛屾墠璁や负绯荤粺鍋ュ悍銆?    return depthOk && ultrasonicOk >= 2;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 瀹炵幇鏈夋晥浼犳劅鍣ㄦ暟閲忕粺璁″嚱鏁般€?uint8_t SensorHub::getSensorCount() const {
    // 鍒濆鍖栨湁鏁堜紶鎰熷櫒鏁伴噺涓?0銆?    uint8_t count = 0;

    // 濡傛灉娣卞害绠＄悊鍣ㄥ瓨鍦ㄤ笖鏁版嵁鏈夋晥銆?    if (_depthMgr && _depthMgr->isValid()) {
        // 鎶婃湁鏁堟暟閲忓姞涓€銆?        ++count;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 濡傛灉瓒呭０娉㈢鐞嗗櫒瀛樺湪銆?    if (_ultrasonicMgr) {
        // 閫愯矾妫€鏌ュ叏閮ㄨ秴澹版尝銆?        for (uint8_t sensor = 0; sensor < NUM_ULTRASONIC; ++sensor) {
            // 濡傛灉杩欎竴 璺秴澹版尝鏈夋晥銆?            if (_ultrasonicMgr->isValid(sensor)) {
                // 鎶婃湁鏁堟暟閲忓姞涓€銆?                ++count;
            // 中文逐行说明：下面这一行保留原始代码 -> }
            }
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 杩斿洖鏈€缁堟湁鏁堜紶鎰熷櫒鏁伴噺銆?    return count;
// 中文逐行说明：下面这一行保留原始代码 -> }
}
