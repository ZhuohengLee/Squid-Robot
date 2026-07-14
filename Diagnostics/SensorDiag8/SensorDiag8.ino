/**********************************************************************
 * SensorDiag8.ino  —  深度详细诊断 + IMU 实时姿态(修正检测)
 *
 * 独立 sketch，不依赖/不修改主固件。
 *
 * 现状: IMU TX/RX 对调后已在 115200 输出 *roll,pitch,yaw (SensorDiag7 验证)。
 *       深度 MS5837 换 3.3V 后仍 reset 无应答。
 *
 * 本 sketch:
 *   [深度] I2C 全地址扫描 + 打印每个地址 endTransmission 错误码 +
 *          0x76 reset 错误码。err=2=地址NACK(总线OK,器件无应答/坏);
 *          err=4=总线错误(上拉/接线)。借此判断"坏了"还是"接线"。
 *   [IMU]  锁 115200 配置 ASCII 并持续显示姿态角(检测改为认 '*' 帧)。
 *
 * 引脚: 深度 I2C SDA=4 SCL=5; CH9434A SPI 10-13,INT9; IMU=U3
 *********************************************************************/

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

static const int DEPTH_SDA = 4, DEPTH_SCL = 5;

static void depthScan() {
    Serial.println(F("\n[深度] I2C 扫描 SDA=IO4 SCL=IO5 @100kHz:"));
    Wire.begin(DEPTH_SDA, DEPTH_SCL, 100000);
    Wire.setTimeOut(50);
    uint8_t found = 0;
    for (uint8_t a = 0x08; a <= 0x77; a++) {
        Wire.beginTransmission(a);
        if (Wire.endTransmission() == 0) { Serial.printf("    设备 @0x%02X\n", a); found++; }
    }
    Wire.beginTransmission(0x76);
    const uint8_t e76 = Wire.endTransmission();
    Wire.beginTransmission(0x77);
    const uint8_t e77 = Wire.endTransmission();
    Serial.printf("    found=%u | probe 0x76 err=%u | 0x77 err=%u\n", found, e76, e77);
    Serial.println(F("    err=0 ACK | err=2 地址NACK(总线OK,器件无应答/坏)"));
    Serial.println(F("    err=4 总线错(上拉/SDA-SCL断或短) | err=5 超时"));
    // 尝试 reset 0x76
    Wire.beginTransmission(0x76); Wire.write(0x1E);
    Serial.printf("    reset 0x76 -> err=%u %s\n", (unsigned)Wire.endTransmission(),
                  found ? "" : "(无器件)");
}

// ===== CH9434A / IMU =====
static const int PIN_MOSI = 11, PIN_MISO = 13, PIN_SCK = 12, PIN_CS = 10, PIN_INT = 9;
static const uint8_t REG_THR = 0x00, REG_RHR = 0x00, REG_DLL = 0x00;
static const uint8_t REG_IER = 0x01, REG_DLM = 0x01, REG_FCR = 0x02;
static const uint8_t REG_LCR = 0x03, REG_LSR = 0x05;
static const uint8_t LCR_DLAB = 0x80, LCR_8N1 = 0x03, FCR_INIT = 0x07;
static const uint8_t LSR_DR = 0x01, LSR_THRE = 0x20;
static const uint8_t IMU_UART = 3, UART_STEP = 0x10;
static const uint32_t CH_CLOCK = 32000000;
static SPISettings spiSettings;
static uint32_t imuBaud = 0;

static void chW(uint8_t u, uint8_t r, uint8_t v) {
    SPI.beginTransaction(spiSettings); digitalWrite(PIN_CS, LOW);
    SPI.transfer(0x80 | (r + UART_STEP * u)); delayMicroseconds(1);
    SPI.transfer(v); delayMicroseconds(3); digitalWrite(PIN_CS, HIGH); SPI.endTransaction();
}
static uint8_t chRr(uint8_t u, uint8_t r) {
    SPI.beginTransaction(spiSettings); digitalWrite(PIN_CS, LOW);
    SPI.transfer(r + UART_STEP * u); delayMicroseconds(3);
    const uint8_t v = SPI.transfer(0xFF); delayMicroseconds(1);
    digitalWrite(PIN_CS, HIGH); SPI.endTransaction(); return v;
}
static void chSetBaud(uint8_t u, uint32_t b) {
    const uint16_t d = CH_CLOCK / (8 * b);
    chW(u, REG_LCR, LCR_8N1 | LCR_DLAB); chW(u, REG_DLL, d & 0xFF);
    chW(u, REG_DLM, (d >> 8) & 0xFF); chW(u, REG_LCR, LCR_8N1);
    chW(u, REG_FCR, FCR_INIT); chW(u, REG_IER, 0x00);
}
static void chFlush(uint8_t u) { chW(u, REG_FCR, FCR_INIT); }
static bool chAvail(uint8_t u) { return chRr(u, REG_LSR) & LSR_DR; }
static void chPutc(uint8_t u, uint8_t c) {
    const uint32_t t0 = millis();
    while (!(chRr(u, REG_LSR) & LSR_THRE)) if (millis() - t0 > 50) break;
    chW(u, REG_THR, c);
}
static void chPuts(uint8_t u, const char* s) { while (*s) chPutc(u, (uint8_t)*s++); }

// 在 baud 配置 ASCII 并数 1s 内 '*' 帧
static uint16_t tryImu(uint32_t b, char* sample, size_t sz) {
    chSetBaud(IMU_UART, b); chFlush(IMU_UART);
    chPuts(IMU_UART, "<stop>"); delay(60); chFlush(IMU_UART);
    const char* seq[] = {"<soc1>","<sof1>","<sor20>","<start>"};
    for (uint8_t i = 0; i < 4; i++) { chPuts(IMU_UART, seq[i]); delay(60); }
    chFlush(IMU_UART);
    char ln[96]; uint8_t n = 0; uint16_t star = 0;
    const uint32_t t0 = millis();
    while (millis() - t0 < 1000) {
        if (!chAvail(IMU_UART)) continue;
        const uint8_t c = chRr(IMU_UART, REG_RHR);
        if (c == '\r') continue;
        if (c == '\n') { ln[n] = '\0'; if (n > 0 && ln[0] == '*') { star++; if (star == 1) strncpy(sample, ln, sz); } n = 0; continue; }
        if (n < 95) ln[n++] = (char)c; else n = 0;
    }
    return star;
}

static void imuStart() {
    Serial.println(F("\n[IMU] 锁定波特率(认 '*' 帧):"));
    uint32_t best = 0; uint16_t bestStar = 0; char sample[96] = {0};
    const uint32_t cand[] = {115200, 9600, 19200, 38400, 57600, 230400};
    for (uint8_t i = 0; i < 6; i++) {
        char s[96] = {0};
        const uint16_t st = tryImu(cand[i], s, sizeof(s));
        Serial.printf("    %6lu: *帧=%u %s\n", (unsigned long)cand[i], st, st ? s : "");
        if (st > bestStar) { bestStar = st; best = cand[i]; strncpy(sample, s, sizeof(sample)); }
        if (st >= 5) break;
    }
    if (bestStar >= 3) { imuBaud = best; Serial.printf("    -> IMU OK @%lu, 样本 %s\n", (unsigned long)best, sample); }
    else Serial.println(F("    -> IMU 未出 * 帧"));
}

static char line[128]; static uint8_t llen = 0;
static uint32_t imuSamples = 0; static char imuLast[128];
static uint32_t lastRep = 0;

void setup() {
    Serial.begin(115200); delay(1500);
    Serial.println(F("\n##### SensorDiag8: 深度详查 + IMU实时 #####"));
    depthScan();
    pinMode(PIN_CS, OUTPUT); digitalWrite(PIN_CS, HIGH);
    pinMode(PIN_INT, INPUT_PULLUP);
    SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);
    spiSettings = SPISettings(10000000, MSBFIRST, SPI_MODE0);
    delay(100);
    imuStart();
    Serial.println(F("\n实时 IMU 姿态(转动 IMU 看角度变化):\n"));
}

void loop() {
    if (imuBaud) {
        while (chAvail(IMU_UART)) {
            const uint8_t c = chRr(IMU_UART, REG_RHR);
            if (c == '\r') continue;
            if (c == '\n') { line[llen] = '\0'; if (llen > 0) { strncpy(imuLast, line, sizeof(imuLast)); if (line[0] == '*') imuSamples++; } llen = 0; continue; }
            if (llen < sizeof(line) - 1) line[llen++] = (char)c; else llen = 0;
        }
        const uint32_t now = millis();
        if (now - lastRep >= 1000) {
            lastRep = now;
            Serial.printf("[IMU] samples=%lu  当前姿态: %s\n", (unsigned long)imuSamples, imuLast);
        }
    }
}
