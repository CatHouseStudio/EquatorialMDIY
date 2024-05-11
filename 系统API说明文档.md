```

```

# 系统API说明文档

更新日期：2024.5.10

## 适用内容

本文档详细描述了通过 Web UI 与 ESP32 单片机之间通信的 API 接口。Web UI 负责发起请求并显示响应结果，而 ESP32 处理并执行相应的命令或任务。本文档中列出的所有 API 均采用 JSON 格式的请求和响应。

## API 接口一览

| 接口            | 请求方法 | 描述                 |
| --------------- | -------- | -------------------- |
| `/get_status` | `GET`  | 获取当前设备工作状态 |
| `/get_config` | `GET`  | 获取当前设备网络状态 |
| `/get_time`   | `GET`  | 获取当前设备时间信息 |
| `/get_gps`    | `GET`  | 获取当前设备位置信息 |
| `/set_status` | `POST` | 设置当前设备工作状态 |
| `/set_status` | `POST` | 设置当前设备网络状态 |
| `/set_time`   | `POST` | 设置当前设备时间信息 |
| `/set_gps`    | `POST` | 设置当前设备位置信息 |
|                 |          |                      |
|                 |          |                      |

## 接口详细说明

### `/get_status`

- **方法：** `GET`
- **描述：** 获取当前 ESP32 设备的工作状态。
- **请求参数：** 无
- **响应：**

```json
{
    "uptime":5000,      // 设备运行时间（秒）
    "direction":2,      // 单电机运行方向（0 = 顺时针，1 = 逆时针，2 = 停止）
    "speed":12.3        // 单电机运行速度（脉冲赫兹）
}
```

### `/get_config`

- **方法：** `GET`
- **描述：** 获取当前 ESP32 设备的网络状态。
- **请求参数：** 无
- **响应：**

```json
{
    "ap_ssid":"ESP32-Access-Point",     // 设备WiFi-AP模式下的SSID
    "ap_password":"123456789",          // 设备WiFi-AP模式下的Password
}
```

### `/get_time`

- **方法：** `GET`
- **描述：** 获取当前 ESP32 设备的时间信息。
- **请求参数：** 无
- **响应：**

```json
{
    "year":2024,    // 2024年
    "month":4,      // 5月（根据POSIX.1-2008标准，从0开始计数，即0表示1月）
    "day":10,       // 10日
    "hour":20,      // 20时
    "minute":30,    // 30分
    "second":27,    // 27秒
}
```

### `/get_gps`

- **方法：** `GET`
- **描述：** 获取当前 ESP32 设备的位置信息。
- **请求参数：** 无
- **响应：**

```json
{
    "longitude":121.93526597,   // 经度
    "latitude":30.92149876      // 纬度
}
```

### `/set_status`

- **方法：** `POST`
- **描述：** 设置当前 ESP32 设备的工作状态。
- **请求参数：** 
  
```json
{
    "direction":0,      // 单电机运行方向（0 = 顺时针，1 = 逆时针，2 = 停止）
    "speed":12.3        // 单电机运行速度（脉冲赫兹）
}
```

- **响应：**

```json
{
    "status":"OK"
}
```

### `/set_config`

- **方法：** `POST`
- **描述：** 设置当前 ESP32 设备的网络状态。
- **请求参数：** 
  
```json
{
    "ap_ssid":"ESP32-Access-Point",     // 设备WiFi-AP模式下的SSID
    "ap_password":"123456789",          // 设备WiFi-AP模式下的Password
}
```

- **响应：**

```json
{
    "status":"OK"
}
```

### `/set_time`

- **方法：** `POST`
- **描述：** 设置当前 ESP32 设备的时间信息。
- **请求参数：** 
  
```json
{
    "year":2024,    // 2024年
    "month":4,      // 5月（根据POSIX.1-2008标准，从0开始计数，即0表示1月）
    "day":10,       // 10日
    "hour":20,      // 20时
    "minute":30,    // 30分
    "second":27,    // 27秒
}
```

- **响应：**

```json
{
    "status":"OK"
}
```

### `/set_gps`

- **方法：** `POST`
- **描述：** 设置当前 ESP32 设备的位置信息。
- **请求参数：** 
  
```json
{
    "longitude":121.93526597,   // 经度
    "latitude":30.92149876      // 纬度
}
```

- **响应：**

```json
{
    "status":"OK"
}
```


## 错误代码

* 400 Bad Request：请求参数不完整或无效。
* 401 Unauthorized：缺少或提供了无效的身份验证信息。
* 404 Not Found：请求的资源不存在。
* 500 Internal Server Error：服务器端发生内部错误。

## 版本历史

* v1.0.0:初始版本，包含基础接口 `/get_status`、`/get_config`、`/get_time` 和 `/get_gps`以及`/set_status`、`/set_config`、`/set_time` 和 `/set_gps`。

### 更新内容：
- 增加了 `/set_status`、`/set_config`、`/set_time` 和 `/set_gps` 的 POST 请求示例。

如果还有其他需求或接口说明，请随时调整和完善。