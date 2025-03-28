```

```

# 系统API说明文档

更新日期：2025.3.28

## 适用内容

本文档详细描述了通过 Web UI 与 ESP32 单片机之间通信的 API 接口。Web UI 负责发起请求并显示响应结果，而 ESP32 处理并执行相应的命令或任务。本文档中列出的所有 API 均采用 JSON 格式的请求和响应。

## API 接口一览

| 接口                | 请求方法 | 描述                                       | 备注                                 |
| ------------------- | -------- | ------------------------------------------ | ------------------------------------ |
| `/get_status`       | `GET`    | 获取当前设备工作状态                       |                                      |
| `/get_config`       | `GET`    | 获取当前设备网络状态                       |                                      |
| `/get_time`         | `GET`    | 获取当前设备时间信息                       |                                      |
| `/get_gps`          | `GET`    | 获取当前设备位置信息                       |                                      |
| `/set_status`       | `POST`   | 设置当前设备工作状态                       |                                      |
| `/set_config`       | `POST`   | 设置当前设备网络状态                       |                                      |
| `/set_time`         | `POST`   | 设置当前设备时间信息                       | 若数据来源为gps模块，则可以删去此api |
| `/set_gps`          | `POST`   | 设置当前设备位置信息                       | 若数据来源为gps模块，则可以删去此api |
| `/get_RA_DEC_Float` | `GET`    | 获取当前跟踪星体的赤经和赤纬信息。浮点格式 |                                      |
| `/get_RA_DEC_HDMS`  | `GET`    | 获取当前跟踪星体的赤经和赤纬信息。HDMS格式 |                                      |
| `/set_RA_DEC_Float` | `POST`   | 设置当前跟踪星体的赤经和赤纬信息。浮点格式 |                                      |
| `/set_RA_DEC_HDMS`  | `POST`   | 设置当前跟踪星体的赤经和赤纬信息。HDMS格式 |                                      |
| `/get_EfuseMac`     | `GET`    | 获取当前设备的芯片ID                       |                                      |

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
    "month":5,      // 5月
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
    "month":5,      // 5月
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

### `/get_RA_DEC_Float`

- **方法：** `GET`
- **描述：** 获取当前跟踪星体的赤经和赤纬信息。浮点格式
- **请求参数：** 无
- **响应：**

```json
{

    "ra":13.25,    // 赤经
    "dec":45.75   // 赤纬
}
```

### `/get_RA_DEC_HDMS`

- **方法：** `GET`
- **描述：** 获取当前跟踪星体的赤经和赤纬信息。HDMS格式
- **请求参数：** 无
- **响应：**

```json
{

    "ra_h":13,    // 赤经
    "ra_m":15,    // 赤经
    "ra_s":30.0,    // 赤经
  
    "dec_d":45,   // 赤纬
    "dec_m":30,   // 赤纬
    "dec_s":0.0,   // 赤纬

}
```

### `/set_RA_DEC_Float`

- **方法：** `POST`
- **描述：** 设置当前跟踪星体的赤经和赤纬信息。浮点格式
- **请求参数：**

```json
{
    "ra":13.25,    // 赤经
    "dec":45.75   // 赤纬
}
```

- **响应：**

```json
{
    "status":"OK"
}
```

### `/set_RA_DEC_HDMS`

- **方法：** `POST`
- **描述：** 设置当前跟踪星体的赤经和赤纬信息。HDMS格式
- **请求参数：**

```json
{

    "ra_h":13,    // 赤经
    "ra_m":15,    // 赤经
    "ra_s":30.0,    // 赤经
  
    "dec_d":45,   // 赤纬
    "dec_m":30,   // 赤纬
    "dec_s":0.0,   // 赤纬

}
```

- **响应：**

```json
{
    "status":"OK"
}
```

### `/get_EfuseMac`

- **方法：** `GET`
- **描述：** 获取当前 ESP32 设备的芯片ID。
- **请求参数：** 无
- **响应：**

```json
{
    "EfuseMac":"24D2F344C9AB"
}
```

## 错误代码

* 400 Bad Request：请求参数不完整或无效。
* 401 Unauthorized：缺少或提供了无效的身份验证信息。
* 404 Not Found：请求的资源不存在。
* 500 Internal Server Error：服务器端发生内部错误。

## 版本历史

* v1.0.0:初始版本，包含基础接口 `/get_status`、`/get_config`、`/get_time` 和 `/get_gps`以及 `/set_status`、`/set_config`、`/set_time` 和 `/set_gps`。
* v1.0.1：新增 `/get_coordinate`、`/move_relative`和 `/move_absolute`三个接口
* v1.0.2：更新了 `/move_relative`和 `/move_absolute`两个接口调用时电机忙碌状态下的返回json
* v1.1.0：新增了 `/get_RA_DEC_Float`、`/get_RA_DEC_HDMS`、`/set_RA_DEC_Float`和 `/set_RA_DEC_HDMS`四个接口。删除了 `/get_coordinate`、`/move_relative`和 `/move_absolute`。

### 更新内容：

- 增加了 `/set_status`、`/set_config`、`/set_time` 和 `/set_gps` 的 POST 请求示例。
- 增加了 `/get_coordinate`、`/move_relative`和 `/move_absolute`的相关接口示例
- 更新了 `/move_relative`和 `/move_absolute`的相关接口示例
- 更新了 `/get_RA_DEC_Float`、`/get_RA_DEC_HDMS`、`/set_RA_DEC_Float`和 `/set_RA_DEC_HDMS`的相关接口示例

如果还有其他需求或接口说明，请随时调整和完善。
