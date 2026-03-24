/**********************************************************************
 * main.cpp
 *
 * 这个文件是 Minima 执行器控制板的主程序入口。
 *********************************************************************/

// 引入 Arduino 基础接口。
#include <Arduino.h>
// 引入运动控制状态机接口。
#include "MotionControl.h"
// 引入协议收发接口。
#include "Protocol.h"

// 定义从 ESP32 接收命令所使用的串口别名。
#define UART_FROM_ESP32 Serial1

// 创建运动控制对象。
MotionController motion;
// 创建协议接收器对象。
ProtocolReceiver protocolReceiver;

// 保存最近一次状态回传时间。
unsigned long tStatus = 0;
// 定义状态回传周期，单位毫秒。
const unsigned long STATUS_INTERVAL = 500;

// 定义发送状态到 ESP32 的辅助函数。
static void sendStatusToESP32(uint8_t cmd, uint8_t data0 = 0, uint8_t data1 = 0, uint8_t data2 = 0) {
    // 调用通用协议帧发送函数，把状态发到 ESP32。
    sendFrame(UART_FROM_ESP32, cmd, data0, data1, data2);
}

// 定义把当前运动状态压缩成状态字节的辅助函数。
static uint8_t getMotionStatus() {
    // 初始化状态字节为 0。
    uint8_t status = 0;

    // 如果前进子系统活跃或处于平衡阶段。
    if (motion.isForwardActive() || motion.isForwardBalancing()) {
        // 置位前进状态位。
        status |= 0x01;
    }
    // 如果转向子系统活跃或处于平衡阶段。
    if (motion.isTurnActive() || motion.isTurnBalancing()) {
        // 置位转向状态位。
        status |= 0x02;
    }
    // 如果浮沉子系统活跃。
    if (motion.isAscendActive() || motion.isDescendActive()) {
        // 置位浮沉状态位。
        status |= 0x04;
    }

    // 返回状态字节。
    return status;
}

// 定义执行命令的辅助函数。
static void executeCommand(uint8_t cmd, uint8_t data0, uint8_t data1, uint8_t data2) {
    // 显式标记这三个参数当前未使用。
    (void)data0;
    (void)data1;
    (void)data2;

    // 根据命令号做分发。
    switch (cmd) {
        // 处理旧版前进切换命令。
        case CMD_FORWARD_TOGGLE:
            // 打印调试信息。
            Serial.println(F(">>> CMD: Forward Toggle"));
            // 调用前进切换逻辑。
            motion.toggleForward();
            // 结束该分支。
            break;
        // 处理开始前进命令。
        case CMD_FORWARD_START:
            // 打印调试信息。
            Serial.println(F(">>> CMD: Forward Start"));
            // 调用开始前进逻辑。
            motion.startForwardCommand();
            // 结束该分支。
            break;
        // 处理停止前进命令。
        case CMD_FORWARD_STOP:
            // 打印调试信息。
            Serial.println(F(">>> CMD: Forward Stop"));
            // 调用停止前进逻辑。
            motion.stopForwardCommand();
            // 结束该分支。
            break;
        // 处理急停命令。
        case CMD_EMERGENCY_STOP:
            // 打印调试信息。
            Serial.println(F(">>> CMD: Emergency Stop"));
            // 调用全局急停逻辑。
            motion.emergencyStopAll();
            // 结束该分支。
            break;
        // 处理右转命令。
        case CMD_TURN_RIGHT:
            // 打印调试信息。
            Serial.println(F(">>> CMD: Turn Right"));
            // 调用右转逻辑。
            motion.startTurnRight();
            // 结束该分支。
            break;
        // 处理左转命令。
        case CMD_TURN_LEFT:
            // 打印调试信息。
            Serial.println(F(">>> CMD: Turn Left"));
            // 调用左转逻辑。
            motion.startTurnLeft();
            // 结束该分支。
            break;
        // 处理停止转向命令。
        case CMD_TURN_STOP:
            // 打印调试信息。
            Serial.println(F(">>> CMD: Turn Stop"));
            // 调用停止转向逻辑。
            motion.stopTurnCommand();
            // 结束该分支。
            break;
        // 处理上浮命令。
        case CMD_ASCEND:
            // 打印调试信息。
            Serial.println(F(">>> CMD: Ascend"));
            // 调用上浮逻辑。
            motion.startAscend();
            // 结束该分支。
            break;
        // 处理下沉命令。
        case CMD_DESCEND:
            // 打印调试信息。
            Serial.println(F(">>> CMD: Descend"));
            // 调用下沉逻辑。
            motion.startDescend();
            // 结束该分支。
            break;
        // 处理停止浮沉命令。
        case CMD_BUOYANCY_STOP:
            // 打印调试信息。
            Serial.println(F(">>> CMD: Buoyancy Stop"));
            // 调用停止浮沉逻辑。
            motion.stopBuoyancy();
            // 结束该分支。
            break;
        // 处理深度校准命令。
        case CMD_CALIBRATE_DEPTH:
            // 打印调试信息。
            Serial.println(F(">>> CMD: Calibrate Depth"));
            // 打印说明，当前深度校准不在 Minima 执行。
            Serial.println(F("    Ignored on Minima; depth sensor is read by ESP32 via CH9434A"));
            // 结束该分支。
            break;
        // 处理未知命令。
        default:
            // 打印未知命令前缀。
            Serial.print(F("??? Unknown command: 0x"));
            // 打印十六进制命令号。
            Serial.println(cmd, HEX);
            // 结束该分支。
            break;
    }
}

// 定义处理 ESP32 命令的辅助函数。
static void processESP32Command() {
    // 创建一帧协议数据结构用于接收输出。
    ProtocolFrame frame;
    // 只要串口里还能解析出完整命令，就持续处理。
    while (protocolReceiver.poll(UART_FROM_ESP32, frame)) {
        // 执行当前收到的命令。
        executeCommand(frame.cmd, frame.data0, frame.data1, frame.data2);
    }
}

// 实现 Arduino 标准 setup 函数。
void setup() {
    // 初始化本地调试串口。
    Serial.begin(115200);
    // 等待串口稳定。
    delay(500);

    // 初始化和 ESP32 通信的硬件串口。
    UART_FROM_ESP32.begin(UART_BAUD_RATE);

    // 打印启动头部。
    Serial.println(F("\n============================================"));
    // 打印系统名称。
    Serial.println(F("  Arduino Minima Actuator Controller"));
    // 打印启动尾部。
    Serial.println(F("============================================"));

    // 初始化运动控制状态机。
    motion.begin();
    // 打印状态机初始化完成提示。
    Serial.println(F("Motion Control: Initialized"));

    // 打印系统就绪提示。
    Serial.println(F("System ready! Waiting for ESP32 commands...\n"));
}

// 实现 Arduino 标准 loop 函数。
void loop() {
    // 先读取并执行来自 ESP32 的命令。
    processESP32Command();
    // 再更新运动状态机。
    motion.update();

    // 到了状态回传周期时。
    if (millis() - tStatus >= STATUS_INTERVAL) {
        // 更新最近一次状态回传时间。
        tStatus = millis();
        // 把当前运动状态回传给 ESP32。
        sendStatusToESP32(STATUS_MOTION, getMotionStatus(), 0, 0);

        // 打印状态时间戳左括号。
        Serial.print(F("["));
        // 打印当前运行秒数。
        Serial.print(millis() / 1000);
        // 打印时间戳后缀。
        Serial.print(F("s] "));
        // 打印当前状态机状态。
        motion.printStatus();
    }
}
