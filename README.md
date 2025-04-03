# [赤道天球仪](https://github.com/CatHouseStudio/EquatorialMDIY/blob/main/README.md#赤道天球仪-----)

本项目是一个基于ESP32开发板构建的赤道天球仪，主要用于天文星体的跟踪和观测。

## 需求设备

* ESP32开发板
* 42步进电机
* DRV8825驱动板（或类似驱动板）
* MPU6050
* 电源
* 若干电线

## [使用方式](https://github.com/CatHouseStudio/EquatorialMDIY/blob/main/README.md#使用方式)

1. 安装vscode
2. 安装[PlatformIO IDE](https://platformio.org)扩展
3. git clone这个工程项目，并在vscode中打开
4. 编译并烧录固件至ESP32开发板上
5. 连接电路并安装机械结构

## [相关说明](https://github.com/CatHouseStudio/EquatorialMDIY/blob/main/README.md#相关说明)

* 本项目采用了Freertos
* [web_view](https://github.com/CatHouseStudio/EquatorialMDIY/tree/main/web_view)目录中为该项目的react前端代码
* [天文参数.rest](https://github.com/CatHouseStudio/EquatorialMDIY/tree/main/天文参数.rest)中存放的是几个simbad地外恒星的信息获取url
* [系统API说明文档](https://github.com/CatHouseStudio/EquatorialMDIY/tree/main/系统API说明文档.md)详细描述了本项目中通过 Web UI 与 ESP32 单片机之间通信的 API 接口

