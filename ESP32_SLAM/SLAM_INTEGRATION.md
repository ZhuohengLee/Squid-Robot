# SLAM 集成说明

## 概述

本文件夹是 ESP32 固件的 SLAM 测试版本，修改了传感器日志格式以兼容
`SLAM/python_local_mapper.py` 占据栅格建图管线。

## 修改文件清单

| 文件 | 变更内容 |
|------|---------|
| `ImuManager.h` | 新增 `getGyroX/Y/Z()`, `hasGyro()` 接口；`_lineBuf` 扩至 128 |
| `ImuManager.cpp` | `<sog0>` → `<sog1>` 开启陀螺仪；解析 6 字段帧 `*r,p,y,gx,gy,gz` |
| `SDLogger.h` | `logSensor()` → `logControlFrame()` 26 参数全量接口 |
| `SDLogger.cpp` | 26 列 CSV：传感器全轴 + 控制状态；列名兼容 mapper |
| `ESP32.ino` | 传递全部 IMU 轴、深度控制、执行器状态到 `logControlFrame()` |

其余文件与 `ESP32/` 完全相同，未做修改。

## sensors.csv 列格式（36 列，learning 仓库训练规范）

对齐 https://github.com/ZhuohengLee/learning/blob/main/README.md

```
session_id,timestamp_ms,dt_ms,robot_mode,control_mode,
depth_valid,imu_valid,battery_v,
target_depth_cm,filtered_depth_cm,depth_speed_cm_s,depth_accel_cm_s2,
roll_deg,pitch_deg,gyro_x_deg_s,gyro_y_deg_s,gyro_z_deg_s,
front_distance_cm,left_distance_cm,right_distance_cm,
depth_err_cm,u_base,u_residual,u_total,
forward_cmd_base,forward_cmd_residual,forward_cmd_total,forward_phase_interval_ms,
yaw_cmd_base,yaw_cmd_residual,yaw_cmd_total,
buoyancy_dir_applied,buoyancy_pwm_applied,actuator_mask,balancing,emergency_stop
```

### 关键列说明

| 列名 | 说明 | 备注 |
|------|------|------|
| `session_id` | 会话 ID | 使用 session 文件夹名 |
| `control_mode` | 控制模式 | `cmdHandler.getMode()` 的枚举值 |
| `filtered_depth_cm` | 深度滤波值 | Kalman 滤波后 |
| `gyro_z_deg_s` | Z 轴角速度 | **由 AHRS yaw 差分计算**（EBIMU 原生 `<sog1>` 未生效时） |
| `u_*` | 深度控制输出 | `_base + _residual = _total`，当前 residual=0 |
| `forward_cmd_*` | 前进指令拆分 | 同上 |
| `yaw_cmd_*` | 偏航指令拆分 | 正=左转，负=右转 |
| `forward_phase_interval_ms` | 前进阀切换周期 | 当前固定 1000ms |

### 当前系统限制

项目现有的 `ForwardControl`/`LeftTurnControl`/`RightTurnControl`/`DepthController`
**没有 base/residual 拆分**（SLAM 仓库的新版控制器才有）。因此：
- `u_base = u_total = depthController.getControlOutput()`, `u_residual = 0`
- `forward_cmd_base = forward_cmd_total`（0 或 100），`forward_cmd_residual = 0`
- `yaw_cmd_base = yaw_cmd_total`（-100/0/100），`yaw_cmd_residual = 0`

接入 `ResidualInference` 和新版控制器后这些字段才有真实拆分。

### SLAM Mapper 兼容性

Python mapper 已更新支持两种列名（向后兼容）：
- `us_front_cm` / `front_cm` / `front_distance_cm` 都接受
- `motion_mode` 缺失时，从 `forward_cmd_total` / `yaw_cmd_total` 推断（阈值 5%）

### 无效值规则

- 传感器离线写 `--`（阈值 `< -9000.0f`）
- Python mapper `_read_optional_float()` 遇到 `--`/`nan`/`NaN` 返回 `None`
- 训练时 learning 仓库过滤规则：`depth_valid=0` / `imu_valid=0` / `balancing=1` / `emergency_stop=1` 的帧排除

## IMU 变更

### 配置变更
- **原**: `<sog0>` (关闭陀螺仪)
- **新**: `<sog1>` (开启陀螺仪 deg/s 输出)

### 帧格式变更
- **原**: `*roll,pitch,yaw`（3字段）
- **新**: `*roll,pitch,yaw,gyro_x,gyro_y,gyro_z`（6字段）

解析器兼容两种格式：若检测到第 4~6 字段则解析陀螺仪数据，否则仅 Euler。

## 注意事项

1. **SLAM repo 自身的列名不兼容**：squid-robot-slam 的 SDLogger 使用
   `front_distance_cm` 列名，但其 mapper 期望 `us_front_cm` 或 `front_cm`。
   本版本使用 `us_front_cm`，与 mapper 直接兼容。

2. **gyro_z_deg_s 是建图质量的关键**：没有陀螺仪数据时 mapper 退化为
   基于运动指令的粗略航向估计（`fallback_turn_rate_deg_s = 30°/s`），
   建图精度大幅下降。

3. **倾斜门控**：当 |roll| 或 |pitch| > 15° 时 mapper 跳过该帧建图，
   避免倾斜导致的超声波投影误差。

## 使用流程

```
1. 烧录 ESP32_SLAM/ 固件到 ESP32
2. 进入 TEST 模式 (mt 命令)，机器人水下运动采集数据
3. 退出 TEST 模式 (md 命令)，取出 SD 卡
4. 将 sensors.csv 复制到电脑
5. 运行 Python mapper:
   python -m SLAM.python_local_mapper path/to/sensors.csv --ascii-map
```
