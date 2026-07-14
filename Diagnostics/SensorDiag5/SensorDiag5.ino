/**********************************************************************
 * SensorDiag5.ino  —  IMU 主动配置+启动 逐波特率扫描
 *
 * 独立 sketch，不依赖/不修改主固件。
 *
 * 背景: IMU 收发通、没接反，但 <stop> 在所有标准波特率都只回乱码。
 *       超声波在 115200 解出过合法校验帧 -> CH9434A 115200 是准的。
 *       => IMU 真实波特率非 115200，或被设成二进制输出。
 *
 * 方法: 对每个候选波特率，主动发 <sof1>(ASCII欧拉)+<sor20>+<start>，
 *       监听 ~1.3s，统计收到的整行数 和 以 '*' 开头的合法姿态帧数。
 *       只有 TX 波特率对上，IMU 才会接受命令并以该波特率回流，
 *       届时能解出干净的 *roll,pitch,yaw。哪个波特率出 '*' 帧即真实波特率。
 *
 * 引脚: CH9434A SPI MOSI=11 MISO=13 SCK=12 CS=10 INT=9; IMU=U3
 * 命令(115200): s=重新扫描
 *********************************************************************/

#include <Arduino.h>
#include <SPI.h>

static const int PIN_MOSI = 11, PIN_MISO = 13, PIN_SCK = 12, PIN_CS = 10, PIN_INT = 9;
static const uint8_t REG_THR = 0x00, REG_RHR = 0x00, REG_DLL = 0x00;
static const uint8_t REG_IER = 0x01, REG_DLM = 0x01, REG_FCR = 0x02;
static const uint8_t REG_LCR = 0x03, REG_LSR = 0x05;
static const uint8_t LCR_DLAB = 0x80, LCR_8N1 = 0x03, FCR_INIT = 0x07;
static const uint8_t LSR_DR = 0x01, LSR_THRE = 0x20;
static const uint8_t IMU_UART = 3, UART_STEP = 0x10;
static const uint32_t CH_CLOCK = 32000000;
static SPISettings spiSettings;

static const uint32_t CAND[] = {9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600};
static const uint8_t  NCAND = sizeof(CAND) / sizeof(CAND[0]);
static uint32_t goodBaud = 0;

static void chW(uint8_t u, uint8_t reg, uint8_t v) {
    const uint8_t a = reg + UART_STEP * u;
    SPI.beginTransaction(spiSettings);
    digitalWrite(PIN_CS, LOW);
    SPI.transfer(0x80 | a); delayMicroseconds(1);
    SPI.transfer(v); delayMicroseconds(3);
    digitalWrite(PIN_CS, HIGH);
    SPI.endTransaction();
}
static uint8_t chR(uint8_t u, uint8_t reg) {
    const uint8_t a = reg + UART_STEP * u;
    SPI.beginTransaction(spiSettings);
    digitalWrite(PIN_CS, LOW);
    SPI.transfer(a); delayMicroseconds(3);
    const uint8_t v = SPI.transfer(0xFF); delayMicroseconds(1);
    digitalWrite(PIN_CS, HIGH);
    SPI.endTransaction();
    return v;
}
static void chSetBaud(uint8_t u, uint32_t baud) {
    const uint16_t div = CH_CLOCK / (8 * baud);
    chW(u, REG_LCR, LCR_8N1 | LCR_DLAB);
    chW(u, REG_DLL, div & 0xFF);
    chW(u, REG_DLM, (div >> 8) & 0xFF);
    chW(u, REG_LCR, LCR_8N1);
    chW(u, REG_FCR, FCR_INIT);
    chW(u, REG_IER, 0x00);
}
static void chFlush(uint8_t u) { chW(u, REG_FCR, FCR_INIT); }
static bool chAvail(uint8_t u) { return chR(u, REG_LSR) & LSR_DR; }
static void chPutc(uint8_t u, uint8_t c) {
    const uint32_t t0 = millis();
    while (!(chR(u, REG_LSR) & LSR_THRE)) if (millis() - t0 > 50) break;
    chW(u, REG_THR, c);
}
static void chPuts(uint8_t u, const char* s) { while (*s) chPutc(u, (uint8_t)*s++); }

// 在 baud 下主动启动并监听，返回 '*' 帧数；sample 存一条样本行
static uint16_t tryBaud(uint32_t baud, uint16_t* totalLines, char* sample, size_t sampSz) {
    chSetBaud(IMU_UART, baud);
    chFlush(IMU_UART);
    delay(10);
    chPuts(IMU_UART, "<stop>");  delay(60);
    chFlush(IMU_UART);
    chPuts(IMU_UART, "<sof1>");  delay(60);   // ASCII 欧拉角
    chPuts(IMU_UART, "<sor20>"); delay(60);   // 50Hz
    chPuts(IMU_UART, "<start>"); delay(60);   // 启动
    chFlush(IMU_UART);

    char line[96]; uint8_t len = 0;
    uint16_t lines = 0, star = 0;
    const uint32_t t0 = millis();
    while (millis() - t0 < 1300) {
        if (!chAvail(IMU_UART)) continue;
        const uint8_t c = chR(IMU_UART, REG_RHR);
        if (c == '\r') continue;
        if (c == '\n') {
            line[len] = '\0';
            if (len > 0) {
                lines++;
                if (line[0] == '*') { star++; if (star == 1) strncpy(sample, line, sampSz); }
            }
            len = 0;
            continue;
        }
        if (len < sizeof(line) - 1) line[len++] = (char)c; else len = 0;
    }
    *totalLines = lines;
    return star;
}

static void scan() {
    Serial.println(F("\n===== IMU 主动启动逐波特率扫描 ====="));
    goodBaud = 0; uint16_t bestStar = 0;
    for (uint8_t i = 0; i < NCAND; i++) {
        char sample[96] = {0}; uint16_t lines = 0;
        const uint16_t star = tryBaud(CAND[i], &lines, sample, sizeof(sample));
        Serial.printf("  %6lu: 行=%u  *帧=%u", (unsigned long)CAND[i], lines, star);
        if (star > 0) Serial.printf("  样本: %s", sample);
        Serial.println();
        if (star > bestStar) { bestStar = star; goodBaud = CAND[i]; }
    }
    Serial.println(F("==================================="));
    if (goodBaud && bestStar >= 3) {
        Serial.printf(">>> 找到! IMU 真实波特率 = %lu，持续输出中...\n", (unsigned long)goodBaud);
        chSetBaud(IMU_UART, goodBaud);
        chPuts(IMU_UART, "<start>"); chFlush(IMU_UART);
    } else {
        Serial.println(F(">>> 仍无任何波特率解出 * 帧。"));
        Serial.println(F("    -> 不是波特率问题。可能: IMU 被设成二进制输出 / IMU 故障 /"));
        Serial.println(F("       5V 供电损坏了 3.3V 逻辑 / TX 线信号异常。"));
    }
}

static char line[96]; static uint8_t len = 0;
static uint32_t samples = 0; static char last[96];
static uint32_t lastReport = 0;

void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println(F("\n##### SensorDiag5: IMU 主动启动扫描 #####"));
    pinMode(PIN_CS, OUTPUT); digitalWrite(PIN_CS, HIGH);
    pinMode(PIN_INT, INPUT_PULLUP);
    SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);
    spiSettings = SPISettings(10000000, MSBFIRST, SPI_MODE0);
    delay(100);
    scan();
    Serial.println(F("命令: s=重新扫描\n"));
}

void loop() {
    if (Serial.available() && Serial.read() == 's') { samples = 0; last[0] = '\0'; scan(); }
    if (goodBaud) {
        while (chAvail(IMU_UART)) {
            const uint8_t c = chR(IMU_UART, REG_RHR);
            if (c == '\r') continue;
            if (c == '\n') { line[len] = '\0'; if (len > 0) { strncpy(last, line, sizeof(last)); if (line[0] == '*') samples++; } len = 0; continue; }
            if (len < sizeof(line) - 1) line[len++] = (char)c; else len = 0;
        }
        const uint32_t now = millis();
        if (now - lastReport >= 1000) {
            lastReport = now;
            Serial.printf("[%lus] samples=%lu last=\"%s\"\n", now / 1000, (unsigned long)samples, last);
        }
    }
}
