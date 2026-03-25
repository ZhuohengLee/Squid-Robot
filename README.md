# Squid-Robot

双 MCU 水下机器人控制固件。

本仓库包含两套固件：

- `ESP32/`
  主控。负责超声波读取、深度滤波、手动/自动控制、OTA，以及把执行器掩码发送给 `Minima`。
- `Minima/`
  执行端。负责直接驱动气泵和电磁阀，同时通过 `A4/A5` 读取 `MS5837` 水压传感器，并把深度数据回传给 `ESP32`。

## 当前架构

当前真实数据链路如下：

1. `MS5837` 接在 `Arduino UNO R4 Minima` 的 `A4/A5`
2. `Minima` 读取压力和温度，做补偿计算，并把深度值通过高速串口回传给 `ESP32`
3. `ESP32` 接收深度帧后做卡尔曼滤波，并用于显示、定深和自动模式
4. 三路超声波接到 `CH9434A`
5. `ESP32` 读取三路超声波，完成避障决策
6. `ESP32` 把最终执行器位掩码发给 `Minima`
7. `Minima` 只负责按位输出到真实引脚

一句话概括：

- 深度传感器在 `Minima`
- 超声波在 `ESP32`
- 控制决策在 `ESP32`
- 执行输出在 `Minima`

## 硬件连接

### 控制板

- 主控板：`ESP32-S3`
- 执行板：`Arduino UNO R4 Minima`
- 串口扩展：`CH9434A`

### 深度传感器

- 传感器型号：`MS5837-02BA`
- 接线位置：`Minima`
- I2C 引脚：
  - `A4 -> MS5837 SDA`
  - `A5 -> MS5837 SCL`

注意：

- `MS5837` 是 `3.3V` 器件
- 如果你用的是裸传感器或 3.3V 模块，不要把 I2C 上拉到 `5V`
- 如果模块板本身已经做了电平转换和稳压，再按该模块说明接线

### 超声波传感器

三路超声波通过 `CH9434A` 接到 `ESP32`：

- `UART1 -> Front`
- `UART2 -> Left`
- `UART3 -> Right`

### Minima 执行器引脚

当前引脚定义见 [Minima/PinDefinitions.h](/D:/working/squid%20robot/code/receiver/Minima/PinDefinitions.h)。

前进子系统：

- `PUMP_A = 2`
- `VALVE_B = 6`
- `VALVE_C = 7`

转向子系统：

- `PUMP_D = 5`
- `VALVE_E = 3`
- `VALVE_F = 4`

浮沉子系统：

- `PUMP_G = 8`
- `VALVE_H = 9`
- `VALVE_I = 10`

## 目录结构

```text
ESP32/   ESP32-S3 主控固件
Minima/  Arduino UNO R4 Minima 执行固件
```

关键模块：

- [ESP32/CommandHandler.cpp](/D:/working/squid%20robot/code/receiver/ESP32/CommandHandler.cpp)
  串口命令解析
- [ESP32/AutoNavigator.cpp](/D:/working/squid%20robot/code/receiver/ESP32/AutoNavigator.cpp)
  自动避障
- [ESP32/DepthController.cpp](/D:/working/squid%20robot/code/receiver/ESP32/DepthController.cpp)
  浮沉和定深控制
- [ESP32/DepthSensorManager.cpp](/D:/working/squid%20robot/code/receiver/ESP32/DepthSensorManager.cpp)
  接收 `Minima` 回传深度并滤波
- [ESP32/UltrasonicManager.cpp](/D:/working/squid%20robot/code/receiver/ESP32/UltrasonicManager.cpp)
  三路超声波读取与诊断
- [Minima/DepthSensorLink.cpp](/D:/working/squid%20robot/code/receiver/Minima/DepthSensorLink.cpp)
  `MS5837` 采集和深度回传
- [Minima/MotionControl.cpp](/D:/working/squid%20robot/code/receiver/Minima/MotionControl.cpp)
  执行器 GPIO 输出
- [Minima/Protocol.cpp](/D:/working/squid%20robot/code/receiver/Minima/Protocol.cpp)
  双 MCU 固定长度串口协议

## 编译目标

- `ESP32`：`esp32:esp32:esp32s3`
- `Minima`：`arduino:renesas_uno:minima`

示例编译命令：

```powershell
arduino-cli compile --fqbn esp32:esp32:esp32s3 .\ESP32
arduino-cli compile --fqbn arduino:renesas_uno:minima .\Minima
```

## 烧录方式

### 第一次烧录

第一次建议两块板都走 USB：

1. 先把 `Minima` 固件烧进去
2. 再把 `ESP32` 固件烧进去
3. 上电后打开 `ESP32` 串口观察日志

### 后续更新

- `Minima` 继续通过 USB 烧录
- `ESP32` 可以继续 USB，也可以用 OTA

### ESP32 OTA

`ESP32` 已经集成 OTA。

默认行为：

- 优先尝试连接已配置的 Wi-Fi
- 如果没配网或连不上，会自动启动 `SoftAP`

默认 OTA 配置在 [ESP32/OtaConfig.h](/D:/working/squid%20robot/code/receiver/ESP32/OtaConfig.h)。

如果不想把本地 Wi-Fi 账号密码提交到仓库，可以新建：

```text
ESP32/OtaConfig.local.h
```

该文件已经被 `.gitignore` 忽略。

## 使用方式

主要通过 `ESP32` 的 USB 串口交互：

- 波特率：`115200`
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
c         以当前深度为 0 重新校准
g         打印全部传感器状态
v         开关详细回传输出
h         显示帮助
```

### MANUAL 模式

默认上电进入 `MANUAL`。

在手动模式下：

- `w` 启动前进
- `a` 触发一次左转序列
- `d` 触发一次右转序列
- `j` 触发一次上浮脉冲
- `k` 触发一次下潜脉冲
- `l<number>` 启动定深控制

### AUTO 模式

输入 `q` 进入 `AUTO`。

自动模式下：

- 前进和转向由 `AutoNavigator` 根据三路超声波自动决策
- 如果深度在线，进入 `AUTO` 时会尝试保持当前深度
- 普通手动运动命令会被拒绝，需要先按 `q` 回到手动模式

## 动作逻辑

### 前进

前进由 [ESP32/ForwardControl.cpp](/D:/working/squid%20robot/code/receiver/ESP32/ForwardControl.cpp) 生成节律控制：

- `PUMP_A` 持续打开
- `VALVE_B` 和 `VALVE_C` 每 `0.9 s` 一起切换一次
- 停止前进后会进入一个短暂平衡阶段

### 转向

左转和右转由独立控制器实现：

- [ESP32/LeftTurnControl.cpp](/D:/working/squid%20robot/code/receiver/ESP32/LeftTurnControl.cpp)
- [ESP32/RightTurnControl.cpp](/D:/working/squid%20robot/code/receiver/ESP32/RightTurnControl.cpp)

当前参数：

- 左转/右转主动作约 `1000 ms`
- 转向后的平衡阶段约 `200 ms`

### 浮沉

浮沉由 [ESP32/DepthController.cpp](/D:/working/squid%20robot/code/receiver/ESP32/DepthController.cpp) 管理：

- `j`：上浮脉冲，约 `1500 ms`
- `k`：下潜脉冲，约 `1500 ms`
- `l<number>`：进入定深模式

控制器使用：

- 滤波后的深度
- 滤波后的深度速度
- 自适应 PID

## 深度链路说明

当前深度链路如下：

1. `Minima` 通过 `Wire` 从 `MS5837` 读取压力和温度
2. `Minima` 开机约 `2 s` 后自动把当时压力作为空气零点
3. `Minima` 把深度数据通过 `STATUS_DEPTH` 帧回传给 `ESP32`
4. `ESP32` 的 [ESP32/DepthSensorManager.cpp](/D:/working/squid%20robot/code/receiver/ESP32/DepthSensorManager.cpp) 对回传深度做卡尔曼滤波
5. 输入 `c` 时，`ESP32` 会发 `CMD_CALIBRATE_DEPTH_ZERO`，要求 `Minima` 重新校零

## 超声波诊断

`ESP32` 现在会每秒打印一次超声波诊断信息。

常见状态：

- `valid`
  收到合法距离帧
- `out_of_range`
  回包合法，但距离值超出当前允许范围
- `bad_checksum`
  收到帧，但校验和不对
- `short_frame`
  收到的数据长度不够
- `no_data`
  这一 UART 完全没有任何回包

调试入口见 [ESP32/UltrasonicManager.cpp](/D:/working/squid%20robot/code/receiver/ESP32/UltrasonicManager.cpp) 和 [ESP32/SensorHub.cpp](/D:/working/squid%20robot/code/receiver/ESP32/SensorHub.cpp)。

## 当前串口协议

双 MCU 协议为固定 `8` 字节帧：

- 帧头
- 长度
- 命令字
- `data0`
- `data1`
- `data2`
- `CRC8`
- 帧尾

当前常用命令/状态：

- `CMD_SET_ACTUATORS`
- `CMD_EMERGENCY_STOP`
- `CMD_CALIBRATE_DEPTH_ZERO`
- `STATUS_MOTION`
- `STATUS_HEARTBEAT`
- `STATUS_DEPTH`

## 开发说明

- 主工作目录：`ESP32/` 和 `Minima/`
- 临时构建产物建议放在 `.tmp/`
- 渲染或导出产物建议放在 `.render/`

## 许可证

本项目使用仓库根目录中的 `LICENSE`。
