/**********************************************************************
 * SensorDiag3.ino  —  主动驱动测试（超声波触发 + IMU 命令握手）
 *
 * 独立 sketch，不依赖/不修改主固件。复刻主固件协议主动驱动传感器，
 * 才能判断它们到底工不工作（被动监听因为没发命令，无法判定）。
 *
 * 协议来源：UltrasonicManager.cpp / ImuManager.cpp
 *   超声波: 发 0x00 触发 -> 回 4 字节帧 [FF, dH, dL, cs], cs=(FF+dH+dL)&0xFF
 *           距离mm = (dH<<8)|dL；桌面无目标时回超量程(~0xFFFD)。
 *   IMU(EBIMU-9DOFV6): 发 <stop><sof1>...<start> 配置后输出 ASCII
 *           `*roll,pitch,yaw\r\n`；每条命令 IMU 会回 <ok>。
 *
 * 引脚: CH9434A SPI MOSI=11 MISO=13 SCK=12 CS=10 INT=9
 *       U0=右超声 U1=前超声 U2=左超声 U3=IMU
 *
 * 串口命令(115200):
 *   h = 重新做 IMU 握手(发<stop>看是否回<ok>)
 *   1=115200 2=57600 3=9600 4=19200 5=38400  (改 IMU 波特率并重握手)
 *********************************************************************/

#include <Arduino.h>
#include <SPI.h>

static const int PIN_MOSI = 11, PIN_MISO = 13, PIN_SCK = 12, PIN_CS = 10, PIN_INT = 9;

static const uint8_t REG_RHR = 0x00, REG_THR = 0x00, REG_DLL = 0x00;
static const uint8_t REG_IER = 0x01, REG_DLM = 0x01, REG_FCR = 0x02;
static const uint8_t REG_LCR = 0x03, REG_LSR = 0x05;
static const uint8_t LCR_DLAB = 0x80, LCR_8N1 = 0x03, FCR_INIT = 0x07;
static const uint8_t LSR_DR = 0x01, LSR_THRE = 0x20;
static const uint8_t UART_STEP = 0x10;
static const uint32_t CH_CLOCK = 32000000;
static SPISettings spiSettings;

static const uint8_t US_UART[3] = {1, 2, 0};          // 前, 左, 右
static const char*   US_NAME[3] = {"前", "左", "右"};
static const uint8_t IMU_UART = 3;
static uint32_t imuBaud = 115200;

// ── CH9434A 最小读写 ─────────────────────────────────────────────
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
static void chConfig(uint8_t u, uint32_t baud) {
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

// ── 超声波：触发并读一帧 ─────────────────────────────────────────
static void pingUltrasonic(uint8_t idx) {
    const uint8_t u = US_UART[idx];
    chFlush(u);
    chPutc(u, 0x00);                 // 触发
    const uint32_t t0 = millis();
    uint8_t buf[10]; uint8_t n = 0;
    while (millis() - t0 < 120 && n < sizeof(buf)) {
        if (chAvail(u)) buf[n++] = chR(u, REG_RHR);
        else if (n >= 4) break;
    }
    Serial.printf("  超声%s(U%u): ", US_NAME[idx], u);
    if (n == 0) { Serial.println(F("无响应 (没触发到/没接/波特率?)")); return; }
    Serial.print(F("raw")); for (uint8_t i = 0; i < n; i++) Serial.printf(" %02X", buf[i]);
    // 找 FF 帧头
    int s = -1;
    for (uint8_t i = 0; i + 3 < n; i++) if (buf[i] == 0xFF) { s = i; break; }
    if (s < 0) { Serial.println(F("  -> 无FF帧头")); return; }
    const uint8_t cs = (uint8_t)(buf[s] + buf[s+1] + buf[s+2]);
    if (cs != buf[s+3]) { Serial.println(F("  -> 校验错")); return; }
    const uint16_t mm = (buf[s+1] << 8) | buf[s+2];
    if (mm >= 0xFFF0 || mm > 6000) Serial.println(F("  -> 校验OK, 超量程(探头前空)"));
    else Serial.printf("  -> 距离 %u mm (%.1f cm) ✓\n", mm, mm / 10.0f);
}

// ── IMU：握手 + 配置 ─────────────────────────────────────────────
static void imuSendCmd(const char* cmd) { chPuts(IMU_UART, cmd); delay(100); }

static bool imuHandshake() {
    Serial.printf("\n[IMU] 握手 @%lu: 发 <stop> 等回复...\n", (unsigned long)imuBaud);
    chConfig(IMU_UART, imuBaud);
    chFlush(IMU_UART);
    chPuts(IMU_UART, "<stop>");
    // 收 400ms 回复
    uint8_t buf[64]; uint8_t n = 0;
    const uint32_t t0 = millis();
    while (millis() - t0 < 400 && n < sizeof(buf))
        if (chAvail(IMU_UART)) buf[n++] = chR(IMU_UART, REG_RHR);
    if (n == 0) {
        Serial.println(F("  ✗ IMU 无任何回复 -> TX到IMU不通/IMU没收到/波特率错/没接/坏"));
        Serial.println(F("    (可按 2/3/4/5 换波特率重试)"));
        return false;
    }
    Serial.print(F("  ✓ IMU 有回复! hex:"));
    for (uint8_t i = 0; i < n; i++) Serial.printf(" %02X", buf[i]);
    Serial.print(F("  ascii: "));
    for (uint8_t i = 0; i < n; i++) Serial.print((buf[i] >= 32 && buf[i] < 127) ? (char)buf[i] : '.');
    Serial.println();
    Serial.println(F("  -> 收发双通、波特率对、没接反。下发配置并启动输出..."));
    return true;
}

static void imuConfigure() {
    imuSendCmd("<sof1>"); imuSendCmd("<sog0>"); imuSendCmd("<soa0>");
    imuSendCmd("<som0>"); imuSendCmd("<sod0>"); imuSendCmd("<sot0>");
    imuSendCmd("<sots0>"); imuSendCmd("<sor20>"); imuSendCmd("<pons1>");
    imuSendCmd("<start>");
    chFlush(IMU_UART);
}

static void imuInit() {
    if (imuHandshake()) imuConfigure();
}

// ── IMU 流解析 ───────────────────────────────────────────────────
static char  imuLine[96]; static uint8_t imuLen = 0;
static uint32_t imuSamples = 0, imuParseErr = 0;
static char  imuLast[96];

static void imuPoll() {
    while (chAvail(IMU_UART)) {
        const uint8_t c = chR(IMU_UART, REG_RHR);
        if (c == '\r') continue;
        if (c == '\n') {
            imuLine[imuLen] = '\0';
            if (imuLen > 0) {
                strncpy(imuLast, imuLine, sizeof(imuLast));
                if (imuLine[0] == '*') imuSamples++; else imuParseErr++;
            }
            imuLen = 0;
            continue;
        }
        if (imuLen < sizeof(imuLine) - 1) imuLine[imuLen++] = (char)c;
        else imuLen = 0;
    }
}

// ── 主循环 ───────────────────────────────────────────────────────
static uint32_t lastUs = 0, lastReport = 0;
static uint8_t usIdx = 0;

void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println(F("\n##### SensorDiag3: 主动驱动测试 #####"));
    pinMode(PIN_CS, OUTPUT); digitalWrite(PIN_CS, HIGH);
    pinMode(PIN_INT, INPUT_PULLUP);
    SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);
    spiSettings = SPISettings(10000000, MSBFIRST, SPI_MODE0);
    delay(100);
    for (uint8_t i = 0; i < 3; i++) chConfig(US_UART[i], 115200);
    imuInit();
    Serial.println(F("\n开始: 每秒触发超声波一轮 + 显示IMU。手遮挡超声探头看距离变化。"));
    Serial.println(F("命令: h=IMU重握手  1/2/3/4/5=IMU换波特率\n"));
}

void loop() {
    if (Serial.available()) {
        const char c = Serial.read();
        bool re = true;
        switch (c) {
            case '1': imuBaud = 115200; break;
            case '2': imuBaud = 57600;  break;
            case '3': imuBaud = 9600;   break;
            case '4': imuBaud = 19200;  break;
            case '5': imuBaud = 38400;  break;
            case 'h': break;
            default: re = false; break;
        }
        if (re) { imuSamples = 0; imuParseErr = 0; imuLast[0] = '\0'; imuInit(); }
    }

    imuPoll();

    // 每秒一轮：触发 3 路超声波 + 报告 IMU
    const uint32_t now = millis();
    if (now - lastReport >= 1000) {
        lastReport = now;
        Serial.printf("---- [%lus] ----\n", now / 1000);
        for (uint8_t i = 0; i < 3; i++) { pingUltrasonic(i); imuPoll(); }
        Serial.printf("  IMU: samples=%lu parse_err=%lu  last=\"%s\"\n",
                      (unsigned long)imuSamples, (unsigned long)imuParseErr, imuLast);
    }
}
