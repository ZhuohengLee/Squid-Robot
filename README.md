# Squid-Robot

双 MCU 水下机器人控制固件。

这个仓库包含两套固件：
- `ESP32`：主控，负责读取深度与超声波传感器、运行卡尔曼滤波、执行手动/自动控制逻辑，并把最终执行器掩码发送给执行端。
- `Minima`：执行端，负责接收 `ESP32` 下发的执行器命令，直接驱动引脚，并回传运动状态与心跳。

## 硬件架构

- 主控板：`ESP32-S3`
- 执行板：`Arduino UNO R4 Minima`
- 串口扩展：`CH9434A`
- 传感器：
  - 深度传感器 1 路
  - 超声波传感器 3 路

当前数据流如下：
- 深度/超声波数据进入 `ESP32`
- `ESP32` 内部完成解析、滤波、控制决策
- `ESP32` 通过高速串口把执行器位掩码发送给 `Minima`
- `Minima` 执行输出，并把状态帧回传给 `ESP32`

## 目录结构

```text
ESP32/   ESP32-S3 主控固件
Minima/  Arduino UNO R4 Minima 执行固件
```

关键模块：
- `ESP32/DepthSensorManager.*`：深度传感器读取与滤波
- `ESP32/UltrasonicManager.*`：三路超声波读取与滤波
- `ESP32/KalmanFilter.*`：位置/速度二阶卡尔曼滤波器
- `ESP32/CommandHandler.*`：串口命令解析
- `ESP32/OtaManager.*`：ESP32 OTA 管理
- `Minima/Protocol.*`：双 MCU 串口协议
- `Minima/MotionControl.*`：执行器输出控制

## 构建目标

- `ESP32`：`esp32:esp32:esp32s3`
- `Minima`：`arduino:renesas_uno:minima`

示例编译命令：

```powershell
arduino-cli compile --fqbn esp32:esp32:esp32s3 .\ESP32
arduino-cli compile --fqbn arduino:renesas_uno:minima .\Minima
```

## 烧录方式

### 首次烧录

首次需要通过 USB 烧录 `ESP32`，这样设备上才会有 OTA 固件。

### ESP32 OTA

`ESP32` 现在已经集成 OTA：
- 优先尝试连接已配置的 Wi-Fi
- 如果未配置 Wi-Fi 或连接失败，会自动启动 SoftAP 兜底

默认 OTA 参数定义在 `ESP32/OtaConfig.h`：
- `OTA_HOSTNAME = "squid-receiver-esp32"`
- `OTA_PASSWORD = "receiver-ota"`
- `OTA_AP_SSID = "SquidReceiver-OTA"`
- `OTA_AP_PASSWORD = "receiver-ota"`

如果你不想把本地 Wi-Fi 账号密码提交到仓库，可以新建：

```text
ESP32/OtaConfig.local.h
```

这个文件已被 `.gitignore` 忽略，可用于覆盖默认 OTA 配置。

## 传感器滤波

- 深度传感器由 `DepthSensorManager` 持有一个 `KalmanFilter`
- 三路超声波由 `UltrasonicManager` 分别持有三个 `KalmanFilter`
- 滤波器输出位置与速度状态，深度控制会直接使用滤波后的深度和速度

## 当前协议特性

- 固定长度 8 字节帧
- 帧头、长度、CRC、帧尾校验
- `Minima` 周期回传运动状态
- `Minima` 周期回传心跳

## 开发说明

- 主要工作目录：`ESP32/` 与 `Minima/`
- 临时构建产物建议放在 `.tmp/`
- 渲染或导出产物建议放在 `.render/`

## 许可证

本项目使用仓库根目录中的 `LICENSE`。
