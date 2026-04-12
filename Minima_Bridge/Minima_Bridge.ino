/**********************************************************************
 * Arduino HC-12 透明串口桥  (Arduino UNO R4 Minima)
 *
 * 架构：
 *   PC  ──USB Serial──►  Minima  ──HC-12 无线──►  ESP32 机器人
 *   PC  ◄─USB Serial──   Minima  ◄─HC-12 无线──   ESP32 机器人
 *
 * 正常使用：
 *   完全透明转发。PC 发送的所有内容原样经 HC-12 转发给机器人，
 *   机器人的所有回传也原样转发回 PC。
 *
 * ACK 重发机制：
 *   每条命令发出后扫描回传流，检测到 "[OK]" 即确认送达（1500ms 超时）。
 *   若超时未收到 ACK，自动重发，最多重试 3 次。
 *
 * 本端信道配置命令（PC 发 → Minima 拦截处理，不转发）：
 *   HC001 ~ HC127   将本端 HC-12 切换到指定信道
 *
 * 机器人端信道配置命令（PC 发 → 透传给机器人）：
 *   ESP001 ~ ESP127  机器人收到后自行配置其 HC-12 信道
 *
 * ⚠ 双端同步信道的正确顺序：
 *   1. 先发 ESP025  → 机器人切换到新信道
 *   2. 再发 HC025   → 本端切换到新信道 → 链路恢复
 *
 * HC-12 接线（Arduino UNO R4 Minima）：
 *   HC-12 TX  → D2  (SoftwareSerial RX)
 *   HC-12 RX  → D3  (SoftwareSerial TX)
 *   HC-12 SET → D4
 *   HC-12 VCC → 5V
 *   HC-12 GND → GND
 *********************************************************************/

#include <SoftwareSerial.h>

#define HC12_RX_PIN  2
#define HC12_TX_PIN  3
#define HC12_SET_PIN 4
#define PC_BAUD      115200
#define HC12_BAUD    9600
#define PC_BUF_MAX   64

constexpr uint32_t ACK_TIMEOUT_MS = 1500;
constexpr uint8_t  MAX_RETRIES    = 3;

// ACK 模式匹配（滚动扫描字节流，不依赖换行符）
static const char ACK_PREFIX[]   = "[OK]";
static const uint8_t ACK_LEN     = 4;

SoftwareSerial hc12(HC12_RX_PIN, HC12_TX_PIN);

String   pcBuffer   = "";
String   pendingCmd = "";
uint32_t pendingMs  = 0;
uint8_t  retryCount = 0;
bool     waitingAck = false;
uint8_t  ackMatchIdx = 0;  // 当前已匹配 ACK_PREFIX 的字节数

// ────────────────────────────────────────
// 发送命令并启动 ACK 等待
// ────────────────────────────────────────
void sendCmd(const String& cmd) {
    hc12.println(cmd);
    pendingCmd   = cmd;
    pendingMs    = millis();
    retryCount   = 0;
    waitingAck   = true;
    ackMatchIdx  = 0;
}

// ────────────────────────────────────────
// 将本端 HC-12 切换到指定信道（AT 模式）
// ────────────────────────────────────────
void configHC12Channel(const String& channel) {
    waitingAck  = false;
    ackMatchIdx = 0;

    Serial.print(F("[HC-12] Setting channel to "));
    Serial.print(channel);
    Serial.print(F(" ..."));

    digitalWrite(HC12_SET_PIN, LOW);
    delay(200);

    hc12.print(F("AT+C"));
    hc12.println(channel);

    String response = "";
    const uint32_t start = millis();
    while (millis() - start < 1000) {
        if (hc12.available()) {
            response += (char)hc12.read();
        }
    }

    digitalWrite(HC12_SET_PIN, HIGH);
    delay(200);

    response.trim();
    if (response.indexOf("OK") >= 0) {
        Serial.print(F(" OK (ch."));
        Serial.print(channel);
        Serial.println(F(")"));
    } else {
        Serial.print(F(" FAILED"));
        if (response.length() > 0) {
            Serial.print(F(": "));
            Serial.print(response);
        } else {
            Serial.print(F(": no response — check 5V power and SET wiring"));
        }
        Serial.println();
    }
}

// ────────────────────────────────────────
// 初始化
// ────────────────────────────────────────
void setup() {
    pinMode(HC12_SET_PIN, OUTPUT);
    digitalWrite(HC12_SET_PIN, HIGH);

    Serial.begin(PC_BAUD);
    while (!Serial);
    hc12.begin(HC12_BAUD);

    Serial.println(F("============================================"));
    Serial.println(F("   Squid Robot  —  Minima HC-12 Bridge"));
    Serial.println(F("--------------------------------------------"));
    Serial.println(F(" Channel pairing"));
    Serial.println(F("   HC025   set THIS  end to ch.025"));
    Serial.println(F("   ESP025  set ROBOT end to ch.025"));
    Serial.println(F("   Order : ESP025 first, HC025 second"));
    Serial.println(F("--------------------------------------------"));
    Serial.println(F(" ACK retry: 1500ms timeout, max 3 retries"));
    Serial.println(F("============================================"));
}

// ────────────────────────────────────────
// 主循环
// ────────────────────────────────────────
void loop() {
    // ── PC → HC-12 ──────────────────────────────────────────────────
    while (Serial.available()) {
        const char c = Serial.read();

        if (c == '\n' || c == '\r') {
            if (pcBuffer.length() > 0) {
                if (pcBuffer.length() == 5  &&
                    pcBuffer[0] == 'H'      &&
                    pcBuffer[1] == 'C'      &&
                    isDigit(pcBuffer[2])    &&
                    isDigit(pcBuffer[3])    &&
                    isDigit(pcBuffer[4])) {
                    configHC12Channel(pcBuffer.substring(2));
                } else {
                    sendCmd(pcBuffer);
                }
                pcBuffer = "";
            }
        } else {
            pcBuffer += c;
            if (pcBuffer.length() > PC_BUF_MAX) {
                Serial.println(F("[Bridge] Input overflow, buffer cleared."));
                pcBuffer = "";
            }
        }
    }

    // ── HC-12 → PC（字节级转发 + ACK 模式扫描）───────────────────────
    // 所有字节原样转发给 PC，同时滚动扫描 "[OK]" 确认 ACK。
    while (hc12.available()) {
        const char c = hc12.read();
        Serial.write(c);  // 原样转发

        // 滚动匹配 "[OK]"
        if (waitingAck) {
            if (c == ACK_PREFIX[ackMatchIdx]) {
                ackMatchIdx++;
                if (ackMatchIdx == ACK_LEN) {
                    // 匹配完成
                    waitingAck  = false;
                    ackMatchIdx = 0;
                    Serial.print(F("\n[ACK] "));
                    Serial.println(pendingCmd);
                }
            } else {
                ackMatchIdx = (c == ACK_PREFIX[0]) ? 1 : 0;
            }
        }
    }

    // ── ACK 超时重发 ─────────────────────────────────────────────────
    if (waitingAck && (millis() - pendingMs) > ACK_TIMEOUT_MS) {
        if (retryCount < MAX_RETRIES) {
            retryCount++;
            hc12.println(pendingCmd);
            pendingMs   = millis();
            ackMatchIdx = 0;
            Serial.print(F("[Retry "));
            Serial.print(retryCount);
            Serial.print('/');
            Serial.print(MAX_RETRIES);
            Serial.print(F("] "));
            Serial.println(pendingCmd);
        } else {
            Serial.print(F("[NoACK] "));
            Serial.println(pendingCmd);
            waitingAck  = false;
            ackMatchIdx = 0;
        }
    }
}
