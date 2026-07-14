/**********************************************************************
 * SensorDiag9.ino  —  三路超声波读取测试(空气中验证通信)
 *
 * 独立 sketch，不依赖/不修改主固件。
 *
 * DYP-L08 水下超声波(受控 UART): 发低脉冲触发 -> 回 4 字节帧
 *   [0xFF, Data_H, Data_L, SUM], SUM=(0xFF+Data_H+Data_L)&0xFF
 *   距离mm = Data_H*256 + Data_L。空气中无水介质 -> 回无回波/超量程(~0xFFxx)。
 *   触发周期建议 >=15ms (datasheet 3.1.1)。
 *
 * 本测试: 规范触发三路, 统计各路响应率/校验通过率, 显示原始帧与解析。
 *   能收到合法校验帧 = 该路通信(接线/波特率/芯片通道)正常, 即使空气中距离无效。
 *
 * 引脚: CH9434A SPI 10-13,INT9; U1=前 U2=左 U0=右
 *********************************************************************/

#include <Arduino.h>
#include <SPI.h>

static const int PIN_MOSI = 11, PIN_MISO = 13, PIN_SCK = 12, PIN_CS = 10, PIN_INT = 9;
static const uint8_t REG_THR = 0x00, REG_RHR = 0x00, REG_DLL = 0x00;
static const uint8_t REG_IER = 0x01, REG_DLM = 0x01, REG_FCR = 0x02;
static const uint8_t REG_LCR = 0x03, REG_LSR = 0x05;
static const uint8_t LCR_DLAB = 0x80, LCR_8N1 = 0x03, FCR_INIT = 0x07;
static const uint8_t LSR_DR = 0x01, LSR_THRE = 0x20;
static const uint8_t UART_STEP = 0x10;
static const uint32_t CH_CLOCK = 32000000;
static SPISettings spiSettings;

static const uint8_t US_UART[3] = {1, 2, 0};
static const char*   US_NAME[3] = {"前U1", "左U2", "右U0"};

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
static void chConfig(uint8_t u) {
    const uint16_t d = CH_CLOCK / (8 * 115200);
    chW(u, REG_LCR, LCR_8N1 | LCR_DLAB); chW(u, REG_DLL, d & 0xFF);
    chW(u, REG_DLM, (d >> 8) & 0xFF); chW(u, REG_LCR, LCR_8N1);
    chW(u, REG_FCR, FCR_INIT); chW(u, REG_IER, 0x00);
}
static void chFlush(uint8_t u) { chW(u, REG_FCR, FCR_INIT); }
static bool chAvail(uint8_t u) { return chRr(u, REG_LSR) & LSR_DR; }
static void chPutc(uint8_t u, uint8_t c) {
    const uint32_t t0 = millis();
    while (!(chRr(u, REG_LSR) & LSR_THRE)) if (millis() - t0 > 20) break;
    chW(u, REG_THR, c);
}

static uint32_t trig[3] = {0, 0, 0}, resp[3] = {0, 0, 0}, good[3] = {0, 0, 0};
static uint8_t lastBuf[3][8], lastLen[3] = {0, 0, 0};

// 触发一路并读取一帧
static void ping(uint8_t idx) {
    const uint8_t u = US_UART[idx];
    chFlush(u);
    chPutc(u, 0x00);             // 低脉冲触发
    trig[idx]++;
    uint8_t buf[8]; uint8_t n = 0;
    const uint32_t t0 = millis();
    while (millis() - t0 < 30 && n < 8) {
        if (chAvail(u)) buf[n++] = chRr(u, REG_RHR);
        else if (n >= 4) break;
    }
    if (n == 0) return;
    resp[idx]++;
    lastLen[idx] = n; for (uint8_t i = 0; i < n; i++) lastBuf[idx][i] = buf[i];
    // 找 FF 帧头校验
    for (uint8_t i = 0; i + 3 < n || i == 0; i++) {
        if (i + 3 >= n) break;
        if (buf[i] == 0xFF && (uint8_t)(buf[i] + buf[i+1] + buf[i+2]) == buf[i+3]) { good[idx]++; break; }
    }
}

static uint32_t lastRep = 0;

void setup() {
    Serial.begin(115200); delay(1500);
    Serial.println(F("\n##### SensorDiag9: 三路超声波读取测试 #####"));
    Serial.println(F("(空气中: 能收到合法校验帧=通信OK; 距离会是超量程/无回波)"));
    pinMode(PIN_CS, OUTPUT); digitalWrite(PIN_CS, HIGH);
    pinMode(PIN_INT, INPUT_PULLUP);
    SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);
    spiSettings = SPISettings(10000000, MSBFIRST, SPI_MODE0);
    delay(100);
    for (uint8_t i = 0; i < 3; i++) chConfig(US_UART[i]);
    Serial.println();
}

void loop() {
    // 依次触发三路, 每路间隔 >=15ms
    for (uint8_t i = 0; i < 3; i++) { ping(i); delay(20); }

    const uint32_t now = millis();
    if (now - lastRep >= 1000) {
        lastRep = now;
        Serial.printf("---- [%lus] ----\n", now / 1000);
        for (uint8_t i = 0; i < 3; i++) {
            Serial.printf("  %s: 触发%lu 响应%lu 合法帧%lu | 末帧:",
                          US_NAME[i], (unsigned long)trig[i], (unsigned long)resp[i], (unsigned long)good[i]);
            for (uint8_t j = 0; j < lastLen[i]; j++) Serial.printf(" %02X", lastBuf[i][j]);
            if (lastLen[i] >= 4 && lastBuf[i][0] == 0xFF) {
                const uint16_t mm = (lastBuf[i][1] << 8) | lastBuf[i][2];
                if (mm >= 0xFFF0) Serial.print(F("  -> 超量程/无回波(空气正常)"));
                else Serial.printf("  -> %u mm", mm);
            }
            Serial.println();
        }
    }
}
