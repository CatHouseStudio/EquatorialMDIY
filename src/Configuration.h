// This file is used to set the parameters (eg: PIN,delay_time etc...).
//===================================================================================
//! Warning: On Arduino Mega2560, some PINs are used for Serial communication.
// Serial<DEFAULT> PIN: 1(RX),2(TX)
// Serial<ONE> PIN: 19(RX),18(TX)
// Serial<TWO> PIN: 17(RX),16(TX)
// Serial<THREE> PIN: 15(RX),14(TX)
//===================================================================================
// 42 Stepper Motor(1.8°motor) can use these kinds of motor drivers.
// 1) DRV8825
//===================================================================================
//| 1  | 2  | 3  |                Microstep                         |    Steps/rev
//-----------------------------------------------------------------------------------
//| 0  | 0  | 0	 | Full step (2-phase excitation) with 71% current  |       200
//-----------------------------------------------------------------------------------
//| 1  | 0  | 0  |          2 microsteps (1-2 phase excitation)     |       400
//-----------------------------------------------------------------------------------
//| 0  | 1  | 0  |          4 microsteps (W1-2 phase excitation)    |       800
//-----------------------------------------------------------------------------------
//| 1  | 1  | 0  |              8 microsteps                        |       1600
//-----------------------------------------------------------------------------------
//| 0  | 0  | 1  |              16 microsteps                       |       3200
//-----------------------------------------------------------------------------------
//| 1  | 0  | 1  |              32 microsteps                       |       6400
//-----------------------------------------------------------------------------------
//| 0  | 1  | 1  |              32 microsteps                       |       6400
//-----------------------------------------------------------------------------------
//| 1  | 1  | 1  |              32 microsteps                       |       6400
//===================================================================================
// 2) A4988
//===================================================================================
//| 1  | 2  | 3  |                Microstep                         |    Steps/rev
//-----------------------------------------------------------------------------------
//| 0  | 0  | 0  |                Full step                         |       200
//-----------------------------------------------------------------------------------
//| 1  | 0  | 0  |              2 microsteps                        |       400
//-----------------------------------------------------------------------------------
//| 0  | 1  | 0  |              4 microsteps                        |       800
//-----------------------------------------------------------------------------------
//| 1  | 1  | 0  |              8 microsteps                        |       1600
//-----------------------------------------------------------------------------------
//| 1  | 1  | 1  |              16 microsteps                       |       3200
//===================================================================================
// 3) TMC 2208
//| 1  | 2  | 3  |                Microstep                         |    Steps/rev
//-----------------------------------------------------------------------------------
//| 1  | 0  | 0  |              2 microsteps                        |       200
//-----------------------------------------------------------------------------------
//| 0  | 1  | 0  |              4 microsteps                        |       400
//-----------------------------------------------------------------------------------
//| 0  | 0  | 0  |              8 microsteps                        |       800
//-----------------------------------------------------------------------------------
//| 1  | 1  | 0  |              16 microsteps                       |       1600
//===================================================================================
// 4) DM542C SW1,SW2,SW3 for Dynamic Current SW4,SW5,SW6,SW7 for Microstep Resolution
//| 4  | 5  | 6  | 7  |                Microstep                    |    Steps/rev
//-----------------------------------------------------------------------------------
//| 0  | 1  | 1  | 1  |                  2 microsteps               |       400
//-----------------------------------------------------------------------------------
//| 1  | 0  | 1  | 1  |                  4 microsteps               |       800
//-----------------------------------------------------------------------------------
//| 0  | 0  | 1  | 1  |                  8 microsteps               |       1600
//-----------------------------------------------------------------------------------
//| 1  | 1  | 0  | 1  |                  16 microsteps              |       3200
//-----------------------------------------------------------------------------------
//| 0  | 1  | 0  | 1  |                  32 microsteps              |       6400
//-----------------------------------------------------------------------------------
//| 1  | 0  | 0  | 1  |                  64 microsteps              |       12800
//-----------------------------------------------------------------------------------
//| 0  | 0  | 0  | 1  |                  128 microsteps             |       25600
//-----------------------------------------------------------------------------------
//| 1  | 1  | 1  | 0  |                  5 microsteps               |       1000
//-----------------------------------------------------------------------------------
//| 0  | 1  | 1  | 0  |                  10 microsteps              |       2000
//-----------------------------------------------------------------------------------
//| 1  | 0  | 1  | 0  |                  20 microsteps              |       4000
//-----------------------------------------------------------------------------------
//| 0  | 0  | 1  | 0  |                  25 microsteps              |       5000
//-----------------------------------------------------------------------------------
//| 1  | 1  | 0  | 0  |                  40 microsteps              |       8000
//-----------------------------------------------------------------------------------
//| 0  | 1  | 0  | 0  |                  50 microsteps              |       10000
//-----------------------------------------------------------------------------------
//| 1  | 0  | 0  | 0  |                  100 microsteps             |       20000
//-----------------------------------------------------------------------------------
//| 0  | 0  | 0  | 0  |                  125 microsteps             |       25000
//===================================================================================

// More information can be found on https://reprap.org/wiki/MKS_GEN#Motor_Driver
//===================================================================================
// Thanks to the MKS.
// Writen down on 2024/4/9 by LYF
#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include "SPIFFS.h"
// ================================================

// Set the Stepper Motors' PIN.

// Celestial Stepper
// TODO you must change the pin value
// In this case, you should connect the pin of enable to gnd.
// RA Stepper
const uint8_t Pin_Stepper_RA_Dir = 33;
const uint8_t Pin_Stepper_RA_Step = 25;

// DEC Stepper
const uint8_t Pin_Stepper_DEC_Dir = 26;
const uint8_t Pin_Stepper_DEC_Step = 27;

// =============================================
// Set the Serials' Bit rate
const unsigned long Serial_0_Bit_Rate = 115200;
// This Serial2 for GPS Module
const uint8_t Pin_Serial_GPS_RX = 16;
const uint8_t Pin_Serial_GPS_TX = 17;
const unsigned long Serial_GPS_Bit_Rate = 4800;

// =============================================
// Celestial Stepper (RA / DEC) parameters
//
// 旧实现用软件翻转 STEP 引脚，一个脉冲是 "STEP 拉高 P µs + STEP 拉低 P µs"，
// 所以 P 是半步周期，整步周期为 2*P（P=250 时约 2000 步/秒）。
// 现在脉冲由 FastAccelStepper 的硬件定时器产生，下面的数值保持原来的含义，
// 步进速率与原来一致。P 改名为 StepHalfPeriodUs：它一直是微秒，从来不是毫秒。
//
// The old code bit-banged STEP: one pulse was "STEP high for P us, STEP low for P us",
// so P is the half period and the full step period is 2*P (P=250 -> about 2000 steps/s).
// FastAccelStepper now generates the pulse train; these values keep the original meaning
// so the step rate is unchanged. P was renamed ...Us because it was always microseconds.

// RA Stepper
const uint8_t Stepper_RA_Initialize_Dir = HIGH;
const uint8_t Stepper_RA_Work_Dir = LOW;
const uint32_t Stepper_RA_StepHalfPeriodUs = 250; // 半步周期 µs（原 Stepper_RA_DelayMs）
const uint32_t Stepper_RA_Acceleration = 1000000;  // 步/秒²，取大值≈原来的恒速起停
const int Stepper_RA_K = 114514;                   // Mechanical structure reduction ratio coefficient
// DEC Stepper
const uint8_t Stepper_DEC_Initialize_Dir = HIGH;
const uint8_t Stepper_DEC_Work_Dir = LOW;
const uint32_t Stepper_DEC_StepHalfPeriodUs = 250; // 半步周期 µs（原 Stepper_DEC_DelayMs）
const uint32_t Stepper_DEC_Acceleration = 1000000;  // 步/秒²，取大值≈原来的恒速起停
const int Stepper_DEC_K = 114514;                   // Mechanical structure reduction ratio coefficient

// Serial 0 Config
const uint8_t Serial0_Max_Message_Length = 100;
const uint8_t Serial0_Max_Queue_Length = 20;

// SPIFFS Path
const char *fs_path_config = "/Config.json";

// Wifi STA and AP
//! you must change the wifi ssid and passwd
const char *sta_ssid = "";
const char *sta_passwd = "";
const char *default_ap_ssid = "ESP32-Access-Point";
const char *default_ap_password = "123456789";


inline void Initialize_Pin() // This function is used for Initializing
{
    // Stepper
    pinMode(Pin_Stepper_RA_Dir, OUTPUT);
    pinMode(Pin_Stepper_RA_Step, OUTPUT);
    pinMode(Pin_Stepper_DEC_Dir, OUTPUT);
    pinMode(Pin_Stepper_DEC_Step, OUTPUT);
}

// =============================================
// ! DelayUs() has been removed.
//
// 它原本用来在软件里给 STEP 脉冲计时，有三处硬伤：
//   1) 每次调用都 esp_timer_create/delete 一个定时器，2 kHz 步进就是每秒 4000 次
//      堆分配/释放，长期运行必然碎片化，且创建失败时 timer 未初始化就被 start/delete；
//   2) 定时器回调走 esp_timer 任务，唤醒 + 通知 + 调度一个来回通常 30~100 µs 且抖动，
//      比它自己要延时的 250 µs 还接近，实际脉冲周期既不准也不稳；
//   3) 它和 Stepper_RA_Stop()/Stepper_DEC_Stop() 共用同一个任务通知槽位：
//      停止请求可能被 ulTaskNotifyTake 吞掉（停止丢失），定时器完成的通知也可能
//      被 xTaskNotifyWait 读走当成一次假的 ACTION_STOP。
//
// 现在 STEP 脉冲由 FastAccelStepper 的硬件定时器产生，停止改用任务通知，
// 两者不再抢同一个通知槽位，也不再需要这个函数。
//
// It is gone because it created/deleted an esp_timer on every call (heap churn at
// 2 kHz), its latency/jitter was on the same order as the delay it produced, and it
// shared the task notification slot with the ACTION_STOP mechanism, so a stop could
// be swallowed. FastAccelStepper now owns the pulse timing.
// =====================
// Utility Conversions
// =====================

// Convert h:m:s → hours
inline double hms_to_hours(int h, int m, double s)
{
    return h + m / 60.0 + s / 3600.0;
}

// Convert d:m:s → decimal degrees
inline double dms_to_degrees(int d, int m, double s)
{
    double sign = (d < 0 || m < 0 || s < 0) ? -1.0 : 1.0;
    return sign * (fabs(d) + fabs(m) / 60.0 + fabs(s) / 3600.0);
}

// Convert decimal degrees → degrees, minutes, seconds
inline void degrees_to_dms(double deg, int &d, int &m, double &s)
{
    int sign = (deg >= 0) ? 1 : -1;
    deg = fabs(deg);
    d = static_cast<int>(deg);
    m = static_cast<int>((deg - d) * 60.0);
    s = ((deg - d) * 60.0 - m) * 60.0;
    d *= sign;
}

// Convert degrees → h:m:s (for Azimuth display)
inline void degrees_to_hms(double deg, int &h, int &m, double &s)
{
    deg = fmod(deg + 360.0, 360.0);
    double hours = deg / 15.0;
    h = static_cast<int>(hours);
    m = static_cast<int>((hours - h) * 60.0);
    s = ((hours - h) * 60.0 - m) * 60.0;
}