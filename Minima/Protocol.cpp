/**********************************************************************
 * Protocol.cpp
 *
 * 杩欎釜鏂囦欢瀹炵幇 Minima 渚у浐瀹氶暱搴﹀崗璁抚鐨勬敹鍙戝拰鏍￠獙銆? *********************************************************************/

// 寮曞叆鍗忚澶存枃浠躲€?#include "Protocol.h"

// 瀹炵幇鍗忚鎺ユ敹鍣ㄦ瀯閫犲嚱鏁般€?ProtocolReceiver::ProtocolReceiver()
    // 鍒濆鍖栨帴鏀剁储寮曚负 0銆?    : rxIndex(0) {}

// 瀹炵幇 CRC8 璁＄畻鍑芥暟銆?uint8_t calculateCRC8(const uint8_t* data, uint8_t len) {
    // 鍒濆鍖?CRC 瀵勫瓨鍣ㄣ€?    uint8_t crc = 0x00;
    // 閫愬瓧鑺傚鐞嗚緭鍏ユ暟鎹€?    for (uint8_t i = 0; i < len; i++) {
        // 鎶婂綋鍓嶅瓧鑺傚紓鎴栬繘 CRC銆?        crc ^= data[i];
        // 瀵瑰綋鍓嶅瓧鑺傛墽琛?8 娆＄Щ浣嶈凯浠ｃ€?        for (uint8_t j = 0; j < 8; j++) {
            // 濡傛灉鏈€楂樹綅涓?1銆?            if (crc & 0x80) {
                // 灏卞乏绉诲苟涓庡椤瑰紡 0x07 寮傛垨銆?                crc = (crc << 1) ^ 0x07;
            // 中文逐行说明：下面这一行保留原始代码 -> } else {
            } else {
                // 鍚﹀垯鍙墽琛屽乏绉汇€?                crc <<= 1;
            // 中文逐行说明：下面这一行保留原始代码 -> }
            }
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
    // 杩斿洖鏈€缁?CRC 鍊笺€?    return crc;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 瀹炵幇鍗忚甯у彂閫佸嚱鏁般€?void sendFrame(Stream& port, uint8_t cmd, uint8_t data0, uint8_t data1, uint8_t data2) {
    // 鍒涘缓鍥哄畾闀垮害甯х紦鍐插尯銆?    uint8_t frame[FRAME_LENGTH];

    // 鍐欏叆甯уご銆?    frame[0] = FRAME_HEADER;
    // 鍐欏叆甯ч暱搴︺€?    frame[1] = FRAME_LENGTH;
    // 鍐欏叆鍛戒护瀛楄妭銆?    frame[2] = cmd;
    // 鍐欏叆绗竴涓暟鎹瓧鑺傘€?    frame[3] = data0;
    // 鍐欏叆绗簩涓暟鎹瓧鑺傘€?    frame[4] = data1;
    // 鍐欏叆绗笁涓暟鎹瓧鑺傘€?    frame[5] = data2;
    // 璁＄畻骞跺啓鍏?CRC8銆?    frame[6] = calculateCRC8(&frame[2], 4);
    // 鍐欏叆甯у熬銆?    frame[7] = FRAME_TAIL;

    // 鎶婃暣甯у彂閫佸埌鎸囧畾涓插彛銆?    port.write(frame, FRAME_LENGTH);
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 瀹炵幇鍗忚鎺ユ敹杞鍑芥暟銆?bool ProtocolReceiver::poll(Stream& port, ProtocolFrame& frame) {
    // 鍙涓插彛閲岃繕鏈夋暟鎹紝灏辨寔缁鍙栥€?    while (port.available()) {
        // 璇诲彇涓€涓瓧鑺傘€?        const uint8_t byte = static_cast<uint8_t>(port.read());

        // 濡傛灉褰撳墠鏈熷緟甯уご锛岃€屾敹鍒扮殑涓嶆槸甯уご锛屽氨涓㈠純瀹冦€?        if (rxIndex == 0 && byte != FRAME_HEADER) {
            // 中文逐行说明：下面这一行保留原始代码 -> continue;
            continue;
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }

        // 鎶婂綋鍓嶅瓧鑺傚啓鍏ユ帴鏀剁紦鍐插尯銆?        rxBuffer[rxIndex++] = byte;

        // 濡傛灉褰撳墠杩樻病鏈夋敹婊′竴甯э紝灏辩户缁鍙栥€?        if (rxIndex < FRAME_LENGTH) {
            // 中文逐行说明：下面这一行保留原始代码 -> continue;
            continue;
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }

        // 鍒ゆ柇甯у熬鏄惁姝ｇ‘銆?        const bool validTail = rxBuffer[7] == FRAME_TAIL;
        // 閲嶆柊璁＄畻 CRC銆?        const uint8_t crc = calculateCRC8(&rxBuffer[2], 4);
        // 鍒ゆ柇 CRC 鏄惁鍖归厤銆?        const bool validCrc = crc == rxBuffer[6];

        // 涓€甯у鐞嗙粨鏉熷悗閲嶇疆鎺ユ敹绱㈠紩銆?        rxIndex = 0;

        // 濡傛灉甯у熬鎴?CRC 閿欒锛屽氨涓㈠純杩欎竴甯с€?        if (!validTail || !validCrc) {
            // 中文逐行说明：下面这一行保留原始代码 -> continue;
            continue;
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }

        // 鎶婂懡浠ゅ瓧鑺傛嫹璐濆埌杈撳嚭缁撴瀯浣撱€?        frame.cmd = rxBuffer[2];
        // 鎶婄涓€涓暟鎹瓧鑺傛嫹璐濆埌杈撳嚭缁撴瀯浣撱€?        frame.data0 = rxBuffer[3];
        // 鎶婄浜屼釜鏁版嵁瀛楄妭鎷疯礉鍒拌緭鍑虹粨鏋勪綋銆?        frame.data1 = rxBuffer[4];
        // 鎶婄涓変釜鏁版嵁瀛楄妭鎷疯礉鍒拌緭鍑虹粨鏋勪綋銆?        frame.data2 = rxBuffer[5];
        // 杩斿洖 true锛岃〃绀烘垚鍔熸敹鍒颁竴甯ф湁鏁堝懡浠ゃ€?        return true;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 濡傛灉娌℃湁鏀跺埌瀹屾暣鏈夋晥甯э紝灏辫繑鍥?false銆?    return false;
// 中文逐行说明：下面这一行保留原始代码 -> }
}
