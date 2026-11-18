# 系统API说明文档

更新日期：2026.11.18（对应 v3 固件）

## 适用内容

本文档描述 Web UI 与 ESP32 之间的通信接口。Web UI 负责发起请求并显示响应结果，ESP32 负责执行命令并返回状态。所有 API 均采用 JSON 格式。

当前设计由前端完全承担双轴步进脉冲量计算，固件只执行"走多少步"和"一直走"，不做坐标解算。

## 通用约定

| 项 | 说明 |
|---|---|
| Base URL | AP 模式默认 `http://192.168.4.1`；STA 模式为设备获取到的局域网 IP |
| Content-Type | 请求 `application/json`；响应多数为 `application/json`，个别错误分支实际返回纯文本 |
| 请求体 | **必须一次送达**，不支持分片/chunked |

## 前端API 接口一览

| 接口                                | 请求方法 | 描述                                         | 状态 / 备注                              |
| ----------------------------------- | -------- | -------------------------------------------- | ---------------------------------------- |
| `/api/get_status`                   | `GET`    | 获取当前设备工作状态                         | **未实现**（500）                        |
| `/api/get_ap_config`                | `GET`    | 读取 SPIFFS 中的 AP 配置                     | 已完成                                   |
| `/api/set_ap_config`                | `POST`   | 设置 AP 配置并重启热点                       | 已完成，ssid 非空、密码 8~63 或留空      |
| `/api/get_ratio_config`             | `GET`    | 获取当前设备减速箱速比                       | **已废弃**（500）                        |
| `/api/set_ratio_config`             | `POST`   | 设置当前使用RA轴与DEC轴的减速箱减速比        | **已废弃**（500）                        |
| `/api/get_TiltFusion`               | `GET`    | 获取当前相机的空间姿态                       | 已完成，SDA=21 / SCL=22                  |
| `/update`                           | `GET`    | 固件和SPIFFS文件的OTA更新                    | 专用页面                                 |
| `/get_SystemStatus`                 | `GET`    | 获取当前系统状态                             | 已完成，**路径无 `/api` 前缀**           |
| `/get_EfuseMac`                     | `GET`    | 获取 ESP32 芯片 ID                           | 已完成，**路径无 `/api` 前缀**           |
| `/get_ChipDiagnostics`              | `GET`    | 获取芯片与时钟信息                           | 已完成，**路径无 `/api` 前缀**           |
| `/api/start_tracking`               | `GET`    | 定位到目标天体后启动RA轴同步地球自转跟踪天体 | **未实现**（500）                        |
| `/api/pluse_to_target`              | `POST`   | 前端计算完位移量后发送给后端进行电机驱动     | 已完成，双轴独立下发                     |
| `/api/stop_moving`                  | `GET`    | 停止全部电机的运动                           | 已完成                                   |
| `/api/set_current_motor_position`   | `POST`   | 初始化电机步数                               | **未实现**（500）                        |
| `/api/get_motor_status`             | `GET`    | 获取当前电机位置                             | **未实现**（500）                        |

> ⚠️ `/get_SystemStatus`、`/get_EfuseMac`、`/get_ChipDiagnostics` 三条路径**没有 `/api` 前缀**，与其余接口不一致，调用时注意。

## 电机命令语义

固件内部只有三种动作，每根轴同一时刻只允许一个动作在执行。

| 动作 | 触发方式 | 行为 |
|---|---|---|
| 定量定位 | `pluse_to_target`，`tracking=false` | 走 `steps` 步后自停 |
| 定位 + 持续跟踪 | `pluse_to_target`，`tracking=true` | 先走 `steps` 步，再以固定速度一直走 |
| 纯持续运行 | `pluse_to_target`，`steps=0` + `tracking=true` | 跳过定位段，直接一直走 |
| 停止 | `GET /api/stop_moving` | 立即停止 |

**并发规则**

- 轴正在运动时，再收到定位/持续命令会被**拒绝**（不排队、不覆盖），响应里该轴标记为 `"busy"`；
- **只有 STOP 能打断**正在执行的动作；
- 两轴**相互独立**：RA 忙不影响 DEC 下发，反之亦然。

**停止行为**：先取消队列中未启动的命令，再同步断开脉冲输出，**立即不再产生任何 STEP 脉冲**。停止后电机仍带保持力矩（驱动器 EN 常接地，固件不控制使能）。OTA 升级期间停止接口仍然可用。

## 接口详细说明

### `/api/get_status`

- **方法：** `GET`
- **状态：** **未实现**，固定返回 `500` 与 `API not Implement!!!`

设计中的响应（供参考）：

```json
{
    "current_ra": 15.55,
    "current_dec": 78.88,
    "stepmotor_status": {
        "RA": "idle",
        "DEC": "moving"
    },
    "tracking": false
}
```

### `/api/get_ap_config`

- **方法：** `GET`
- **描述：** 读取 SPIFFS 中的 `/Config.json`。
- **请求参数：** 无
- **响应：** `200`

```json
{
    "ssid": "ESP32-Access-Point",
    "pwd": "123456789"
}
```

- **错误：** `400`（纯文本 `Invalid JSON on SPIFFS`）—— 文件不存在或内容无法解析。

> ⚠️ 该接口**明文返回 AP 密码**。

### `/api/get_ratio_config`

- **方法：** `GET`
- **状态：** **已废弃**，固定返回 `500` 与 `API Deprecated!!!`

### `/api/set_ap_config`

- **方法：** `POST`
- **描述：** 写入 `/Config.json` 并立即重启热点。
- **请求参数：**

```json
{
    "ssid": "ESP32-Access-Point",
    "pwd": "123456789"
}
```

| 字段 | 约束 |
|---|---|
| `ssid` | 非空，≤ 32 字符 |
| `pwd` | 留空表示开放网络；非空时须 **8~63 字符**（WPA2 硬约束） |

- **响应：** `200`

```json
{
    "status": "OK"
}
```

- **错误：** `400` JSON 非法 / `ssid` 为空 / 密码长度不在 8~63。

设置成功后热点会以新凭据重启，**已连接的客户端会断开**。

### `/api/set_ratio_config`

- **方法：** `POST`
- **状态：** **已废弃**，固定返回 `500` 与 `API Deprecated!!!`

### `/get_EfuseMac`

- **方法：** `GET`
- **描述：** 获取当前 ESP32 设备的芯片 ID。
- **请求参数：** 无
- **响应：** `200`

```json
{
    "EfuseMac": "24D2F344C9AB"
}
```

> 路径是 `/get_EfuseMac`，**没有 `/api` 前缀**。

### `/api/get_TiltFusion`

- **方法：** `GET`
- **描述：** 获取当前相机的空间姿态，单位：度。
- **请求参数：** 无
- **响应：** `200`

```json
{
    "roll": 1.1,
    "pitch": -31.2,
    "ztilt": 31.2
}
```

- **错误：** `503`（纯文本 `No MPU data`）—— 传感器尚未产出数据。

### `/get_ChipDiagnostics`

- **方法：** `GET`
- **描述：** 获取当前 ESP32 芯片的相关参数。
- **请求参数：** 无
- **响应：** `200`

```json
{
    "chip": {
        "model": "ESP32",
        "cores": 2,
        "revision": 3,
        "psram": 0
    },
    "clock": {
        "cpu_mhz": 240
    },
    "flash": {
        "size_mb": 4
    }
}
```

| 字段 | 说明 |
|---|---|
| `chip.model` | 芯片型号字符串 |
| `chip.cores` | 核心数 |
| `chip.revision` | 芯片版本号 |
| `chip.psram` | PSRAM 容量，**字节**；无 PSRAM 时为 `0`（旧版文档写的是布尔值，已变更） |
| `clock.cpu_mhz` | CPU 主频 |
| `flash.size_mb` | Flash 容量 |

> 路径是 `/get_ChipDiagnostics`，**没有 `/api` 前缀**。

### `/get_SystemStatus`

- **方法：** `GET`
- **描述：** 获取当前系统状态。
- **请求参数：** 无
- **响应：** `200`

```json
{
    "uptime_ms": 91269,
    "heap": {
        "total_bytes": 328508,
        "free_bytes": 162596,
        "min_free_bytes": 106608
    },
    "rtos": {
        "tick_rate_hz": 1000,
        "task_count": 15
    }
}
```

> 路径是 `/get_SystemStatus`，**没有 `/api` 前缀**。

### `/api/pluse_to_target`

- **方法：** `POST`
- **描述：** 下发双轴运动命令，两轴各自独立评估、独立下发。

**请求参数：**

```json
{
    "RA": {
        "steps": 114514,
        "dir": 0
    },
    "DEC": {
        "steps": 1145,
        "dir": 1
    },
    "tracking": true
}
```

| 字段 | 类型 | 必填 | 说明 |
|---|---|---|---|
| `RA.steps` | uint32 | 是 | RA 轴定位脉冲数；`0` 表示跳过定位段 |
| `RA.dir` | uint32 | 是 | `0` = 工作方向，`1` = 初始化方向 |
| `DEC.steps` | uint32 | 是 | DEC 轴定位脉冲数 |
| `DEC.dir` | uint32 | 是 | 同上 |
| `tracking` | bool | 否 | 仅对 RA 有效。`true` 表示 RA 定位完成后继续持续运行；DEC 恒为 `false` |

**说明**

- `dir` **只接受 0 和 1**，其他值返回 400。停止请用 `/api/stop_moving`，不要靠 `dir`。
- 持续运行的**速度由固件常量 `Stepper_*_StepHalfPeriodUs` 决定**，当前 API 不提供运行时调速。

**响应（至少一轴被接受）：** `200`

```json
{
    "status": "moving",
    "RA": "moving",
    "DEC": "busy"
}
```

`RA` / `DEC` 字段取值：`"moving"`（已接受）或 `"busy"`（该轴正在运动）。

**响应（两轴都被拒绝）：** `409`

```json
{
    "status": "busy",
    "RA": "busy",
    "DEC": "busy"
}
```

**错误**

| 码 | 场景 |
|---|---|
| `400` | JSON 非法 |
| `400` | `dir` 不是 0/1 |
| `409` | 两轴都忙 |
| `409` | OTA 升级进行中（正文为纯文本 `OTA in progress`） |

### `/api/start_tracking`

- **方法：** `GET`
- **状态：** **未实现**，固定返回 `500` 与 `API not Implement!!!`

### `/api/stop_moving`

- **方法：** `GET`
- **描述：** 停止全部电机的运动。
- **请求参数：** 无
- **响应：** `200`

```json
{
    "status": "stopped"
}
```

无论当前是否有电机在动，都返回这个结果。

### `/api/set_current_motor_position`

- **方法：** `POST`
- **状态：** **未实现**，固定返回 `500` 与 `API not Implement!!!`

### `/api/get_motor_status`

- **方法：** `GET`
- **状态：** **未实现**，固定返回 `500` 与 `API not Implement!!!`

## 错误代码

* 400 Bad Request：请求参数不完整、类型错误、取值越界或 JSON 非法。
* 409 Conflict：电机忙，或 OTA 升级进行中。
* 415 Unsupported Media Type：不支持请求数据的媒体格式（预留，当前未使用）。
* 500 Internal Server Error：接口未实现或已废弃。
* 503 Service Unavailable：传感器数据尚未就绪。

## 已知问题

1. **错误响应类型不一致**：多数错误分支以 `application/json` 作 Content-Type，但正文是纯文本（如 `OTA in progress`、`Invalid JSON`），前端按 JSON 解析会失败。
2. **`/api/get_ap_config` 明文返回密码**。
3. **三条接口缺 `/api` 前缀**：`/get_SystemStatus`、`/get_EfuseMac`、`/get_ChipDiagnostics`。
4. **不支持分片请求体**：超过单个回调分片的 body 会解析失败。
5. **持续运行速度不可通过 API 调整**，只能改固件常量后重新编译。
6. **WebSocket 不可用**：`/ws` 已声明但未注册到服务器。
7. **无绝对位置**：固件不记录电机步数，`/api/get_motor_status` 与 `/api/set_current_motor_position` 因此无法实现。

## 版本历史

* v1.0.0:初始版本，包含基础接口 `/get_status`、`/get_config`、`/get_time` 和 `/get_gps`以及 `/set_status`、`/set_config`、`/set_time` 和 `/set_gps`。
* v1.0.1：新增 `/get_coordinate`、`/move_relative`和 `/move_absolute`三个接口
* v1.0.2：更新了 `/move_relative`和 `/move_absolute`两个接口调用时电机忙碌状态下的返回json
* v1.0.3：新增了 `/get_EfuseMac`接口，用于获得esp32设备的唯一id。
* v1.1.0：新增了 `/get_RA_DEC_Float`、`/get_RA_DEC_HDMS`、`/set_RA_DEC_Float`和 `/set_RA_DEC_HDMS`四个接口。删除了 `/get_coordinate`、`/move_relative`和 `/move_absolute`。
* v1.1.1：新增了 `/get_TiltFusion`接口，用于获取当前相机的空间姿态。
* v1.1.2：调整了HTTP_GET部分的路由，优化代码可读性和维护性。在Configuration.h头部的注释部分补充了DM542C驱动的细分表。按照FreeRTOS的文档建议，调整了 `task_Move_RA`和 `task_Move_DEC`的 `parameters`传递方式。
* v1.1.3：新增了 `/update`接口的备忘说明。
* v1.1.4：新增了 `/get_ChipDiagnostics`，用于获取当前ESP32芯片的相关参数。新增了`/get_SystemStatus`，用于获取当前系统状态。
* v2.0.0 重构了部分API以适应新架构，当前计算量将从后端转移至前端本地,由计算得出需要向后端传递的脉冲量驱使点击运动，当前API设计的问题：
    -如何在前端实现电机步数同步（得出电机现有位置）
    -如何初始化电机位置
    -是否提供同步API以纠正可能的手工调整或自然扰动
    -是否提供手动同步API
* **v3（2026.11.18）**
  - 步进控制改用 FastAccelStepper，脉冲由 RMT 硬件产生，不再由任务软件翻转
  - 停止改为"入队 + 同步断脉冲"，不再有约 20 ms 的残余脉冲
  - 修复双轴命令互相牵连的问题（原 `&&` 短路会让 DEC 收不到命令）
  - 字段统一为 `steps`（原 RA 为 `step`、DEC 为 `steps`）
  - `dir` 增加校验，只接受 0/1；停止改用 `/api/stop_moving`
  - `pluse_to_target` 响应增加 `RA` / `DEC` 字段，说明各轴结果
  - OTA 期间拒绝运动命令
  - `set_ap_config` 增加 ssid / 密码校验
  - SPIFFS 接口改为互斥量串行化，读失败可被上层识别
  - `/api/get_TiltFusion` 修复（此前恒返回 503）
