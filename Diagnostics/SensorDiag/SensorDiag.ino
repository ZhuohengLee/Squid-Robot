/**********************************************************************
 * SensorDiag.ino  —  独立传感器诊断 sketch（不依赖、不修改主固件）
 *
 * 用途：诊断 Squid-Robot ESP32-S3 上的传感器，区分
 *   「扩展芯片 CH9434A / 总线问题」 vs 「外部传感器 / 接线问题」。
 *
 * 烧录这个 sketch 会临时覆盖主固件；诊断完后把主固件重新烧回即可。
 *
 * 引脚与主固件 Protocol.h 保持一致：
 *   深度 MS5837 I2C : SDA=IO4, SCL=IO5, addr 0x76
 *   CH9434A SPI     : MOSI=IO11, MISO=IO13, SCK=IO12, CS=IO10, INT=IO9
 *   CH9434A UART 分配: U1=前超声, U2=左超声, U0=右超声, U3=IMU
 *
 * 串口命令（115200）：
 *   i  → 只扫描深度 I2C 总线
 *   c  → 只跑 CH9434A 自检（寄存器回读 + UART 内部自环）
 *   u  → 监听 4 路 UART 原始字节 5 秒（看传感器有没有真的在发数据）
 *   a  → 全部依次跑一遍（上电默认执行一次）
 *********************************************************************/

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

// ── 引脚（对齐 Protocol.h）────────────────────────────────────────
static const int DEPTH_SDA = 4;
static const int DEPTH_SCL = 5;
static const uint8_t DEPTH_ADDR = 0x76;

static const int PIN_MOSI = 11;
static const int PIN_MISO = 13;
static const int PIN_SCK  = 12;
static const int PIN_CS   = 10;
static const int PIN_INT  = 9;

// ── CH9434A 寄存器（16550 兼容）──────────────────────────────────
static const uint8_t REG_RHR = 0x00, REG_THR = 0x00, REG_DLL = 0x00;
static const uint8_t REG_IER = 0x01, REG_DLM = 0x01;
static const uint8_t REG_IIR = 0x02, REG_FCR = 0x02;
static const uint8_t REG_LCR = 0x03, REG_MCR = 0x04, REG_LSR = 0x05;
static const uint8_t LCR_DLAB = 0x80, LCR_8N1 = 0x03;
static const uint8_t FCR_INIT = 0x07;          // enable | rx_reset | tx_reset
static const uint8_t MCR_LOOP = 0x10;
static const uint8_t LSR_DR   = 0x01;
static const uint8_t NUM_UARTS = 4;
static const uint8_t UART_STEP = 0x10;

// CH9434A 内部时钟，用于波特率分频
static const uint32_t CH_CLOCK = 32000000;
// SPI 频率：主固件用 10MHz；如怀疑信号完整性可改成 1000000 复测
static uint32_t spiFreq = 10000000;
static SPISettings spiSettings;

// ── CH9434A 最小 SPI 读写 ────────────────────────────────────────
static void chWrite(uint8_t uart, uint8_t reg, uint8_t val) {
    const uint8_t addr = reg + UART_STEP * uart;
    SPI.beginTransaction(spiSettings);
    digitalWrite(PIN_CS, LOW);
    SPI.transfer(0x80 | addr);
    delayMicroseconds(1);
    SPI.transfer(val);
    delayMicroseconds(3);
    digitalWrite(PIN_CS, HIGH);
    SPI.endTransaction();
}

static uint8_t chRead(uint8_t uart, uint8_t reg) {
    const uint8_t addr = reg + UART_STEP * uart;
    SPI.beginTransaction(spiSettings);
    digitalWrite(PIN_CS, LOW);
    SPI.transfer(addr);
    delayMicroseconds(3);
    const uint8_t val = SPI.transfer(0xFF);
    delayMicroseconds(1);
    digitalWrite(PIN_CS, HIGH);
    SPI.endTransaction();
    return val;
}

static void chSetBaud(uint8_t uart, uint32_t baud) {
    const uint16_t div = CH_CLOCK / (8 * baud);
    const uint8_t lcr = chRead(uart, REG_LCR);
    chWrite(uart, REG_LCR, lcr | LCR_DLAB);
    chWrite(uart, REG_DLL, div & 0xFF);
    chWrite(uart, REG_DLM, (div >> 8) & 0xFF);
    chWrite(uart, REG_LCR, lcr & ~LCR_DLAB);
}

static void chConfig(uint8_t uart, uint32_t baud) {
    chSetBaud(uart, baud);
    chWrite(uart, REG_LCR, LCR_8N1);
    chWrite(uart, REG_FCR, FCR_INIT);
}

// ── 1) 深度传感器 I2C 扫描 ───────────────────────────────────────
static void runI2cScan() {
    Serial.println(F("\n================ I2C SCAN ================"));
    Serial.printf("SDA=IO%d  SCL=IO%d  @100kHz\n", DEPTH_SDA, DEPTH_SCL);

    Wire.begin(DEPTH_SDA, DEPTH_SCL, 100000);
    Wire.setTimeOut(50);

    uint8_t found = 0;
    for (uint8_t addr = 0x08; addr <= 0x77; addr++) {
        Wire.beginTransmission(addr);
        const uint8_t err = Wire.endTransmission();
        if (err == 0) {
            Serial.printf("  device @ 0x%02X\n", addr);
            found++;
        }
    }
    Wire.beginTransmission(DEPTH_ADDR);
    const uint8_t probeErr = Wire.endTransmission();
    Serial.printf("  probe 0x76 -> %s (endTransmission=%u)\n",
                  probeErr == 0 ? "ACK" : "no ACK", probeErr);

    Serial.printf("  total found: %u\n", found);
    Serial.println(F("  解读: 0 设备 + err=2 -> 设备不在/没供电;"));
    Serial.println(F("        0 设备 + err=4 -> 总线坏(缺上拉/SDA-SCL 断或短)"));
    Serial.println(F("=========================================\n"));
}

// ── 2) CH9434A 自检：寄存器回读 + UART 内部自环 ──────────────────
static void runChipTest() {
    Serial.println(F("\n============ CH9434A SELF-TEST ==========="));
    Serial.printf("SPI: MOSI=%d MISO=%d SCK=%d CS=%d  @%luHz\n",
                  PIN_MOSI, PIN_MISO, PIN_SCK, PIN_CS, (unsigned long)spiFreq);

    const uint8_t regPat[]  = {0x00, 0xFF, 0xAA, 0x55, 0x5A, 0xA5};
    const uint8_t loopByte[] = {0xA5, 0x3C, 0x00, 0xFF};
    uint16_t totBad = 0; uint8_t totLoopFail = 0;

    for (uint8_t u = 0; u < NUM_UARTS; u++) {
        Serial.printf("\n-- UART%u --\n", u);
        Serial.printf("  LSR=0x%02X IIR=0x%02X (健康复位后 LSR 应=0x60)\n",
                      chRead(u, REG_LSR), chRead(u, REG_IIR));

        // 寄存器写回读完整性（DLAB 下 DLL 当 scratch），48 次
        chWrite(u, REG_LCR, LCR_DLAB);
        uint16_t total = 0, bad = 0;
        for (uint8_t r = 0; r < 8; r++)
            for (uint8_t p = 0; p < sizeof(regPat); p++) {
                chWrite(u, REG_DLL, regPat[p]);
                if (chRead(u, REG_DLL) != regPat[p]) bad++;
                total++;
            }
        chWrite(u, REG_LCR, LCR_8N1);
        totBad += bad;
        Serial.printf("  SPI reg r/w: %u/%u %s\n",
                      total - bad, total, bad == 0 ? "OK" : "<-- MISMATCH");

        // UART 内部自环（不依赖外部传感器）
        chSetBaud(u, 115200);
        chWrite(u, REG_LCR, LCR_8N1);
        chWrite(u, REG_FCR, FCR_INIT);
        chWrite(u, REG_MCR, MCR_LOOP);
        uint8_t pass = 0;
        for (uint8_t i = 0; i < sizeof(loopByte); i++) {
            chWrite(u, REG_THR, loopByte[i]);
            bool ready = false;
            const uint32_t t0 = millis();
            while (millis() - t0 < 20)
                if (chRead(u, REG_LSR) & LSR_DR) { ready = true; break; }
            if (ready && chRead(u, REG_RHR) == loopByte[i]) pass++;
        }
        chWrite(u, REG_MCR, 0x00);
        chWrite(u, REG_FCR, FCR_INIT);
        if (pass != sizeof(loopByte)) totLoopFail++;
        Serial.printf("  loopback:    %u/%u %s\n",
                      pass, (unsigned)sizeof(loopByte),
                      pass == sizeof(loopByte) ? "OK" : "<-- FAIL");
    }

    Serial.println(F("\n---- VERDICT ----"));
    if (totBad == 0 && totLoopFail == 0) {
        Serial.println(F("CH9434A 健康(SPI + UART 内核都正常)。"));
        Serial.println(F("-> 故障在外部: 传感器供电/接线/传感器本身。用 'u' 看 UART 原始数据。"));
    } else if (totBad > 0) {
        Serial.println(F("SPI 寄存器回读有错 -> SPI 链路/CH9434A 不稳。"));
        Serial.println(F("-> 查 SPI 接线 IO10-13、CH9434A 供电；可把 spiFreq 改 1MHz 复测。"));
    } else {
        Serial.println(F("寄存器 OK 但自环失败 -> CH9434A UART 内核/波特率异常(芯片半损)。"));
    }
    Serial.println(F("=========================================\n"));
}

// ── 3) 监听 4 路 UART 原始字节 ───────────────────────────────────
// 超声波各波特率 115200；IMU 115200。监听看是否真有数据进来。
static void runUartSniff() {
    const uint32_t bauds[NUM_UARTS] = {115200, 115200, 115200, 115200};
    const char* names[NUM_UARTS] = {"U0(右超声)", "U1(前超声)", "U2(左超声)", "U3(IMU)"};
    Serial.println(F("\n============ UART SNIFF (5s) ============="));
    for (uint8_t u = 0; u < NUM_UARTS; u++) chConfig(u, bauds[u]);

    uint32_t cnt[NUM_UARTS] = {0, 0, 0, 0};
    uint8_t  sample[NUM_UARTS][8] = {{0}};
    uint8_t  sampLen[NUM_UARTS] = {0};

    const uint32_t t0 = millis();
    while (millis() - t0 < 5000) {
        for (uint8_t u = 0; u < NUM_UARTS; u++) {
            while (chRead(u, REG_LSR) & LSR_DR) {
                const uint8_t b = chRead(u, REG_RHR);
                if (sampLen[u] < 8) sample[u][sampLen[u]++] = b;
                cnt[u]++;
            }
        }
    }

    for (uint8_t u = 0; u < NUM_UARTS; u++) {
        Serial.printf("  %s: %lu bytes", names[u], (unsigned long)cnt[u]);
        if (sampLen[u]) {
            Serial.print(F("  head:"));
            for (uint8_t i = 0; i < sampLen[u]; i++) Serial.printf(" %02X", sample[u][i]);
        }
        Serial.println();
    }
    Serial.println(F("  解读: 某路 >0 bytes -> 该传感器在发数据(芯片RX通);"));
    Serial.println(F("        全 0 bytes 但自检 OK -> 传感器没供电/没接/坏。"));
    Serial.println(F("=========================================\n"));
}

static void initSpi() {
    pinMode(PIN_CS, OUTPUT);
    digitalWrite(PIN_CS, HIGH);
    if (PIN_INT >= 0) pinMode(PIN_INT, INPUT_PULLUP);
    SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);
    spiSettings = SPISettings(spiFreq, MSBFIRST, SPI_MODE0);
    delay(100);
    // 基础初始化 4 路（与主固件 begin 一致）
    for (uint8_t u = 0; u < NUM_UARTS; u++) {
        chWrite(u, REG_IER, 0x00);
        chWrite(u, REG_FCR, FCR_INIT);
        chWrite(u, REG_LCR, LCR_8N1);
    }
}

static void runAll() {
    runI2cScan();
    runChipTest();
    runUartSniff();
    Serial.println(F(">>> 命令: i=I2C扫描  c=芯片自检  u=UART监听  a=全部"));
}

void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println(F("\n##### Squid-Robot Sensor Diagnostics #####"));
    initSpi();
    runAll();
}

void loop() {
    if (Serial.available()) {
        const char c = Serial.read();
        switch (c) {
            case 'i': runI2cScan();  break;
            case 'c': runChipTest(); break;
            case 'u': runUartSniff(); break;
            case 'a': runAll();      break;
            default: break;
        }
    }
}
