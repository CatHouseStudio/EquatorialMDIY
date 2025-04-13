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

// RA Stepper
const uint8_t Stepper_RA_Initialize_Dir = HIGH;
const uint8_t Stepper_RA_Work_Dir = LOW;
const uint32_t Stepper_RA_DelayMs = 250;
const float Stepper_RA_K = 114514; // Mechanical structure reduction ratio coefficient
// DEC Stepper
const uint8_t Stepper_DEC_Initialize_Dir = HIGH;
const uint8_t Stepper_DEC_Work_Dir = LOW;
const uint32_t Stepper_DEC_DelayMs = 250;
const float Stepper_DEC_K = 114514; // Mechanical structure reduction ratio coefficient

// Serial 0 Config
const uint8_t Serial0_Max_Message_Length = 100;
const uint8_t Serial0_Max_Queue_Length = 20;

inline void Initialize_Pin() // This function is used for Initializing
{
    // Stepper
    pinMode(Pin_Stepper_RA_Dir, OUTPUT);
    pinMode(Pin_Stepper_RA_Step, OUTPUT);
    pinMode(Pin_Stepper_DEC_Dir, OUTPUT);
    pinMode(Pin_Stepper_DEC_Step, OUTPUT);
}

// Beta vTaskDelayMicroseconds, for better status, us>=100
inline void DelayUs(uint32_t us)
{
    TaskHandle_t currentTask = xTaskGetCurrentTaskHandle();

    esp_timer_handle_t timer;
    esp_timer_create_args_t timerArgs = {
        .callback = [](void *arg)
        {
            TaskHandle_t handle = (TaskHandle_t)arg;
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            xTaskNotifyGive(handle); // ✅ 非 ISR 中调用 notify，安全
        },
        .arg = (void *)currentTask,
        .dispatch_method = ESP_TIMER_TASK, // ✅ 推荐使用方式，兼容所有 Arduino ESP32 版本
        .name = "delay_us_task"};
    esp_timer_create(&timerArgs, &timer);
    esp_timer_start_once(timer, us);

    // 当前任务进入等待
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    esp_timer_delete(timer);
}
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