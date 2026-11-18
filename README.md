# [赤道天球仪](https://github.com/CatHouseStudio/EquatorialMDIY/blob/main/README.md#赤道天球仪-----)

本项目是一个基于ESP32开发板构建的赤道天球仪，主要用于天文星体的跟踪和观测。

## 需求设备

* ESP32开发板
* 42步进电机 ×2
* 步进驱动板 ×2（DRV8825、A4988、TMC2208、DM542C 均可，细分见 `src/Configuration.h` 顶部注释）
* MPU6050
* INA226
* SSD1306
* 电源
* 若干电线

## [使用方式](https://github.com/CatHouseStudio/EquatorialMDIY/blob/main/README.md#使用方式)

1. 安装vscode
2. 安装[PlatformIO IDE](https://platformio.org)扩展
3. git clone这个工程项目，并在vscode中打开
4. 编译并烧录固件至ESP32开发板上
5. 烧录SPIFFS文件系统：`pio run -t uploadfs`
6. 连接电路并安装机械结构

## [相关说明](https://github.com/CatHouseStudio/EquatorialMDIY/blob/main/README.md#相关说明)

* 本项目采用了Freertos
* 步进电机由 FastAccelStepper 驱动，脉冲由 RMT 硬件定时器产生，不再由任务软件翻转
* 固件只负责执行"走多少步"和"一直走"，脉冲量由前端计算
* [web_view](https://github.com/CatHouseStudio/EquatorialMDIY/tree/main/web_view)目录中为该项目的react前端代码
* [天文参数.rest](https://github.com/CatHouseStudio/EquatorialMDIY/tree/main/天文参数.rest)中存放的是几个simbad地外恒星的信息获取url
* [系统API说明文档](https://github.com/CatHouseStudio/EquatorialMDIY/tree/main/系统API说明文档.md)详细描述了本项目中通过 Web UI 与 ESP32 单片机之间通信的 API 接口

## [电机控制](https://github.com/CatHouseStudio/EquatorialMDIY/blob/main/README.md#电机控制)

* 每根轴同一时刻只允许一个动作，运动中收到新的定位/持续命令会被拒绝
* 只有停止指令能打断正在执行的动作
* 停止是立即断开脉冲输出，不是断力矩（驱动器 EN 接地，常使能）
* 两轴相互独立，RA 忙不影响 DEC 下发

## [配置参数](https://github.com/CatHouseStudio/EquatorialMDIY/blob/main/README.md#配置参数)

主要在 `src/Configuration.h` 中修改：

* 步进引脚：`Pin_Stepper_RA_Step`、`Pin_Stepper_RA_Dir` 等
* `Stepper_*_StepHalfPeriodUs`：半步周期（µs），整步周期为两倍，默认 250，约 2000 步/秒
* `Stepper_*_Acceleration`：加速度（步/秒²），取大值近似直起直停
* `sta_ssid` / `sta_passwd`：STA 模式要连接的 WiFi
* `default_ap_ssid` / `default_ap_password`：AP 模式默认热点

## [已知限制](https://github.com/CatHouseStudio/EquatorialMDIY/blob/main/README.md#已知限制)

* 固件不记录电机位置，没有绝对定位
* 前端承担脉冲计算，丢步或手动拨动后无法校正
* 停止不是断力矩，电机仍带保持力矩
* 不支持分片请求体，POST 的 JSON 需要一次送达
* `/api/get_ap_config` 会明文返回 AP 密码
* `/get_SystemStatus`、`/get_EfuseMac`、`/get_ChipDiagnostics` 三条路径没有 `/api` 前缀
* WebSocket 端点尚未注册
