/**********************************************************************
 * SensorDiag4.ino  —  IMU 波特率自动扫描 + 流输出
 *
 * 独立 sketch，不依赖/不修改主固件。
 *
 * 背景: SensorDiag3 证明 IMU 收发都通(回了字节)、没接反，但 @115200
 *       回的是乱码 FB 7D 1B 00 而非 <ok> -> 典型波特率不匹配。
 *
 * 本 sketch: 逐个波特率发 <stop>，看哪个能收到干净 ASCII <ok>，
 *            判定 IMU 真实波特率；找到后用它配置(<sof1>..<start>)并
 *            持续输出姿态 *roll,pitch,yaw。
 *
 * 引脚: CH9434A SPI MOSI=11 MISO=13 SCK=12 CS=10 INT=9; IMU=U3
 * 命令(115200): s=重新扫描   b=只在已选波特率重新配置+输出
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
static uint32_t chosenBaud = 0;

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

// 在给定波特率发 <stop>，统计回复中可打印 ASCII 比例
static uint8_t probeBaud(uint32_t baud, uint8_t* outBuf, uint8_t* outN) {
    chSetBaud(IMU_UART, baud);
    chFlush(IMU_UART);
    delay(20);
    chPuts(IMU_UART, "<stop>");
    uint8_t n = 0, printable = 0;
    const uint32_t t0 = millis();
    while (millis() - t0 < 300 && n < 64) {
        if (chAvail(IMU_UART)) {
            const uint8_t b = chR(IMU_UART, REG_RHR);
            if ((b >= 32 && b < 127) || b == '\r' || b == '\n') printable++;
            outBuf[n++] = b;
        }
    }
    *outN = n;
    return n == 0 ? 0 : (uint8_t)((uint16_t)printable * 100 / n);  // 可打印百分比
}

static void scanBauds() {
    Serial.println(F("\n========= IMU BAUD SCAN ========="));
    uint32_t best = 0; uint8_t bestScore = 0; uint8_t bestN = 0;
    for (uint8_t i = 0; i < NCAND; i++) {
        uint8_t buf[64], n = 0;
        const uint8_t score = probeBaud(CAND[i], buf, &n);
        Serial.printf("  %6lu: %u bytes, %u%% ascii  |", (unsigned long)CAND[i], n, score);
        for (uint8_t j = 0; j < n && j < 12; j++) Serial.printf(" %02X", buf[j]);
        Serial.print(F("  \""));
        for (uint8_t j = 0; j < n && j < 12; j++)
            Serial.print((buf[j] >= 32 && buf[j] < 127) ? (char)buf[j] : '.');
        Serial.println(F("\""));
        // 评分：优先包含 'o''k' 或高可打印率
        bool hasOk = false;
        for (uint8_t j = 0; j + 1 < n; j++) if (buf[j] == 'o' && buf[j+1] == 'k') hasOk = true;
        const uint8_t eff = hasOk ? 100 : score;
        if (n > 0 && eff > bestScore) { bestScore = eff; best = CAND[i]; bestN = n; }
    }
    Serial.println(F("================================="));
    if (best && bestScore >= 60) {
        chosenBaud = best;
        Serial.printf(">>> 选定 IMU 波特率 = %lu (ascii %u%%)\n", (unsigned long)best, bestScore);
    } else {
        chosenBaud = 0;
        Serial.println(F(">>> 未找到干净 ASCII 的波特率。可能: 二进制输出模式/非标波特率/"));
        Serial.println(F("    CH9434A 分频在高波特率下不准。最高分: "));
        Serial.printf("    %lu (%u%%, %u bytes)\n", (unsigned long)best, bestScore, bestN);
    }
}

static char  line[96]; static uint8_t lineLen = 0;
static uint32_t samples = 0, parseErr = 0; static char last[96];

static void configureAndStart() {
    if (!chosenBaud) { Serial.println(F("无选定波特率，跳过配置。")); return; }
    Serial.printf("\n用 %lu 配置 IMU 并启动输出...\n", (unsigned long)chosenBaud);
    chSetBaud(IMU_UART, chosenBaud);
    const char* seq[] = {"<sof1>","<sog0>","<soa0>","<som0>","<sod0>",
                         "<sot0>","<sots0>","<sor20>","<pons1>","<start>"};
    for (uint8_t i = 0; i < 10; i++) { chPuts(IMU_UART, seq[i]); delay(100); }
    chFlush(IMU_UART);
    lineLen = 0; samples = 0; parseErr = 0; last[0] = '\0';
}

static void imuPoll() {
    while (chAvail(IMU_UART)) {
        const uint8_t c = chR(IMU_UART, REG_RHR);
        if (c == '\r') continue;
        if (c == '\n') {
            line[lineLen] = '\0';
            if (lineLen > 0) { strncpy(last, line, sizeof(last)); if (line[0] == '*') samples++; else parseErr++; }
            lineLen = 0; continue;
        }
        if (lineLen < sizeof(line) - 1) line[lineLen++] = (char)c; else lineLen = 0;
    }
}

static uint32_t lastReport = 0;

void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println(F("\n##### SensorDiag4: IMU 波特率扫描 #####"));
    pinMode(PIN_CS, OUTPUT); digitalWrite(PIN_CS, HIGH);
    pinMode(PIN_INT, INPUT_PULLUP);
    SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);
    spiSettings = SPISettings(10000000, MSBFIRST, SPI_MODE0);
    delay(100);
    scanBauds();
    configureAndStart();
    Serial.println(F("\n命令: s=重新扫描  b=重新配置+输出\n"));
}

void loop() {
    if (Serial.available()) {
        const char c = Serial.read();
        if (c == 's') { scanBauds(); configureAndStart(); }
        else if (c == 'b') configureAndStart();
    }
    imuPoll();
    const uint32_t now = millis();
    if (now - lastReport >= 1000) {
        lastReport = now;
        Serial.printf("[%lus] IMU samples=%lu parse_err=%lu last=\"%s\"\n",
                      now / 1000, (unsigned long)samples, (unsigned long)parseErr, last);
    }
}
