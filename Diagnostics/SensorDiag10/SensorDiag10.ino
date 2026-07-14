/**********************************************************************
 * SensorDiag10.ino  —  深度 I2C SDA/SCL 双接法扫描（查是否接反）
 *
 * 独立 sketch。ESP32 原生 I2C 引脚可软件交换，分别用两种接法扫描：
 *   A) 正常: SDA=IO4, SCL=IO5
 *   B) 交换: SDA=IO5, SCL=IO4
 * 哪种能扫到 0x76（err=0 ACK）即为正确接法。
 *
 * 解读:
 *   某一种 found>0 / 0x76 err=0  -> 传感器在, 用那种接法
 *   两种都 found=0 err=2         -> 不是接反; 是没供电/断线/传感器坏
 *   出现 err=4                   -> 该接法下总线异常
 *********************************************************************/

#include <Arduino.h>
#include <Wire.h>

static void scanOrient(const char* tag, int sda, int scl) {
    Serial.printf("\n--- %s: SDA=IO%d SCL=IO%d @100kHz ---\n", tag, sda, scl);
    Wire.end();
    delay(20);
    if (!Wire.begin(sda, scl, 100000)) {
        Serial.println("  Wire.begin 失败");
        return;
    }
    Wire.setTimeOut(50);
    delay(20);

    uint8_t found = 0;
    for (uint8_t a = 0x08; a <= 0x77; a++) {
        Wire.beginTransmission(a);
        if (Wire.endTransmission() == 0) {
            Serial.printf("    设备 @0x%02X (ACK)\n", a);
            found++;
        }
    }
    Wire.beginTransmission(0x76);
    const uint8_t e76 = Wire.endTransmission();
    Wire.beginTransmission(0x77);
    const uint8_t e77 = Wire.endTransmission();
    Serial.printf("  found=%u | 0x76 err=%u | 0x77 err=%u\n", found, e76, e77);
    if (found) Serial.println("  >>> 这种接法有器件应答！");
}

void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println("\n##### SensorDiag10: 深度 I2C 双接法扫描 #####");
    Serial.println("err=0 ACK | err=2 地址NACK(总线OK无器件) | err=4 总线错 | err=5 超时");
}

// 在工作接法 (SDA=5,SCL=4) 下完整读 MS5837：reset+PROM+CRC+压力
static uint8_t crc4(uint16_t* n) {
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
static bool readAdc(uint8_t conv, uint32_t* v) {
    Wire.beginTransmission(0x76); Wire.write(conv);
    if (Wire.endTransmission() != 0) return false;
    delay(20);
    Wire.beginTransmission(0x76); Wire.write(0x00);
    if (Wire.endTransmission(true) != 0) return false;
    if (Wire.requestFrom((int)0x76, 3) != 3) return false;
    *v = ((uint32_t)Wire.read() << 16) | ((uint32_t)Wire.read() << 8) | Wire.read();
    return true;
}
static void fullRead() {
    uint16_t prom[8];
    Wire.beginTransmission(0x76); Wire.write(0x1E);
    if (Wire.endTransmission() != 0) { Serial.println("  完整读: reset 失败"); return; }
    delay(20);
    for (uint8_t i = 0; i < 7; i++) {
        Wire.beginTransmission(0x76); Wire.write(0xA0 + i * 2);
        if (Wire.endTransmission(true) != 0) { Serial.println("  PROM 写失败"); return; }
        if (Wire.requestFrom((int)0x76, 2) != 2) { Serial.println("  PROM 读失败"); return; }
        prom[i] = ((uint16_t)Wire.read() << 8) | Wire.read();
    }
    prom[7] = 0;
    const uint8_t ce = prom[0] >> 12, cc = crc4(prom);
    Serial.printf("  PROM CRC: exp=%X calc=%X %s\n", ce, cc, (ce == cc) ? "OK ✓" : "FAIL");
    if (ce != cc) return;
    uint32_t d1 = 0, d2 = 0;
    if (!readAdc(0x4A, &d1) || !readAdc(0x5A, &d2)) { Serial.println("  ADC 读失败"); return; }
    const int32_t dT = (int32_t)d2 - ((int32_t)prom[5] << 8);
    int64_t temp = 2000 + ((int64_t)dT * prom[6] >> 23);
    int64_t off = ((int64_t)prom[2] << 17) + (((int64_t)prom[4] * dT) >> 6);
    int64_t sens = ((int64_t)prom[1] << 16) + (((int64_t)prom[3] * dT) >> 7);
    const int64_t p = (((int64_t)d1 * sens >> 21) - off) >> 15;
    Serial.printf("  >>> 压力=%.2f mbar  温度=%.2f C  (空气中应≈1013mbar) ✓✓✓\n",
                  p * 0.01f, temp * 0.01f);
}

void loop() {
    scanOrient("A 正常", 4, 5);
    scanOrient("B 交换", 5, 4);
    // B 接法下完整读一次
    Serial.println("  -- B 接法完整读取 --");
    fullRead();
    Serial.println("\n========== 等 3s 重扫 ==========");
    delay(3000);
}
