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

## 使用方式

系统主要通过 `ESP32` 的 USB 串口交互。

上电后：
- 打开 `ESP32` 串口监视器
- 波特率使用 `115200`
- 输入命令后按回车

### 基本命令

```text
q         切换 MANUAL / AUTO 模式
w         前进
a         左转
d         右转
j         上浮
k         下潜
l<number> 定深到指定厘米，例如 l35
s         急停
c         将当前深度校准为 0
g         显示传感器数据
v         开关详细输出
h         显示帮助
```

### 手动模式

默认开机进入 `MANUAL` 模式。

在手动模式下：
- `w` 启动前进子系统
- `a` 触发一次左转序列
- `d` 触发一次右转序列
- `j` 触发一次上浮脉冲
- `k` 触发一次下潜脉冲
- `l35` 这类命令会进入定深控制，目标深度单位是厘米

### 自动模式

输入 `q` 可切换到 `AUTO` 模式。

自动模式下：
- 前进与转向由 `ESP32` 根据三路超声波自动决策
- 如果深度传感器正常，切换到 `AUTO` 时会自动保持当前深度
- 在 `AUTO` 模式中，普通手动运动命令会被拒绝，需要先按 `q` 回到手动模式

## 动作实现说明

### 前进如何实现

前进不是简单常开，而是由 `ForwardControl` 生成节律性输出：
- 泵持续工作
- 两个前进阀按固定周期交替进入工作相位
- 停止前进后，还会有一个短暂的平衡阶段

对应实现见：
- [ESP32/ForwardControl.cpp](/D:/working/squid%20robot/code/receiver/ESP32/ForwardControl.cpp)

### 左右转向如何实现

左转和右转由两个独立控制器管理，但它们属于同一个“转向子系统”，所以不会同时生效。

行为规则：
- `a` 会取消右转，再启动左转
- `d` 会取消左转，再启动右转
- 每次转向不是无限持续，而是一个定长动作序列
- 转向结束后会进入一个短平衡阶段

当前时序参数：
- 左转/右转主动作约 `1000 ms`
- 转向后平衡约 `200 ms`

对应实现见：
- [ESP32/LeftTurnControl.cpp](/D:/working/squid%20robot/code/receiver/ESP32/LeftTurnControl.cpp)
- [ESP32/RightTurnControl.cpp](/D:/working/squid%20robot/code/receiver/ESP32/RightTurnControl.cpp)

### 浮沉如何实现

浮沉由 `DepthController` 独立管理，不会和前进、转向抢占同一套执行器。

手动浮沉：
- `j` 会清除当前定深目标，并触发一次上浮脉冲
- `k` 会清除当前定深目标，并触发一次下潜脉冲

当前脉冲时长：
- 上浮脉冲约 `1500 ms`
- 下潜脉冲约 `1500 ms`

对应实现见：
- [ESP32/DepthController.cpp](/D:/working/squid%20robot/code/receiver/ESP32/DepthController.cpp)

### 定深如何实现

输入 `l<number>` 后，`ESP32` 会把该值设为目标深度。

例如：

```text
l35
```

表示保持在 `35 cm` 附近。

定深控制逻辑：
- 使用深度传感器的滤波深度和滤波速度
- 内部执行自适应 PID
- 根据误差和速度决定上浮还是下潜
- 每次实际执行的仍然是离散浮沉脉冲，不是连续比例阀控制

### 自动避障如何实现

自动模式下由 `AutoNavigator` 根据三路超声波距离作决策：
- 正前方过近时，停止前进并选择左转或右转
- 左侧太近时，优先右转
- 右侧太近时，优先左转
- 前方畅通时，继续前进

对应实现见：
- [ESP32/AutoNavigator.cpp](/D:/working/squid%20robot/code/receiver/ESP32/AutoNavigator.cpp)

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
