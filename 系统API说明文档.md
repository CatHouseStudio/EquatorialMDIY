```

```

<!-- # 系统API说明文档

更新日期：2025.3.28

## 适用内容

本文档详细描述了通过 Web UI 与 ESP32 单片机之间通信的 API 接口。Web UI 负责发起请求并显示响应结果，而 ESP32 处理并执行相应的命令或任务。本文档中列出的所有 API 均采用 JSON 格式的请求和响应。

## API 接口一览

| 接口                   | 请求方法 | 描述                                       | 备注                            |
| ---------------------- | -------- | ------------------------------------------ | ------------------------------- |
| `/get_status`          | `GET`    | 获取当前设备工作状态                       |                                 |
| `/get_config`          | `GET`    | 获取当前设备网络状态                       |                                 |
| `/get_ratio_config`    | `GET`    | 获取当前设备减速箱速比                      |                                 |
| `/get_time`            | `GET`    | 获取当前设备时间信息                       | （尚未确定）                    |
| `/get_gps`             | `GET`    | 获取当前设备位置信息                       |                                 |
| `/set_status`          | `POST`   | 设置当前设备工作状态   
| `/set_ratio_config`    | `POST`     设置当前使用减速箱速比                     | 
| `/set_config`          | `POST`   | 设置当前设备网络状态                       |                                 |
| `/set_time`?            | `POST`   | 设置当前设备时间信息                       | （尚未确定）                    |
| `/set_gps`             | `POST`   | 设置当前设备位置信息                       |                                 |
| `/get_RA_DEC_Float`    | `GET`    | 获取当前跟踪星体的赤经和赤纬信息。浮点格式 |                                 |
| `/get_RA_DEC_HDMS`     | `GET`    | 获取当前跟踪星体的赤经和赤纬信息。HDMS格式 |                                 |
| `/set_RA_DEC_Float`    | `POST`   | 设置当前跟踪星体的赤经和赤纬信息。浮点格式 |                                 |
| `/set_RA_DEC_HDMS`     | `POST`   | 设置当前跟踪星体的赤经和赤纬信息。HDMS格式 |                                 |
| `/get_EfuseMac`        | `GET`    | 获取当前设备的芯片ID                       |                                 |
| `/get_TiltFusion`      | `GET`    | 获取当前相机的空间姿态                     | ESP32的SDA为Pin-21，SCL为Pin-22 |
| `/update`              |          | 固件和SPIFFS文件的OTA更新                  |                                 |
| `/get_ChipDiagnostics` | `GET`    | 获取当前ESP32芯片的相关参数                |                                 |
| `/get_SystemStatus`    | `GET`    | 获取当前系统状态                           |                                 | -->

# 系统API说明文档

更新日期：2025.5.13

## 适用内容

重新设计Web UI 与 ESP32 单片机之间通信的 API 接口。Web UI 负责发起请求并显示响应结果，而 ESP32 处理并执行相应的命令或任务。本文档中列出的所有 API 均采用 JSON 格式的请求和响应。
目前规划中的接口大致分为：配置内容的设置以及配置内容和硬件信息的获取
当前设计为前端完全承担双轴步进脉冲量计算，需考虑电机同步与初始化

## 前端API 接口一览

| 接口                   | 请求方法 | 描述                                       | 备注                            |
| ---------------------- | -------- | ------------------------------------------ | ------------------------------- |
| `/api/get_status`          | `GET`    | 获取当前设备工作状态                       |                                 |
态                |                                 |
| `/api/get_ap_config`       | `GET`    | 获取当前设备网络状态                       |                                 |
| `/api/set_ap_config`       | `GET`    | 设置当前设备网络状态                       |                                 |
| `/api/get_ratio_config`    | `GET`    | 获取当前设备减速箱速比                      |                                 |
| `/api/set_ratio_config`    | `POST`   | 设置当前使用RA轴与DEC轴的减速箱减速比                 | 
| `/api/get_TiltFusion`      | `GET`    | 获取当前相机的空间姿态                     | ESP32的SDA为Pin-21，SCL为Pin-22 |
| `/api/update`              |          | 固件和SPIFFS文件的OTA更新                  |                                 |
| `/api/get_SystemStatus`    | `GET`    | 获取当前系统状态                           |                                 |
| `/api/start_tracking`      | `GET`    | 定位到目标天体后启动RA轴同步地球自转跟踪天体                          
| `/api/pluse_to_target`      | `POST`  | 前端计算完位移量后发送给后端进行电机驱动                          
| `/api/stop_moving`         | `GET`    | 停止全部电机的运动                          
| `/api/set_current_motor_position`     | `POST`    | 初始化电机步数                          
| `/api/get_motor_status`     | `GET`    | 获取当前电机位置                       

## 接口详细说明

### `/api/get_status`

- **方法：** `GET`
- **描述：** 获取当前 ESP32 设备的工作状态。
- **请求参数：** 无
- **响应：**

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
- **描述：** 获取当前 ESP32 设备的网络状态。
- **请求参数：** 无
- **响应：**

```json response
{
    "ap_ssid":"ESP32-Access-Point",     // 设备WiFi-AP模式下的SSID
    "ap_password":"123456789",          // 设备WiFi-AP模式下的Password
}
```
### `/get_ratio_config`

- **方法：** `GET`
- **描述：** 获取当前赤道仪减速箱减速比。
- **请求参数：** 无
- **响应：**

```json response
{
    "ratio_RA":"50",     // RA轴减速箱减速比
    "ratio_DEC":"50",    // DEC轴减速箱减速比
    
}
```

### `/set_ap_config`

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
### `/api/set_ratio_config`

- **方法：** `POST`
- **描述：** 设置当前赤道仪减速箱减速比。
- **请求参数：**

```json
{
    "RA_ratio":"50",     // RA轴减速箱减速比
    "DEC_ratio":"50",     // DEC轴减速箱减速比
   
}
```

- **响应：**

```json
{
    "status":"OK"
}
```

<!-- ### `/get_RA_DEC_Float`

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
``` -->

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

### `/get_TiltFusion`

- **方法：** `GET`
- **描述：** 获取当前相机的空间姿态。
- **请求参数：** 无
- **响应：**

```json
{
    "roll":1.1,         // 横向翻转角	 ≈ 0°（不歪）
	"pitch" :2.2,       // 俯仰角	    ≈ -纬度（朝上仰）
	"ztilt" :3.3        // 总倾角	    ≈ 纬度（理论上约等于 pitch 的绝对值）
}
```

### `/get_ChipDiagnostics`

- **方法：** `GET`
- **描述：** 获取当前ESP32芯片的相关参数。
- **请求参数：** 无
- **响应：**

```json
{
  "chip": {
    "model": "ESP32",
    "cores": 2,
    "revision": 3,
    "psram": false,
    "features": {
      "wifi": true,
      "bt": true,
      "ble": true
    }
  },
  "clock": {
    "cpu_mhz": 240
  },
  "flash": {
    "size_mb": 4
  }
}
```

### `/get_SystemStatus`

- **方法：** `GET`
- **描述：** 获取当前系统状态
- **请求参数：** 无
- **响应：**

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
### `/api/pluse_to_target`
- **方法：** `POST`
- **描述：**  电机需要移动的脉冲量
- **请求参数：** 
- **响应：**

```json
{
    "RA": {
        "step": 114514,
        "dir": 0 //0=顺时针，1=逆时针，2=停止
    },
    "DEC": {
        "steps": 1145,
        "dir": 1
    },
    "tracking": true //决定移动之后是否开启RA轴自转同步进行追踪
}
```
- **响应**

```json response
{
    "status": "moving",
}
```
### `/api/start_tracking`
- **方法：** `GET`
- **描述：**  RA开始同步自转跟踪
- **请求参数：** 无
- **响应：**

```json
{
  
}
```
- **响应**

```json response
{
    "status": "tracking"
}
```
### `/api/stop_moving`
- **方法：** `GET`
- **描述：**  停止全部电机的运动
- **请求参数：** 无
- **响应：**

```json
{
  
}
```
- **响应**

```json response
{
    "status": "stopped"
}
```
### `/api/set_current_motor_position`
- **方法：** `POST`
- **描述：**  初始化当前电机位置
- **请求参数：** 
- **响应：**

```json
{
    "RA":0,
    "DEC":0
  
}
```
- **响应**

```json response
{
    "status": "ok"
}
```
### `/api/get_motor_status`
- **方法：** `POST`
- **描述：**  获取当前电机位置
- **请求参数：** 
- **响应：**

```json
{
}
```
- **响应**

```json response
{
    "RA": 114,
    "DEC": 514
}


## 错误代码

* 400 Bad Request：请求参数不完整或无效。
* 401 Unauthorized：缺少或提供了无效的身份验证信息。
* 404 Not Found：请求的资源不存在。
* 409 Conflict：请求与服务器的当前状态冲突。
* 415 Unsupported Media Type：不支持请求数据的媒体格式。
* 500 Internal Server Error：服务器端发生内部错误。

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
    

### 更新内容：

- 增加了 `/set_status`、`/set_config`、`/set_time` 和 `/set_gps` 的 POST 请求示例。
- 增加了 `/get_coordinate`、`/move_relative`和 `/move_absolute`的相关接口示例
- 更新了 `/move_relative`和 `/move_absolute`的相关接口示例
- 增加了 `/get_EfuseMac`接口的请求示例
- 更新了 `/get_RA_DEC_Float`、`/get_RA_DEC_HDMS`、`/set_RA_DEC_Float`和 `/set_RA_DEC_HDMS`的相关接口示例
- 更新了 `/get_TiltFusion`接口示例
- 增加了一个尚未测试的 `void DelayUs(uint64_t us)`，使用了定时器和FreeRTOS的信号机制，理论效果应该是相当于不存在的 `vTaskDelayMicroseconds(uint32_t us)`
- 在 `/set_status`用注释模拟了一段调用RA和DEC两个电机的方法
- 增加了 `/get_ChipDiagnostics`和`/get_SystemStatus`的相关接口示例
- 对所有接口增加了全局日志中间件（串口输出）。
  如果还有其他需求或接口说明，请随时调整和完善。
