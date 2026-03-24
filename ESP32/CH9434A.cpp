/**********************************************************************
 * CH9434A.cpp
 *
 * 杩欎釜鏂囦欢瀹炵幇 CH9434A 鑺墖鐨勫簳灞?SPI 璇诲啓椹卞姩銆? *********************************************************************/

// 寮曞叆 CH9434A 绫诲０鏄庛€?#include "CH9434A.h"

// 瀹炵幇鏋勯€犲嚱鏁般€?CH9434A::CH9434A(uint8_t csPin, int8_t intPin)
    // 淇濆瓨鐗囬€夊紩鑴氱紪鍙枫€?    : _csPin(csPin),
      // 淇濆瓨涓柇寮曡剼缂栧彿銆?      _intPin(intPin) {
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 瀹炵幇鍒濆鍖栧嚱鏁般€?bool CH9434A::begin(uint32_t spiFreq) {
    // 鎶婄墖閫夎剼閰嶇疆涓鸿緭鍑恒€?    pinMode(_csPin, OUTPUT);
    // 榛樿鍏堟媺楂樼墖閫夎剼銆?    digitalWrite(_csPin, HIGH);

    // 濡傛灉鎻愪緵浜嗕腑鏂剼銆?    if (_intPin >= 0) {
        // 灏辨妸涓柇鑴氶厤缃负涓婃媺杈撳叆銆?        pinMode(_intPin, INPUT_PULLUP);
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 鍒濆鍖?SPI 澶栬銆?    SPI.begin();
    // 淇濆瓨 SPI 鏃跺簭閰嶇疆銆?    _spiSettings = SPISettings(spiFreq, MSBFIRST, SPI_MODE0);

    // 涓婄數鍚庡厛绛夊緟鑺墖绋冲畾銆?    delay(100);

    // 閫愯矾鍒濆鍖?4 涓?UART 閫氶亾銆?    for (uint8_t i = 0; i < CH9434A_NUM_UARTS; i++) {
        // 鍏抽棴璇ラ€氶亾鎵€鏈変腑鏂€?        writeReg(i, CH9434A_REG_IER, 0x00);
        // 浣胯兘 FIFO 骞跺悓鏃舵竻绌烘敹鍙?FIFO銆?        writeReg(i, CH9434A_REG_FCR,
                 // 中文逐行说明：下面这一行保留原始代码 -> CH9434A_FCR_ENABLE | CH9434A_FCR_RX_RESET | CH9434A_FCR_TX_RESET);
                 CH9434A_FCR_ENABLE | CH9434A_FCR_RX_RESET | CH9434A_FCR_TX_RESET);
        // 鎶婁覆鍙ｆ牸寮忚涓?8N1銆?        writeReg(i, CH9434A_REG_LCR, CH9434A_LCR_8N1);
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 璇诲彇 UART0 鐨勭嚎璺姸鎬佸瘎瀛樺櫒鍋氫竴娆″熀纭€杩為€氭€ф鏌ャ€?    const uint8_t lsr = readReg(0, CH9434A_REG_LSR);
    // 杩囨护鏄庢樉鏃犳晥鐨勮繑鍥炲€笺€?    return (lsr != 0xFF && lsr != 0x00 && lsr != 0x10);
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 瀹炵幇 UART 閰嶇疆鍑芥暟銆?bool CH9434A::config(uint8_t uartNum, uint32_t baudrate, uint8_t config) {
    // 濡傛灉 UART 缂栧彿瓒婄晫锛屽氨杩斿洖澶辫触銆?    if (uartNum >= CH9434A_NUM_UARTS) {
        // 中文逐行说明：下面这一行保留原始代码 -> return false;
        return false;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 鍏堣缃?UART 娉㈢壒鐜囥€?    setBaudrate(uartNum, baudrate);
    // 鍐嶅啓鍏ョ嚎璺牸寮忛厤缃€?    writeReg(uartNum, CH9434A_REG_LCR, config);
    // 浣胯兘 FIFO 骞舵竻绌烘敹鍙戠紦瀛樸€?    writeReg(uartNum, CH9434A_REG_FCR,
             // 中文逐行说明：下面这一行保留原始代码 -> CH9434A_FCR_ENABLE | CH9434A_FCR_RX_RESET | CH9434A_FCR_TX_RESET);
             CH9434A_FCR_ENABLE | CH9434A_FCR_RX_RESET | CH9434A_FCR_TX_RESET);

    // 杩斿洖閰嶇疆鎴愬姛銆?    return true;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 瀹炵幇鍐欏崟瀛楄妭鍑芥暟銆?void CH9434A::write(uint8_t uartNum, uint8_t data) {
    // 濡傛灉 UART 缂栧彿瓒婄晫锛屽氨鐩存帴杩斿洖銆?    if (uartNum >= CH9434A_NUM_UARTS) {
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 璁＄畻鍐欒秴鏃舵椂鍒汇€?    const uint32_t timeout = millis() + 100;
    // 鎸佺画绛夊緟鍙戦€佷繚鎸佸瘎瀛樺櫒鍙樹负绌恒€?    while (!(readReg(uartNum, CH9434A_REG_LSR) & CH9434A_LSR_THRE)) {
        // 濡傛灉宸茬粡瓒呮椂锛屽氨鏀惧純鏈鍙戦€併€?        if (millis() > timeout) {
            // 中文逐行说明：下面这一行保留原始代码 -> return;
            return;
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }
        // 璁╁嚭 CPU锛岄伩鍏嶉暱鏃堕棿蹇欑瓑銆?        yield();
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 鎶婄洰鏍囧瓧鑺傚啓鍏ュ彂閫佷繚鎸佸瘎瀛樺櫒銆?    writeReg(uartNum, CH9434A_REG_THR, data);
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 瀹炵幇鍐欑紦鍐插尯鍑芥暟銆?size_t CH9434A::write(uint8_t uartNum, const uint8_t* buffer, size_t length) {
    // 閫愬瓧鑺傚啓鍑烘暣涓紦鍐插尯銆?    for (size_t i = 0; i < length; i++) {
        // 璋冪敤鍗曞瓧鑺傚啓鍑芥暟銆?        write(uartNum, buffer[i]);
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
    // 杩斿洖璇锋眰鍐欏叆鐨勬€婚暱搴︺€?    return length;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 瀹炵幇鎵撳嵃瀛楃涓插嚱鏁般€?size_t CH9434A::print(uint8_t uartNum, const char* str) {
    // 鎶婂瓧绗︿覆寮鸿浆鎴愬瓧鑺傛暟缁勫悗璋冪敤缂撳啿鍖哄啓鍑芥暟銆?    return write(uartNum, reinterpret_cast<const uint8_t*>(str), strlen(str));
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 瀹炵幇鎵撳嵃瀛楃涓插苟鎹㈣鍑芥暟銆?size_t CH9434A::println(uint8_t uartNum, const char* str) {
    // 鍏堟墦鍗板瓧绗︿覆涓讳綋銆?    const size_t n = print(uartNum, str);
    // 鍙戦€佸洖杞︾銆?    write(uartNum, '\r');
    // 鍙戦€佹崲琛岀銆?    write(uartNum, '\n');
    // 杩斿洖鎬诲啓鍑哄瓧绗︽暟銆?    return n + 2;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 瀹炵幇璇诲崟瀛楄妭鍑芥暟銆?uint8_t CH9434A::read(uint8_t uartNum) {
    // 濡傛灉 UART 缂栧彿瓒婄晫锛屽氨杩斿洖 0銆?    if (uartNum >= CH9434A_NUM_UARTS) {
        // 中文逐行说明：下面这一行保留原始代码 -> return 0;
        return 0;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 濡傛灉鎺ユ敹鏁版嵁灏辩华浣嶆湭缃綅锛屽氨璇存槑褰撳墠娌℃湁鏁版嵁銆?    if (!(readReg(uartNum, CH9434A_REG_LSR) & CH9434A_LSR_DR)) {
        // 中文逐行说明：下面这一行保留原始代码 -> return 0;
        return 0;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 浠庢帴鏀朵繚鎸佸瘎瀛樺櫒璇诲彇涓€涓瓧鑺傚苟杩斿洖銆?    return readReg(uartNum, CH9434A_REG_RHR);
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 瀹炵幇璇诲涓瓧鑺傚嚱鏁般€?size_t CH9434A::read(uint8_t uartNum, uint8_t* buffer, size_t maxLength) {
    // 鍒濆鍖栧凡璇诲彇瀛楄妭鏁颁负 0銆?    size_t count = 0;

    // 鍦ㄧ紦鍐插尯鏈弧鏃舵寔缁鍙栥€?    while (count < maxLength) {
        // 濡傛灉褰撳墠宸茬粡娌℃湁鍙鏁版嵁锛屽氨閫€鍑哄惊鐜€?        if (!(readReg(uartNum, CH9434A_REG_LSR) & CH9434A_LSR_DR)) {
            // 中文逐行说明：下面这一行保留原始代码 -> break;
            break;
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }
        // 璇诲彇涓€涓瓧鑺傛斁鍏ョ紦鍐插尯锛屽苟閫掑璁℃暟鍣ㄣ€?        buffer[count++] = readReg(uartNum, CH9434A_REG_RHR);
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }

    // 杩斿洖瀹為檯璇诲彇鐨勫瓧鑺傛暟銆?    return count;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 瀹炵幇鍙鏁版嵁鏌ヨ鍑芥暟銆?int CH9434A::available(uint8_t uartNum) {
    // 濡傛灉 UART 缂栧彿瓒婄晫锛屽氨杩斿洖 0銆?    if (uartNum >= CH9434A_NUM_UARTS) {
        // 中文逐行说明：下面这一行保留原始代码 -> return 0;
        return 0;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
    // 鏍规嵁鏁版嵁灏辩华浣嶈繑鍥?1 鎴?0銆?    return (readReg(uartNum, CH9434A_REG_LSR) & CH9434A_LSR_DR) ? 1 : 0;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 瀹炵幇娓呯┖鎺ユ敹 FIFO 鐨勫嚱鏁般€?void CH9434A::flush(uint8_t uartNum) {
    // 濡傛灉 UART 缂栧彿瓒婄晫锛屽氨鐩存帴杩斿洖銆?    if (uartNum >= CH9434A_NUM_UARTS) {
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
    // 浣胯兘 FIFO 骞跺浣嶆帴鏀?FIFO銆?    writeReg(uartNum, CH9434A_REG_FCR, CH9434A_FCR_ENABLE | CH9434A_FCR_RX_RESET);
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 瀹炵幇绾胯矾鐘舵€佽鍙栧嚱鏁般€?uint8_t CH9434A::getLineStatus(uint8_t uartNum) {
    // 濡傛灉 UART 缂栧彿瓒婄晫锛屽氨杩斿洖 0銆?    if (uartNum >= CH9434A_NUM_UARTS) {
        // 中文逐行说明：下面这一行保留原始代码 -> return 0;
        return 0;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
    // 璇诲彇骞惰繑鍥炵嚎璺姸鎬佸瘎瀛樺櫒銆?    return readReg(uartNum, CH9434A_REG_LSR);
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 瀹炵幇涓柇鐘舵€佽鍙栧嚱鏁般€?uint8_t CH9434A::getInterruptStatus(uint8_t uartNum) {
    // 濡傛灉 UART 缂栧彿瓒婄晫锛屽氨杩斿洖 0銆?    if (uartNum >= CH9434A_NUM_UARTS) {
        // 中文逐行说明：下面这一行保留原始代码 -> return 0;
        return 0;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
    // 璇诲彇骞惰繑鍥炰腑鏂姸鎬佸瘎瀛樺櫒銆?    return readReg(uartNum, CH9434A_REG_IIR);
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 瀹炵幇涓柇浣胯兘鍑芥暟銆?void CH9434A::enableInterrupt(uint8_t uartNum, uint8_t flags) {
    // 濡傛灉 UART 缂栧彿瓒婄晫锛屽氨鐩存帴杩斿洖銆?    if (uartNum >= CH9434A_NUM_UARTS) {
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
    // 鍚戜腑鏂娇鑳藉瘎瀛樺櫒鍐欏叆鐩爣鏍囧織浣嶃€?    writeReg(uartNum, CH9434A_REG_IER, flags);
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 瀹炵幇涓柇鍏抽棴鍑芥暟銆?void CH9434A::disableInterrupt(uint8_t uartNum) {
    // 濡傛灉 UART 缂栧彿瓒婄晫锛屽氨鐩存帴杩斿洖銆?    if (uartNum >= CH9434A_NUM_UARTS) {
        // 中文逐行说明：下面这一行保留原始代码 -> return;
        return;
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
    // 鎶婁腑鏂娇鑳藉瘎瀛樺櫒娓呴浂銆?    writeReg(uartNum, CH9434A_REG_IER, 0x00);
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 瀹炵幇鍙戦€佸畬鎴愭鏌ュ嚱鏁般€?bool CH9434A::isTxEmpty(uint8_t uartNum) {
    // 妫€鏌ョ嚎璺姸鎬佸瘎瀛樺櫒涓殑鍙戦€佸畬鍏ㄧ┖闂蹭綅銆?    return (getLineStatus(uartNum) & CH9434A_LSR_TEMT) != 0;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 瀹炵幇绛夊緟鍙戦€佸畬鎴愬嚱鏁般€?void CH9434A::waitTxComplete(uint8_t uartNum, uint32_t timeout) {
    // 璁板綍绛夊緟璧峰鏃堕棿銆?    const uint32_t start = millis();
    // 鍙鍙戦€佽繕娌″畬鎴愶紝灏辨寔缁瓑寰呫€?    while (!isTxEmpty(uartNum)) {
        // 濡傛灉绛夊緟鏃堕棿宸茬粡瓒呮椂锛屽氨閫€鍑恒€?        if (millis() - start > timeout) {
            // 中文逐行说明：下面这一行保留原始代码 -> break;
            break;
        // 中文逐行说明：下面这一行保留原始代码 -> }
        }
        // 璁╁嚭 CPU锛岄伩鍏嶉暱鏃堕棿蹇欑瓑銆?        yield();
    // 中文逐行说明：下面这一行保留原始代码 -> }
    }
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 瀹炵幇 UART 鍩哄湴鍧€鑾峰彇鍑芥暟銆?uint16_t CH9434A::getBaseAddr(uint8_t uartNum) {
    // 褰撳墠瀹炵幇涓嶇洿鎺ヤ娇鐢ㄥ熀鍦板潃锛屽洜姝や粎娑堥櫎鏈娇鐢ㄥ弬鏁拌鍛娿€?    (void)uartNum;
    // 杩斿洖 0 浣滀负鍗犱綅鍊笺€?    return 0;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 瀹炵幇瀵勫瓨鍣ㄥ啓鍑芥暟銆?void CH9434A::writeReg(uint8_t uartNum, uint8_t reg, uint8_t value) {
    // 璁＄畻鐩爣瀵勫瓨鍣ㄥ湴鍧€銆?    const uint8_t addr = reg + (CH9434A_UART_BASE_STEP * uartNum);

    // 寮€濮嬩竴娆?SPI 浜嬪姟銆?    SPI.beginTransaction(_spiSettings);
    // 鎷変綆鐗囬€夎剼锛岄€変腑 CH9434A銆?    digitalWrite(_csPin, LOW);

    // 鍙戦€佸啓鍛戒护锛屾渶楂樹綅涓?1銆?    SPI.transfer(0x80 | addr);
    // 鎸夊綋鍓嶇粡楠屽鍔犵煭寤舵椂锛屾敼鍠勬椂搴忕ǔ瀹氭€с€?    delayMicroseconds(1);
    // 鍙戦€佸瘎瀛樺櫒鍊笺€?    SPI.transfer(value);

    // 鍐嶅鍔犱笁涓煭寤舵椂锛岀粰鑺墖鐣欏嚭鍐呴儴閿佸瓨鏃堕棿銆?    delayMicroseconds(1);
    // 中文逐行说明：下面这一行保留原始代码 -> delayMicroseconds(1);
    delayMicroseconds(1);
    // 中文逐行说明：下面这一行保留原始代码 -> delayMicroseconds(1);
    delayMicroseconds(1);

    // 鎷夐珮鐗囬€夎剼锛岀粨鏉熷鑺墖鐨勮闂€?    digitalWrite(_csPin, HIGH);
    // 缁撴潫 SPI 浜嬪姟銆?    SPI.endTransaction();
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 瀹炵幇瀵勫瓨鍣ㄨ鍑芥暟銆?uint8_t CH9434A::readReg(uint8_t uartNum, uint8_t reg) {
    // 璁＄畻鐩爣瀵勫瓨鍣ㄥ湴鍧€銆?    const uint8_t addr = reg + (CH9434A_UART_BASE_STEP * uartNum);

    // 寮€濮嬩竴娆?SPI 浜嬪姟銆?    SPI.beginTransaction(_spiSettings);
    // 鎷変綆鐗囬€夎剼锛岄€変腑 CH9434A銆?    digitalWrite(_csPin, LOW);

    // 鍙戦€佽鍛戒护锛屾渶楂樹綅淇濇寔 0銆?    SPI.transfer(addr);
    // 澧炲姞涓変釜鐭欢鏃讹紝婊¤冻褰撳墠椹卞姩缁忛獙鏃跺簭銆?    delayMicroseconds(1);
    // 中文逐行说明：下面这一行保留原始代码 -> delayMicroseconds(1);
    delayMicroseconds(1);
    // 中文逐行说明：下面这一行保留原始代码 -> delayMicroseconds(1);
    delayMicroseconds(1);

    // 鍙戦€?dummy 瀛楄妭骞跺悓鏃跺彇鍥炶繑鍥炲€笺€?    const uint8_t value = SPI.transfer(0xFF);
    // 鍐嶅鍔犱竴涓煭寤舵椂銆?    delayMicroseconds(1);

    // 鎷夐珮鐗囬€夎剼锛岀粨鏉熷鑺墖鐨勮闂€?    digitalWrite(_csPin, HIGH);
    // 缁撴潫 SPI 浜嬪姟銆?    SPI.endTransaction();

    // 杩斿洖璇诲埌鐨勫瘎瀛樺櫒鍊笺€?    return value;
// 中文逐行说明：下面这一行保留原始代码 -> }
}

// 瀹炵幇娉㈢壒鐜囬厤缃嚱鏁般€?void CH9434A::setBaudrate(uint8_t uartNum, uint32_t baudrate) {
    // 瀹氫箟 CH9434A 鐨勫唴閮ㄦ椂閽熼鐜囥€?    const uint32_t clockFreq = 32000000;
    // 鏍规嵁瀹樻柟鍏紡璁＄畻鍒嗛鍊笺€?    const uint16_t divisor = clockFreq / (8 * baudrate);

    // 鍏堣鍙栧綋鍓嶇嚎璺帶鍒跺瘎瀛樺櫒銆?    const uint8_t lcr = readReg(uartNum, CH9434A_REG_LCR);
    // 缃綅 DLAB锛屽垏鎹㈠埌鍒嗛瀵勫瓨鍣ㄨ闂ā寮忋€?    writeReg(uartNum, CH9434A_REG_LCR, lcr | CH9434A_LCR_DLAB);
    // 鍐欏叆鍒嗛浣庡瓧鑺傘€?    writeReg(uartNum, CH9434A_REG_DLL, divisor & 0xFF);
    // 鍐欏叆鍒嗛楂樺瓧鑺傘€?    writeReg(uartNum, CH9434A_REG_DLM, (divisor >> 8) & 0xFF);
    // 娓呴櫎 DLAB锛屾仮澶嶆甯稿瘎瀛樺櫒璁块棶妯″紡銆?    writeReg(uartNum, CH9434A_REG_LCR, lcr & ~CH9434A_LCR_DLAB);
// 中文逐行说明：下面这一行保留原始代码 -> }
}
