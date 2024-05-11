// This file is used to set the parameters (eg: PIN,delay_time etc...).
//===================================================================================
//! Warning: On Arduino Mega2560, some PINs are used for Serial communication.
// Serial<DEFAULT> PIN: 1(RX),2(TX)
// Serial<ONE> PIN: 19(RX),18(TX)
// Serial<TWO> PIN: 17(RX),16(TX)
// Serial<THREE> PIN: 15(RX),14(TX)
//===================================================================================
// On Mks Gen Mega2560, 42 Stepper Motor can use three kinds of motor drivers.
// 1) DRV8825
//===================================================================================
//| 1  | 2  | 3  |                step mode
//-----------------------------------------------------------------------------------
//| 0  | 0  | 0	 | Full step (2-phase excitation) with 71% current  |
//-----------------------------------------------------------------------------------
//| 1  | 0  | 0  |      1/2 step (1-2 phase excitation)             |
//-----------------------------------------------------------------------------------
//| 0  | 1  | 0  |      1/4 step (W1-2 phase excitation)            |
//-----------------------------------------------------------------------------------
//| 1  | 1  | 0  |         8 microsteps/step                        |
//-----------------------------------------------------------------------------------
//| 0  | 0  | 1  |         16 microsteps/step                       |
//-----------------------------------------------------------------------------------
//| 1  | 0  | 1  |         32 microsteps/step                       |
//-----------------------------------------------------------------------------------
//| 0  | 1  | 1  |         32 microsteps/step                       |
//-----------------------------------------------------------------------------------
//| 1  | 1  | 1  |         32 microsteps/step                       |
//===================================================================================
// 2) A4988
//===================================================================================
//| 1  | 2  | 3  |                step mode                         |
//-----------------------------------------------------------------------------------
//| 0  | 0  | 0  |                Full step                         |
//-----------------------------------------------------------------------------------
//| 1  | 0  | 0  |                1/2 step                          |
//-----------------------------------------------------------------------------------
//| 0  | 1  | 0  |                1/4 step                          |
//-----------------------------------------------------------------------------------
//| 1  | 1  | 0  |              8 microsteps/step                   |
//-----------------------------------------------------------------------------------
//| 1  | 1  | 1  |              16 microsteps/step                  |
//===================================================================================
// 3) TMC 2208
//| 1  | 2  | 3  |                step mode                         |
//-----------------------------------------------------------------------------------
//| 1  | 0  | 0  |                  2 step                          |
//-----------------------------------------------------------------------------------
//| 0  | 1  | 0  |                  4 step                          |
//-----------------------------------------------------------------------------------
//| 0  | 0  | 0  |                  8 step                          |
//-----------------------------------------------------------------------------------
//| 1  | 1  | 0  |                  16 step                         |
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
// X Group
// TODO you must change the pin value
const uint8_t Pin_Stepper_Motor_En = 32;
const uint8_t Pin_Stepper_Motor_Dir = 33;
const uint8_t Pin_Stepper_Motor_Step = 25;

// =============================================
// Set the Serials' Bit rate
const unsigned long Serial_0_Bit_Rate = 115200;
// This Serial2 for GPS Module
const uint8_t Pin_Serial_GPS_RX=16;
const uint8_t Pin_Serial_GPS_TX=17;
const unsigned long Serial_GPS_Bit_Rate = 4800;

// Set the Stepper Motors' Parameter.
// X Group
const uint8_t Stepper_Motor_Initialize_Dir = HIGH;
const uint8_t Stepper_Motor_Work_Dir = LOW;
const uint8_t Stepper_Motor_Freq = 74; // 74Hz
const uint8_t Stepper_Motor_Channel = 0;
const uint8_t Stepper_Motor_resolution = 8;
const uint8_t Stepper_Motor_dutyCycle = 128;

// * Stepper Motor Status
const uint8_t Stepper_Status_Clockwise = 0;
const uint8_t Stepper_Status_CounterClockwise = 1;
const uint8_t Stepper_Status_Stop = 2;

inline void Initialize_Pin() // This function is used for Initializing
{
    // X Group
    pinMode(Pin_Stepper_Motor_En, OUTPUT);
    digitalWrite(Pin_Stepper_Motor_En, LOW);
    pinMode(Pin_Stepper_Motor_Dir, OUTPUT);
    pinMode(Pin_Stepper_Motor_Step, OUTPUT);
    // Attach the PWM OUTPUT
    ledcAttachPin(Pin_Stepper_Motor_Step, Stepper_Motor_Channel);
}
