// GPIO 逐引脚测试
// 开发板: Arduino UNO R4 Minima
//
// 行为:
//   D2 ~ D10 逐个测试
//   每个引脚: 先 HIGH 1s，再 LOW 1s，然后切换到下一个引脚
//   串口监视器 115200 查看当前状态

const uint8_t PINS[] = {2, 3, 4, 5, 6, 7, 8, 9, 10};
const uint8_t PIN_COUNT = sizeof(PINS) / sizeof(PINS[0]);

uint8_t currentPin = 0;
bool pinHigh = true;
unsigned long lastMs = 0;

void setup() {
    Serial.begin(115200);

    for (uint8_t i = 0; i < PIN_COUNT; i++) {
        pinMode(PINS[i], OUTPUT);
        digitalWrite(PINS[i], LOW);
    }

    // 第一个引脚拉高
    digitalWrite(PINS[0], HIGH);
    lastMs = millis();

    Serial.println("GPIO test started. D2-D10.");
    Serial.println("---");
    Serial.println("[D2] HIGH");
}

void loop() {
    unsigned long now = millis();
    if (now - lastMs < 1000) return;
    lastMs = now;

    // 关闭当前引脚
    digitalWrite(PINS[currentPin], LOW);

    if (pinHigh) {
        // 当前引脚拉高过了，现在拉低
        pinHigh = false;
        Serial.print("[D");
        Serial.print(PINS[currentPin]);
        Serial.println("] LOW");
    } else {
        // 切换到下一个引脚
        pinHigh = true;
        currentPin = (currentPin + 1) % PIN_COUNT;
        digitalWrite(PINS[currentPin], HIGH);
        Serial.print("[D");
        Serial.print(PINS[currentPin]);
        Serial.println("] HIGH");
    }
}
