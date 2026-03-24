/**********************************************************************
 * CH9434A.h
 *
 * 杩欎釜鏂囦欢澹版槑 CH9434A SPI 杞?4 璺?UART 鑺墖鐨勯┍鍔ㄦ帴鍙ｃ€? *********************************************************************/

// 闃叉澶存枃浠惰閲嶅鍖呭惈銆?#ifndef CH9434A_H
// 瀹氫箟澶存枃浠朵繚鎶ゅ畯銆?#define CH9434A_H

// 寮曞叆 Arduino 鍩虹绫诲瀷銆?#include <Arduino.h>
// 寮曞叆 Arduino SPI 鎺ュ彛瀹氫箟銆?#include <SPI.h>

// 瀹氫箟鎺ユ敹淇濇寔瀵勫瓨鍣ㄥ湴鍧€銆?#define CH9434A_REG_RHR         0x00
// 瀹氫箟鍙戦€佷繚鎸佸瘎瀛樺櫒鍦板潃銆?#define CH9434A_REG_THR         0x00
// 瀹氫箟涓柇浣胯兘瀵勫瓨鍣ㄥ湴鍧€銆?#define CH9434A_REG_IER         0x01
// 瀹氫箟涓柇璇嗗埆瀵勫瓨鍣ㄥ湴鍧€銆?#define CH9434A_REG_IIR         0x02
// 瀹氫箟 FIFO 鎺у埗瀵勫瓨鍣ㄥ湴鍧€銆?#define CH9434A_REG_FCR         0x02
// 瀹氫箟绾胯矾鎺у埗瀵勫瓨鍣ㄥ湴鍧€銆?#define CH9434A_REG_LCR         0x03
// 瀹氫箟璋冨埗瑙ｈ皟鍣ㄦ帶鍒跺瘎瀛樺櫒鍦板潃銆?#define CH9434A_REG_MCR         0x04
// 瀹氫箟绾胯矾鐘舵€佸瘎瀛樺櫒鍦板潃銆?#define CH9434A_REG_LSR         0x05
// 瀹氫箟璋冨埗瑙ｈ皟鍣ㄧ姸鎬佸瘎瀛樺櫒鍦板潃銆?#define CH9434A_REG_MSR         0x06
// 瀹氫箟鍒嗛浣庡瓧鑺傚瘎瀛樺櫒鍦板潃銆?#define CH9434A_REG_DLL         0x00
// 瀹氫箟鍒嗛楂樺瓧鑺傚瘎瀛樺櫒鍦板潃銆?#define CH9434A_REG_DLM         0x01

// 瀹氫箟 DLAB 浣嶆帺鐮併€?#define CH9434A_LCR_DLAB        0x80
// 瀹氫箟 SBC 浣嶆帺鐮併€?#define CH9434A_LCR_SBC         0x40
// 瀹氫箟鍋舵牎楠岄€夋嫨浣嶆帺鐮併€?#define CH9434A_LCR_EPAR        0x10
// 瀹氫箟鏍￠獙浣胯兘浣嶆帺鐮併€?#define CH9434A_LCR_PARITY      0x08
// 瀹氫箟鍋滄浣嶉厤缃綅鎺╃爜銆?#define CH9434A_LCR_STOPB       0x04
// 瀹氫箟 8N1 閰嶇疆鍊笺€?#define CH9434A_LCR_8N1         0x03
// 瀹氫箟 7N1 閰嶇疆鍊笺€?#define CH9434A_LCR_7N1         0x02
// 瀹氫箟 6N1 閰嶇疆鍊笺€?#define CH9434A_LCR_6N1         0x01
// 瀹氫箟 5N1 閰嶇疆鍊笺€?#define CH9434A_LCR_5N1         0x00

// 瀹氫箟鍙戦€佸畬鍏ㄧ┖闂蹭綅鎺╃爜銆?#define CH9434A_LSR_TEMT        0x40
// 瀹氫箟鍙戦€佷繚鎸佸瘎瀛樺櫒绌轰綅鎺╃爜銆?#define CH9434A_LSR_THRE        0x20
// 瀹氫箟涓柇涓浣嶆帺鐮併€?#define CH9434A_LSR_BI          0x10
// 瀹氫箟甯ч敊璇綅鎺╃爜銆?#define CH9434A_LSR_FE          0x08
// 瀹氫箟鏍￠獙閿欒浣嶆帺鐮併€?#define CH9434A_LSR_PE          0x04
// 瀹氫箟婧㈠嚭閿欒浣嶆帺鐮併€?#define CH9434A_LSR_OE          0x02
// 瀹氫箟鎺ユ敹鏁版嵁灏辩华浣嶆帺鐮併€?#define CH9434A_LSR_DR          0x01

// 瀹氫箟 FIFO 浣胯兘浣嶆帺鐮併€?#define CH9434A_FCR_ENABLE      0x01
// 瀹氫箟鎺ユ敹 FIFO 澶嶄綅浣嶆帺鐮併€?#define CH9434A_FCR_RX_RESET    0x02
// 瀹氫箟鍙戦€?FIFO 澶嶄綅浣嶆帺鐮併€?#define CH9434A_FCR_TX_RESET    0x04

// 瀹氫箟鎺ユ敹涓柇浣胯兘浣嶆帺鐮併€?#define CH9434A_IER_RX          0x01
// 瀹氫箟鍙戦€佷腑鏂娇鑳戒綅鎺╃爜銆?#define CH9434A_IER_TX          0x02
// 瀹氫箟绾胯矾鐘舵€佷腑鏂娇鑳戒綅鎺╃爜銆?#define CH9434A_IER_LSR         0x04
// 瀹氫箟璋冨埗瑙ｈ皟鍣ㄧ姸鎬佷腑鏂娇鑳戒綅鎺╃爜銆?#define CH9434A_IER_MSR         0x08

// 瀹氫箟鑺墖鏀寔鐨?UART 閫氶亾鏁伴噺銆?#define CH9434A_NUM_UARTS       4
// 瀹氫箟鐩搁偦 UART 閫氶亾鐨勫瘎瀛樺櫒鍦板潃姝ラ暱銆?#define CH9434A_UART_BASE_STEP  0x10

// 澹版槑 CH9434A 椹卞姩绫汇€?class CH9434A {
// 中文逐行说明：下面这一行保留原始代码 -> public:
public:
    // 澹版槑鏋勯€犲嚱鏁帮紝鍏佽浼犲叆鐗囬€夎剼鍜屽彲閫変腑鏂剼銆?    CH9434A(uint8_t csPin, int8_t intPin = -1);

    // 澹版槑鍒濆鍖栬姱鐗囧拰 SPI 鍙傛暟鐨勫嚱鏁般€?    bool begin(uint32_t spiFreq = 10000000);

    // 澹版槑閰嶇疆鎸囧畾 UART 娉㈢壒鐜囧拰鏍煎紡鐨勫嚱鏁般€?    bool config(uint8_t uartNum, uint32_t baudrate, uint8_t config = CH9434A_LCR_8N1);

    // 澹版槑鍚戞寚瀹?UART 鍐欏叆鍗曚釜瀛楄妭鐨勫嚱鏁般€?    void write(uint8_t uartNum, uint8_t data);
    // 澹版槑鍚戞寚瀹?UART 鍐欏叆鏁翠釜缂撳啿鍖虹殑鍑芥暟銆?    size_t write(uint8_t uartNum, const uint8_t* buffer, size_t length);
    // 澹版槑鍚戞寚瀹?UART 鎵撳嵃瀛楃涓茬殑鍑芥暟銆?    size_t print(uint8_t uartNum, const char* str);
    // 澹版槑鍚戞寚瀹?UART 鎵撳嵃涓€琛屽瓧绗︿覆鐨勫嚱鏁般€?    size_t println(uint8_t uartNum, const char* str);

    // 澹版槑浠庢寚瀹?UART 璇诲彇鍗曚釜瀛楄妭鐨勫嚱鏁般€?    uint8_t read(uint8_t uartNum);
    // 澹版槑浠庢寚瀹?UART 璇诲彇澶氫釜瀛楄妭鐨勫嚱鏁般€?    size_t read(uint8_t uartNum, uint8_t* buffer, size_t maxLength);
    // 澹版槑鏌ヨ鎸囧畾 UART 鏄惁鏈夋暟鎹彲璇荤殑鍑芥暟銆?    int available(uint8_t uartNum);
    // 澹版槑娓呯┖鎸囧畾 UART 鎺ユ敹 FIFO 鐨勫嚱鏁般€?    void flush(uint8_t uartNum);

    // 澹版槑璇诲彇绾胯矾鐘舵€佸瘎瀛樺櫒鐨勫嚱鏁般€?    uint8_t getLineStatus(uint8_t uartNum);
    // 澹版槑璇诲彇涓柇鐘舵€佸瘎瀛樺櫒鐨勫嚱鏁般€?    uint8_t getInterruptStatus(uint8_t uartNum);

    // 澹版槑寮€鍚?UART 涓柇鐨勫嚱鏁般€?    void enableInterrupt(uint8_t uartNum, uint8_t flags);
    // 澹版槑鍏抽棴 UART 涓柇鐨勫嚱鏁般€?    void disableInterrupt(uint8_t uartNum);

    // 澹版槑鏌ヨ鍙戦€佹槸鍚﹀畬鎴愮殑鍑芥暟銆?    bool isTxEmpty(uint8_t uartNum);
    // 澹版槑绛夊緟鍙戦€佸畬鎴愮殑鍑芥暟銆?    void waitTxComplete(uint8_t uartNum, uint32_t timeout = 100);

// 中文逐行说明：下面这一行保留原始代码 -> private:
private:
    // 淇濆瓨 SPI 鐗囬€夎剼缂栧彿銆?    uint8_t _csPin;
    // 淇濆瓨鍙€変腑鏂剼缂栧彿銆?    int8_t _intPin;
    // 淇濆瓨 SPI 鏃跺簭閰嶇疆瀵硅薄銆?    SPISettings _spiSettings;

    // 澹版槑鑾峰彇 UART 鍩哄湴鍧€鐨勮緟鍔╁嚱鏁般€?    uint16_t getBaseAddr(uint8_t uartNum);
    // 澹版槑鍐欏瘎瀛樺櫒鐨勫簳灞傝緟鍔╁嚱鏁般€?    void writeReg(uint8_t uartNum, uint8_t reg, uint8_t value);
    // 澹版槑璇诲瘎瀛樺櫒鐨勫簳灞傝緟鍔╁嚱鏁般€?    uint8_t readReg(uint8_t uartNum, uint8_t reg);
    // 澹版槑璁剧疆娉㈢壒鐜囧垎棰戠殑杈呭姪鍑芥暟銆?    void setBaudrate(uint8_t uartNum, uint32_t baudrate);
// 中文逐行说明：下面这一行保留原始代码 -> };
};

// 缁撴潫澶存枃浠朵繚鎶ゃ€?#endif // CH9434A_H
