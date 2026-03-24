/**********************************************************************
 * ForwardControl.h
 *
 * 杩欎釜鏂囦欢澹版槑鍓嶈繘瀛愮郴缁熺殑鏃跺簭鎺у埗妯″潡銆? * 瀹冨彧璐熻矗鐢熸垚鍓嶈繘瀛愮郴缁熷搴旂殑鎵ц鍣ㄤ綅鎺╃爜锛屼笉鐩存帴鍙戜覆鍙ｃ€? *********************************************************************/

// 中文逐行说明：下面这一行保留原始代码 -> #ifndef ESP32_FORWARD_CONTROL_H
#ifndef ESP32_FORWARD_CONTROL_H
// 中文逐行说明：下面这一行保留原始代码 -> #define ESP32_FORWARD_CONTROL_H
#define ESP32_FORWARD_CONTROL_H

// 中文逐行说明：下面这一行保留原始代码 -> #include <Arduino.h>
#include <Arduino.h>
// 中文逐行说明：下面这一行保留原始代码 -> #include "Protocol.h"
#include "Protocol.h"

// 中文逐行说明：下面这一行保留原始代码 -> class ForwardControl {
class ForwardControl {
// 中文逐行说明：下面这一行保留原始代码 -> public:
public:
    // 中文逐行说明：下面这一行保留原始代码 -> ForwardControl();
    ForwardControl();

    // 鍒濆鍖栧唴閮ㄧ姸鎬併€?    void begin();

    // 寮€濮嬫寔缁墠杩涖€?    void start();

    // 鍋滄鍓嶈繘锛屽苟杩涘叆鐭殏鐨勫钩琛￠樁娈点€?    void stop();

    // 鍏ㄥ眬鍋滄満鏃剁洿鎺ユ竻绌烘墍鏈夌姸鎬併€?    void emergencyStop();

    // 鏍规嵁褰撳墠鏃堕挓鎺ㄨ繘鍓嶈繘鑺傛媿銆?    void update(uint32_t nowMs);

    // 杈撳嚭褰撳墠鍓嶈繘瀛愮郴缁熼渶瑕佹縺娲荤殑鎵ц鍣ㄤ綅銆?    uint16_t getMask() const;

    // 鐢ㄤ簬璋冭瘯鍜岀姸鎬佹樉绀恒€?    bool isRunning() const;
    // 中文逐行说明：下面这一行保留原始代码 -> bool isBalancing() const;
    bool isBalancing() const;
    // 中文逐行说明：下面这一行保留原始代码 -> bool isBusy() const;
    bool isBusy() const;

// 中文逐行说明：下面这一行保留原始代码 -> private:
private:
    // 中文逐行说明：下面这一行保留原始代码 -> bool _running;
    bool _running;
    // 中文逐行说明：下面这一行保留原始代码 -> bool _balancing;
    bool _balancing;
    // 中文逐行说明：下面这一行保留原始代码 -> bool _valvesOpen;
    bool _valvesOpen;
    // 中文逐行说明：下面这一行保留原始代码 -> bool _balanceValveOpen;
    bool _balanceValveOpen;
    // 中文逐行说明：下面这一行保留原始代码 -> uint32_t _phaseStartMs;
    uint32_t _phaseStartMs;
    // 中文逐行说明：下面这一行保留原始代码 -> uint32_t _balanceStartMs;
    uint32_t _balanceStartMs;
// 中文逐行说明：下面这一行保留原始代码 -> };
};

// 中文逐行说明：下面这一行保留原始代码 -> #endif // ESP32_FORWARD_CONTROL_H
#endif // ESP32_FORWARD_CONTROL_H
