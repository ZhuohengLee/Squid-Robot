/**********************************************************************
 * SensorDiag6.ino  —  IMU <ver> 握手定波特率 + 强制 ASCII 输出
 *
 * 独立 sketch，不依赖/不修改主固件。
 *
 * 依据 EBIMU-9DOFV6 datasheet:
 *  - <ver> 返回固定字符串 "<imu9dofv600>"（不发 <ok>）-> 完美握手/定波特率
 *  - 输出模式由 <soc> 控制: soc1=ASCII(*), soc2=HEX(binary, 5555 包)，存非易失
 *  - 主固件配置序列漏发 <soc1>，若 IMU 存档为二进制模式则永远收不到 ASCII '*' 帧
 *  - <lf> = 恢复出厂(115200/ASCII/...)
 *  - 默认 115200 8N1，波特率范围 9600~921600
 *
 * 流程:
 *  1) 逐波特率发 <ver>，找到回 "imu9dofv6" 的波特率 = 真实波特率
 *  2) 在该波特率发 <stop><soc1><sof1><sor20><pons1><start> 强制 ASCII 欧拉输出
 *  3) 持续显示: ASCII '*' 帧计数 + 最近原始 hex(便于看 5555 二进制包)
 *
 * 引脚: CH9434A SPI MOSI=11 MISO=13 SCK=12 CS=10 INT=9; IMU=U3
 * 命令(115200): s=重新扫描  l=发<lf>恢复出厂(在已选波特率)  a=重发ASCII配置
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
static uint32_t baud = 0;

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
static void chSetBaud(uint8_t u, uint32_t b) {
    const uint16_t div = CH_CLOCK / (8 * b);
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

// 在某波特率发 <ver>，捕获回复，返回是否含 "imu"
static bool probeVer(uint32_t b, uint8_t* buf, uint8_t* n) {
    chSetBaud(IMU_UART, b);
    chFlush(IMU_UART);
    delay(15);
    chPuts(IMU_UART, "<ver>");
    uint8_t cnt = 0;
    const uint32_t t0 = millis();
    while (millis() - t0 < 250 && cnt < 48)
        if (chAvail(IMU_UART)) buf[cnt++] = chR(IMU_UART, REG_RHR);
    *n = cnt;
    // 检测子串 "imu"
    for (uint8_t i = 0; i + 2 < cnt; i++)
        if (buf[i] == 'i' && buf[i+1] == 'm' && buf[i+2] == 'u') return true;
    return false;
}

static void scan() {
    Serial.println(F("\n===== IMU <ver> 波特率握手扫描 ====="));
    baud = 0;
    uint8_t bestAscii = 0; uint32_t bestB = 0;
    for (uint8_t i = 0; i < NCAND; i++) {
        uint8_t buf[48], n = 0;
        const bool hit = probeVer(CAND[i], buf, &n);
        uint8_t pr = 0;
        for (uint8_t j = 0; j < n; j++) if (buf[j] >= 32 && buf[j] < 127) pr++;
        const uint8_t pct = n ? (uint8_t)((uint16_t)pr * 100 / n) : 0;
        Serial.printf("  %6lu: %2u bytes %3u%%ascii |", (unsigned long)CAND[i], n, pct);
        for (uint8_t j = 0; j < n && j < 16; j++) Serial.printf(" %02X", buf[j]);
        Serial.print(F("  \""));
        for (uint8_t j = 0; j < n && j < 16; j++)
            Serial.print((buf[j] >= 32 && buf[j] < 127) ? (char)buf[j] : '.');
        Serial.print(F("\""));
        if (hit) { Serial.print(F("  <== imu!")); baud = CAND[i]; }
        Serial.println();
        if (!baud && pct > bestAscii) { bestAscii = pct; bestB = CAND[i]; }
    }
    Serial.println(F("===================================="));
    if (baud) {
        Serial.printf(">>> IMU 真实波特率 = %lu (收到 <ver> 含 imu)\n", (unsigned long)baud);
    } else {
        baud = bestB;
        Serial.printf(">>> 未收到清晰 <ver>。取最高 ascii%% 波特率 %lu 试配置。\n",
                      (unsigned long)bestB);
    }
}

static void configAscii() {
    if (!baud) return;
    Serial.printf("\n用 %lu 强制 ASCII 欧拉输出 (含 <soc1>)...\n", (unsigned long)baud);
    chSetBaud(IMU_UART, baud);
    const char* seq[] = {"<stop>","<soc1>","<sof1>","<sog0>","<soa0>","<som0>",
                         "<sod0>","<sot0>","<sots0>","<sor20>","<pons1>","<start>"};
    for (uint8_t i = 0; i < 12; i++) { chPuts(IMU_UART, seq[i]); delay(80); }
    chFlush(IMU_UART);
}

static char line[128]; static uint8_t llen = 0;
static uint32_t samples = 0; static char last[128];
static uint8_t raw[24]; static uint8_t rlen = 0;
static uint32_t rep = 0;

void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println(F("\n##### SensorDiag6: IMU ver握手 + 强制ASCII #####"));
    pinMode(PIN_CS, OUTPUT); digitalWrite(PIN_CS, HIGH);
    pinMode(PIN_INT, INPUT_PULLUP);
    SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);
    spiSettings = SPISettings(10000000, MSBFIRST, SPI_MODE0);
    delay(100);
    scan();
    configAscii();
    Serial.println(F("命令: s=重扫  l=<lf>恢复出厂  a=重发ASCII配置\n"));
}

void loop() {
    if (Serial.available()) {
        const char c = Serial.read();
        if (c == 's') { samples = 0; last[0] = '\0'; scan(); configAscii(); }
        else if (c == 'a') { samples = 0; last[0] = '\0'; configAscii(); }
        else if (c == 'l' && baud) {
            Serial.println(F(">>> 发 <lf> 恢复出厂(默认115200/ASCII)..."));
            chSetBaud(IMU_UART, baud); chPuts(IMU_UART, "<lf>"); delay(300);
            baud = 115200; samples = 0; last[0] = '\0'; configAscii();
        }
    }

    if (baud) {
        while (chAvail(IMU_UART)) {
            const uint8_t c = chR(IMU_UART, REG_RHR);
            raw[rlen % sizeof(raw)] = c; rlen++;
            if (c == '\r') continue;
            if (c == '\n') {
                line[llen] = '\0';
                if (llen > 0) { strncpy(last, line, sizeof(last)); if (line[0] == '*') samples++; }
                llen = 0; continue;
            }
            if (llen < sizeof(line) - 1) line[llen++] = (char)c; else llen = 0;
        }
        const uint32_t now = millis();
        if (now - rep >= 1000) {
            rep = now;
            Serial.printf("[%lus] *帧=%lu  last=\"%s\"  原始hex:",
                          now / 1000, (unsigned long)samples, last);
            const uint8_t show = rlen < sizeof(raw) ? rlen : sizeof(raw);
            for (uint8_t i = 0; i < show; i++) Serial.printf(" %02X", raw[i]);
            Serial.println();
        }
    }
}
