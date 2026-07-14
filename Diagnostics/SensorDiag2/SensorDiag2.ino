/**********************************************************************
 * SensorDiag2.ino  —  CH9434A 4 路 UART 实时监听（查 TX/RX 接反）
 *
 * 独立 sketch，不依赖、不修改主固件。烧录会临时覆盖主固件。
 *
 * 用途：CH9434A 已确认是好的(SensorDiag 测过 SPI 48/48、收到过真实字节)。
 *       本 sketch 实时逐秒打印 4 路 UART 收到的字节数和 hex 内容，
 *       用来验证「串口接反 / 断线」假设：
 *         - 把某个传感器的 TX/RX 两根线对调，盯着对应通道字节数，
 *           若从 0 跳到持续有数 -> 之前就是接反了。
 *
 * 引脚(对齐 Protocol.h):
 *   CH9434A SPI: MOSI=11 MISO=13 SCK=12 CS=10 INT=9
 *   U0=右超声  U1=前超声  U2=左超声  U3=IMU(EBIMU-9DOFV6)
 *   传感器供电 5V(已确认)。注意 CH9434A 为 3.3V 器件，5V TTL 直连 RX 偏高，
 *   通常能收但电平偏冒险——若仍异常需确认是否过电平。
 *
 * 串口命令(115200):
 *   1=115200  2=57600  3=9600  4=19200  5=38400   (对全部 4 路同时切波特率)
 *   r = 计数清零
 *********************************************************************/

#include <Arduino.h>
#include <SPI.h>

static const int PIN_MOSI = 11, PIN_MISO = 13, PIN_SCK = 12, PIN_CS = 10, PIN_INT = 9;

static const uint8_t REG_RHR = 0x00, REG_DLL = 0x00, REG_IER = 0x01, REG_DLM = 0x01;
static const uint8_t REG_FCR = 0x02, REG_LCR = 0x03, REG_LSR = 0x05;
static const uint8_t LCR_DLAB = 0x80, LCR_8N1 = 0x03, FCR_INIT = 0x07, LSR_DR = 0x01;
static const uint8_t NUM_UARTS = 4, UART_STEP = 0x10;
static const uint32_t CH_CLOCK = 32000000;
static SPISettings spiSettings;

static const char* NAMES[NUM_UARTS] = {"U0(右超)", "U1(前超)", "U2(左超)", "U3(IMU)"};
static uint32_t curBaud = 115200;

static void chWrite(uint8_t u, uint8_t reg, uint8_t val) {
    const uint8_t addr = reg + UART_STEP * u;
    SPI.beginTransaction(spiSettings);
    digitalWrite(PIN_CS, LOW);
    SPI.transfer(0x80 | addr); delayMicroseconds(1);
    SPI.transfer(val); delayMicroseconds(3);
    digitalWrite(PIN_CS, HIGH);
    SPI.endTransaction();
}
static uint8_t chRead(uint8_t u, uint8_t reg) {
    const uint8_t addr = reg + UART_STEP * u;
    SPI.beginTransaction(spiSettings);
    digitalWrite(PIN_CS, LOW);
    SPI.transfer(addr); delayMicroseconds(3);
    const uint8_t v = SPI.transfer(0xFF); delayMicroseconds(1);
    digitalWrite(PIN_CS, HIGH);
    SPI.endTransaction();
    return v;
}
static void chConfig(uint8_t u, uint32_t baud) {
    const uint16_t div = CH_CLOCK / (8 * baud);
    chWrite(u, REG_LCR, LCR_8N1 | LCR_DLAB);
    chWrite(u, REG_DLL, div & 0xFF);
    chWrite(u, REG_DLM, (div >> 8) & 0xFF);
    chWrite(u, REG_LCR, LCR_8N1);
    chWrite(u, REG_FCR, FCR_INIT);
    chWrite(u, REG_IER, 0x00);
}

static uint32_t total[NUM_UARTS] = {0, 0, 0, 0};
static uint32_t lastSec[NUM_UARTS] = {0, 0, 0, 0};
static uint8_t  head[NUM_UARTS][12];
static uint8_t  headLen[NUM_UARTS] = {0, 0, 0, 0};
static uint32_t winStart = 0;

static void configAll() {
    for (uint8_t u = 0; u < NUM_UARTS; u++) chConfig(u, curBaud);
    Serial.printf("\n>>> 全部 4 路波特率设为 %lu，开始监听...\n", (unsigned long)curBaud);
    Serial.println(F("    把可疑传感器 TX/RX 对调，看对应通道字节数是否从 0 跳起。"));
}

void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println(F("\n##### SensorDiag2: CH9434A UART live monitor #####"));
    pinMode(PIN_CS, OUTPUT); digitalWrite(PIN_CS, HIGH);
    pinMode(PIN_INT, INPUT_PULLUP);
    SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);
    spiSettings = SPISettings(10000000, MSBFIRST, SPI_MODE0);
    delay(100);
    configAll();
    Serial.println(F("命令: 1=115200 2=57600 3=9600 4=19200 5=38400  r=清零\n"));
}

void loop() {
    // 处理命令
    if (Serial.available()) {
        const char c = Serial.read();
        bool change = true;
        switch (c) {
            case '1': curBaud = 115200; break;
            case '2': curBaud = 57600;  break;
            case '3': curBaud = 9600;   break;
            case '4': curBaud = 19200;  break;
            case '5': curBaud = 38400;  break;
            case 'r':
                for (uint8_t u = 0; u < NUM_UARTS; u++) { total[u] = 0; headLen[u] = 0; }
                Serial.println(F(">>> 计数清零"));
                change = false; break;
            default: change = false; break;
        }
        if (change) {
            for (uint8_t u = 0; u < NUM_UARTS; u++) { total[u] = 0; headLen[u] = 0; }
            configAll();
        }
    }

    // 持续抽取各路 RX FIFO
    for (uint8_t u = 0; u < NUM_UARTS; u++) {
        while (chRead(u, REG_LSR) & LSR_DR) {
            const uint8_t b = chRead(u, REG_RHR);
            total[u]++; lastSec[u]++;
            if (headLen[u] < sizeof(head[u])) head[u][headLen[u]++] = b;
        }
    }

    // 每秒汇报一次
    const uint32_t now = millis();
    if (now - winStart >= 1000) {
        winStart = now;
        Serial.print(F("["));
        Serial.print(now / 1000);
        Serial.print(F("s] "));
        for (uint8_t u = 0; u < NUM_UARTS; u++) {
            Serial.printf("%s:%lu/s(tot%lu) ", NAMES[u],
                          (unsigned long)lastSec[u], (unsigned long)total[u]);
            lastSec[u] = 0;
        }
        Serial.println();
        // 有数据的通道顺带打印一小段 hex，便于核对帧格式
        for (uint8_t u = 0; u < NUM_UARTS; u++) {
            if (headLen[u] > 0) {
                Serial.printf("      %s head:", NAMES[u]);
                for (uint8_t i = 0; i < headLen[u]; i++) Serial.printf(" %02X", head[u][i]);
                Serial.println();
            }
        }
    }
}
