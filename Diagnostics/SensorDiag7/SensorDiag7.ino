/**********************************************************************
 * SensorDiag7.ino  —  修复后最终验证（深度 MS5837 + IMU 姿态）
 *
 * 独立 sketch，不依赖/不修改主固件。
 *
 * 修复内容验证:
 *   - 深度 MS5837: 供电由 5V 改 3.3V 后，I2C 应答 + PROM CRC + 读压力
 *     (空气中压力应≈大气压 ~1013mbar，温度≈室温，证明传感器正常)
 *   - IMU EBIMU: TX/RX 对调后，<ver> 应答 + ASCII 姿态流 *roll,pitch,yaw
 *
 * 引脚: 深度 I2C SDA=4 SCL=5 @0x76; CH9434A SPI 10-13,INT9; IMU=U3
 * 命令(115200): s=IMU重扫波特率
 *********************************************************************/

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

// ===== 深度 MS5837 =====
static const int DEPTH_SDA = 4, DEPTH_SCL = 5;
static const uint8_t MS_ADDR = 0x76;
static uint16_t prom[8];
static bool depthOk = false;

static uint8_t ms5837Crc4(uint16_t* n) {
    uint16_t rem = 0, c[8];
    for (uint8_t i = 0; i < 8; i++) c[i] = n[i];
    c[0] &= 0x0FFF; c[7] = 0;
    for (uint8_t cnt = 0; cnt < 16; cnt++) {
        if (cnt & 1) rem ^= (uint16_t)(c[cnt >> 1] & 0x00FF);
        else rem ^= (uint16_t)(c[cnt >> 1] >> 8);
        for (uint8_t b = 0; b < 8; b++)
            rem = (rem & 0x8000) ? (uint16_t)((rem << 1) ^ 0x3000) : (uint16_t)(rem << 1);
    }
    return (uint8_t)((rem >> 12) & 0x0F);
}

static bool ms5837Begin() {
    Wire.begin(DEPTH_SDA, DEPTH_SCL, 100000);
    Wire.setTimeOut(50);
    // reset
    Wire.beginTransmission(MS_ADDR); Wire.write(0x1E);
    if (Wire.endTransmission() != 0) { Serial.println(F("  MS5837 reset: 无应答")); return false; }
    delay(20);
    // PROM 0xA0..0xAC
    for (uint8_t i = 0; i < 7; i++) {
        Wire.beginTransmission(MS_ADDR); Wire.write(0xA0 + i * 2);
        if (Wire.endTransmission(true) != 0) { Serial.printf("  PROM[%u] 写失败\n", i); return false; }
        if (Wire.requestFrom((int)MS_ADDR, 2) != 2) { Serial.printf("  PROM[%u] 读失败\n", i); return false; }
        prom[i] = ((uint16_t)Wire.read() << 8) | Wire.read();
    }
    prom[7] = 0;
    const uint8_t crcExp = prom[0] >> 12, crcCalc = ms5837Crc4(prom);
    Serial.printf("  PROM: %04X %04X %04X %04X %04X %04X %04X  CRC exp=%X calc=%X\n",
                  prom[0], prom[1], prom[2], prom[3], prom[4], prom[5], prom[6], crcExp, crcCalc);
    return crcExp == crcCalc;
}

static bool ms5837ReadAdc(uint8_t conv, uint32_t* val) {
    Wire.beginTransmission(MS_ADDR); Wire.write(conv);
    if (Wire.endTransmission() != 0) return false;
    delay(20);
    Wire.beginTransmission(MS_ADDR); Wire.write(0x00);
    if (Wire.endTransmission(true) != 0) return false;
    if (Wire.requestFrom((int)MS_ADDR, 3) != 3) return false;
    *val = ((uint32_t)Wire.read() << 16) | ((uint32_t)Wire.read() << 8) | Wire.read();
    return true;
}

static void ms5837Read() {
    uint32_t d1 = 0, d2 = 0;
    if (!ms5837ReadAdc(0x4A, &d1) || !ms5837ReadAdc(0x5A, &d2)) {
        Serial.println(F("  ADC 读失败")); return;
    }
    const int32_t dT = (int32_t)d2 - ((int32_t)prom[5] << 8);
    int64_t temp = 2000 + ((int64_t)dT * prom[6] >> 23);
    int64_t off = ((int64_t)prom[2] << 17) + (((int64_t)prom[4] * dT) >> 6);
    int64_t sens = ((int64_t)prom[1] << 16) + (((int64_t)prom[3] * dT) >> 7);
    const int64_t p = (((int64_t)d1 * sens >> 21) - off) >> 15;
    Serial.printf("  D1=%lu D2=%lu | 压力=%.2f mbar | 温度=%.2f C\n",
                  (unsigned long)d1, (unsigned long)d2, p * 0.01f, temp * 0.01f);
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
static const uint32_t CAND[] = {115200, 9600, 19200, 38400, 57600, 230400, 460800, 921600};
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

static void imuScanAndStart() {
    imuBaud = 0;
    Serial.println(F("  IMU <ver> 扫描:"));
    for (uint8_t i = 0; i < 8; i++) {
        chSetBaud(IMU_UART, CAND[i]); chFlush(IMU_UART); delay(15);
        chPuts(IMU_UART, "<ver>");
        uint8_t buf[48], n = 0; const uint32_t t0 = millis();
        while (millis() - t0 < 250 && n < 48) if (chAvail(IMU_UART)) buf[n++] = chRr(IMU_UART, REG_RHR);
        bool hit = false;
        for (uint8_t j = 0; j + 2 < n; j++) if (buf[j] == 'i' && buf[j+1] == 'm' && buf[j+2] == 'u') hit = true;
        Serial.printf("    %6lu: %u bytes \"", (unsigned long)CAND[i], n);
        for (uint8_t j = 0; j < n && j < 20; j++) Serial.print((buf[j] >= 32 && buf[j] < 127) ? (char)buf[j] : '.');
        Serial.print(F("\""));
        if (hit) { Serial.print(F("  <== imu!")); imuBaud = CAND[i]; Serial.println(); break; }
        Serial.println();
    }
    if (!imuBaud) { Serial.println(F("  IMU 仍无 <ver> 应答")); return; }
    Serial.printf("  IMU 波特率=%lu，启动 ASCII 输出\n", (unsigned long)imuBaud);
    chSetBaud(IMU_UART, imuBaud);
    const char* seq[] = {"<stop>","<soc1>","<sof1>","<sog0>","<soa0>","<som0>",
                         "<sod0>","<sot0>","<sots0>","<sor20>","<pons1>","<start>"};
    for (uint8_t i = 0; i < 12; i++) { chPuts(IMU_UART, seq[i]); delay(80); }
    chFlush(IMU_UART);
}

static char line[128]; static uint8_t llen = 0;
static uint32_t imuSamples = 0; static char imuLast[128];
static uint32_t lastDepth = 0, lastRep = 0;

void setup() {
    Serial.begin(115200); delay(1500);
    Serial.println(F("\n##### SensorDiag7: 修复后最终验证 #####"));

    Serial.println(F("\n[1] 深度 MS5837 @3.3V:"));
    depthOk = ms5837Begin();
    Serial.println(depthOk ? F("  -> PROM CRC OK, 传感器在线!") : F("  -> 初始化失败"));

    Serial.println(F("\n[2] IMU EBIMU (TX/RX 已对调):"));
    pinMode(PIN_CS, OUTPUT); digitalWrite(PIN_CS, HIGH);
    pinMode(PIN_INT, INPUT_PULLUP);
    SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);
    spiSettings = SPISettings(10000000, MSBFIRST, SPI_MODE0);
    delay(100);
    imuScanAndStart();

    Serial.println(F("\n开始实时显示 (深度每2s + IMU姿态)。命令: s=IMU重扫\n"));
}

void loop() {
    if (Serial.available() && Serial.read() == 's') { imuSamples = 0; imuLast[0] = '\0'; imuScanAndStart(); }

    if (imuBaud) {
        while (chAvail(IMU_UART)) {
            const uint8_t c = chRr(IMU_UART, REG_RHR);
            if (c == '\r') continue;
            if (c == '\n') { line[llen] = '\0'; if (llen > 0) { strncpy(imuLast, line, sizeof(imuLast)); if (line[0] == '*') imuSamples++; } llen = 0; continue; }
            if (llen < sizeof(line) - 1) line[llen++] = (char)c; else llen = 0;
        }
    }

    const uint32_t now = millis();
    if (depthOk && now - lastDepth >= 2000) { lastDepth = now; Serial.print(F("[深度] ")); ms5837Read(); }
    if (now - lastRep >= 1000) {
        lastRep = now;
        Serial.printf("[IMU] samples=%lu last=\"%s\"\n", (unsigned long)imuSamples, imuLast);
    }
}
