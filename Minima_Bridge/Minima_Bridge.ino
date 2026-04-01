/**********************************************************************
 * Arduino HC-12 透明串口桥  (Arduino UNO R4 Minima)
 *
 * 架构：
 *   PC  ──USB Serial──►  Minima  ──HC-12 无线──►  ESP32 机器人
 *   PC  ◄─USB Serial──   Minima  ◄─HC-12 无线──   ESP32 机器人
 *
 * 正常使用：
 *   完全透明转发。PC 发送的所有内容（w/a/d/s 等控制命令）原样经
 *   HC-12 转发给机器人，机器人的串口回显也原样转发回 PC。
 *
 * 本端信道配置命令（PC 发 → Minima 拦截处理，不转发）：
 *   HC001 ~ HC127   将本端 HC-12 切换到指定信道
 *   例：HC025       切换到信道 025
 *
 * 机器人端信道配置命令（PC 发 → 透传给机器人）：
 *   ESP001 ~ ESP127  机器人收到后自行配置其 HC-12 信道
 *   例：ESP025       让机器人切换到信道 025
 *
 * ⚠ 双端同步信道的正确顺序：
 *   1. 先发 ESP025  → 机器人切换到新信道（此时链路短暂中断）
 *   2. 再发 HC025   → 本端切换到新信道 → 链路恢复
 *   顺序反过来则本端先断开，无法再发 ESP 命令。
 *
 * HC-12 接线（Arduino UNO R4 Minima）：
 *   HC-12 TX  → D2
 *   HC-12 RX  → D3
 *   HC-12 SET → D4
 *   HC-12 VCC → 5V
 *   HC-12 GND → GND
 *********************************************************************/

#include <SoftwareSerial.h>

#define HC12_RX_PIN    2     // 接 HC-12 TX
#define HC12_TX_PIN    3     // 接 HC-12 RX
#define HC12_SET_PIN   4     // 接 HC-12 SET
#define PC_BAUD        115200
#define HC12_BAUD      9600
#define PC_BUF_MAX     64    // PC 输入行缓冲上限

SoftwareSerial hc12(HC12_RX_PIN, HC12_TX_PIN);

String pcBuffer = "";

// ────────────────────────────────────────
// 将本端 HC-12 切换到指定信道（AT 模式）
// channel: 3位字符串，如 "025"
// ────────────────────────────────────────
void configHC12Channel(const String& channel) {
    Serial.print(F("[HC-12] Setting channel to "));
    Serial.print(channel);
    Serial.print(F(" ..."));

    // 拉低 SET 脚，进入 AT 命令模式（HC-12 需要 ≥40ms 才进入）
    digitalWrite(HC12_SET_PIN, LOW);
    delay(200);

    hc12.print(F("AT+C"));
    hc12.println(channel);

    // 等待 HC-12 响应，最多 1000ms
    String response = "";
    uint32_t start = millis();
    while (millis() - start < 1000) {
        if (hc12.available()) {
            response += (char)hc12.read();
        }
    }

    // 拉高 SET 脚，退出 AT 模式，恢复透传
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
    // SET 高电平 = 透传模式
    pinMode(HC12_SET_PIN, OUTPUT);
    digitalWrite(HC12_SET_PIN, HIGH);

    Serial.begin(PC_BAUD);
    while (!Serial);   // 等待 USB CDC 串口连接（Minima 必须）
    hc12.begin(HC12_BAUD);

    Serial.println(F("============================================"));
    Serial.println(F("   Squid Robot  —  Minima HC-12 Bridge"));
    Serial.println(F("--------------------------------------------"));
    Serial.println(F(" Channel pairing"));
    Serial.println(F("   HC025   set THIS  end to ch.025"));
    Serial.println(F("   ESP025  set ROBOT end to ch.025"));
    Serial.println(F("   Order : ESP025 first, HC025 second"));
    Serial.println(F("============================================"));
}

// ────────────────────────────────────────
// 主循环
// ────────────────────────────────────────
void loop() {
    // ── PC → HC-12 ──────────────────────
    // 逐字节读入行缓冲；遇到换行符时判断是否为本端配置命令。
    // 非配置命令则原样发往 HC-12（带换行，使机器人解析器正常工作）。
    while (Serial.available()) {
        char c = Serial.read();

        if (c == '\n' || c == '\r') {
            if (pcBuffer.length() > 0) {
                // HC + 3位数字 → 本端信道配置，拦截不转发
                if (pcBuffer.length() == 5      &&
                    pcBuffer[0] == 'H'          &&
                    pcBuffer[1] == 'C'          &&
                    isDigit(pcBuffer[2])        &&
                    isDigit(pcBuffer[3])        &&
                    isDigit(pcBuffer[4])) {
                    configHC12Channel(pcBuffer.substring(2));
                } else {
                    // 其余命令透传到 HC-12
                    hc12.println(pcBuffer);
                }
                pcBuffer = "";
            }
        } else {
            pcBuffer += c;
            // 超长输入视为噪声，丢弃保护缓冲区
            if (pcBuffer.length() > PC_BUF_MAX) {
                Serial.println(F("[Bridge] Input overflow, buffer cleared."));
                pcBuffer = "";
            }
        }
    }

    // ── HC-12 → PC ──────────────────────
    // 机器人回传的所有数据字节级直接转发到 PC。
    while (hc12.available()) {
        Serial.write(hc12.read());
    }
}
