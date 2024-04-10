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
// ================================================

// Set the Button for start and stop
const uint8_t Pin_Button = 3;
const uint8_t Button_Trig_Status = LOW;
// Set the Limited Switch Pin.
const uint8_t Pin_Limited_Switch_1 = 15;
const uint8_t Limited_Switch_1_Trig_Status = LOW;
const uint8_t Pin_Limited_Switch_2 = 18;
const uint8_t Limited_Switch_2_Trig_Status = LOW;
const uint8_t Pin_Limited_Switch_3 = 19;
const uint8_t Limited_Switch_3_Trig_Status = LOW;
// Set the Stepper Motors' PIN.
// X Group
const uint8_t Pin_Stepper_Motor_X_En = 38;
const uint8_t Pin_Stepper_Motor_X_Dir = 55;
const uint8_t Pin_Stepper_Motor_X_Step = 54;
// Y Group
const uint8_t Pin_Stepper_Motor_Y_En = 56;
const uint8_t Pin_Stepper_Motor_Y_Dir = 61;
const uint8_t Pin_Stepper_Motor_Y_Step = 60;
// Z Group
const uint8_t Pin_Stepper_Motor_Z_En = 62;
const uint8_t Pin_Stepper_Motor_Z_Dir = 48;
const uint8_t Pin_Stepper_Motor_Z_Step = 46;
// E0 Group
const uint8_t Pin_Stepper_Motor_E0_En = 24;
const uint8_t Pin_Stepper_Motor_E0_Dir = 28;
const uint8_t Pin_Stepper_Motor_E0_Step = 26;
// E1 Group
const uint8_t Pin_Stepper_Motor_E1_En = 30;
const uint8_t Pin_Stepper_Motor_E1_Dir = 34;
const uint8_t Pin_Stepper_Motor_E1_Step = 36;

// =============================================
// Set the Serials' Bit rate
const unsigned long Serial_0_Bit_Rate = 115200;
const unsigned long Serial_1_Bit_Rate = 115200;
const unsigned long Serial_2_Bit_Rate = 115200;
const unsigned long Serial_3_Bit_Rate = 115200;
// Set the Stepper Motors' Parameter.
// Recommend 800; At least "HIGH"+"LOW">=400 AND ("HIGH">=20 OR "LOW">=20)
// X Group
const uint8_t Stepper_Motor_X_Initialize_Dir = HIGH;
const uint8_t Stepper_Motor_X_Work_Dir = LOW;
const uint32_t Stepper_Motor_X_Loop_Times = 0;
const uint32_t Stepper_Motor_X_HIGH_Dealyms = 800;
const uint32_t Stepper_Motor_X_LOW_Dealyms = 800;
// Y Group
const uint8_t Stepper_Motor_Y_Initialize_Dir = HIGH;
const uint8_t Stepper_Motor_Y_Work_Dir = LOW;
const uint32_t Stepper_Motor_Y_Loop_Times = 0;
const uint32_t Stepper_Motor_Y_HIGH_Dealyms = 800;
const uint32_t Stepper_Motor_Y_LOW_Dealyms = 800;
// Z Group
const uint8_t Stepper_Motor_Z_Initialize_Dir = HIGH;
const uint8_t Stepper_Motor_Z_Work_Dir = LOW;
const uint32_t Stepper_Motor_Z_Loop_Times = 0;
const uint32_t Stepper_Motor_Z_HIGH_Dealyms = 800;
const uint32_t Stepper_Motor_Z_LOW_Dealyms = 800;
// E0 Group
const uint8_t Stepper_Motor_E0_Initialize_Dir = HIGH;
const uint8_t Stepper_Motor_E0_Work_Dir = LOW;
const uint32_t Stepper_Motor_E0_Loop_Times = 0;
const uint32_t Stepper_Motor_E0_HIGH_Dealyms = 800;
const uint32_t Stepper_Motor_E0_LOW_Dealyms = 800;
// E1 Group
const uint8_t Stepper_Motor_E1_Initialize_Dir = HIGH;
const uint8_t Stepper_Motor_E1_Work_Dir = LOW;
const uint32_t Stepper_Motor_E1_Loop_Times = 0;
const uint32_t Stepper_Motor_E1_HIGH_Dealyms = 800;
const uint32_t Stepper_Motor_E1_LOW_Dealyms = 800;

inline void Initialize_Pin() // This function is used for Initializing
{

    // Set PinMode
    // Led built-in
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    // Button
    pinMode(Pin_Button, INPUT_PULLUP);
    // Limited Switch
    pinMode(Pin_Limited_Switch_1, INPUT_PULLUP);
    pinMode(Pin_Limited_Switch_2, INPUT_PULLUP);
    pinMode(Pin_Limited_Switch_3, INPUT_PULLUP);
    // X Group
    pinMode(Pin_Stepper_Motor_X_En, OUTPUT);
    pinMode(Pin_Stepper_Motor_X_Dir, OUTPUT);
    pinMode(Pin_Stepper_Motor_X_Step, OUTPUT);
    // Y Group
    pinMode(Pin_Stepper_Motor_Y_En, OUTPUT);
    pinMode(Pin_Stepper_Motor_Y_Dir, OUTPUT);
    pinMode(Pin_Stepper_Motor_Y_Step, OUTPUT);
    // Z Group
    pinMode(Pin_Stepper_Motor_Z_En, OUTPUT);
    pinMode(Pin_Stepper_Motor_Z_Dir, OUTPUT);
    pinMode(Pin_Stepper_Motor_Z_Step, OUTPUT);
    // E0 Group
    pinMode(Pin_Stepper_Motor_E0_En, OUTPUT);
    pinMode(Pin_Stepper_Motor_E0_Dir, OUTPUT);
    pinMode(Pin_Stepper_Motor_E0_Step, OUTPUT);
    // E1 Group
    pinMode(Pin_Stepper_Motor_E1_En, OUTPUT);
    pinMode(Pin_Stepper_Motor_E1_Dir, OUTPUT);
    pinMode(Pin_Stepper_Motor_E1_Step, OUTPUT);
    // Limited Switch Pin.
    pinMode(Pin_Limited_Switch_1, INPUT_PULLUP);
    pinMode(Pin_Limited_Switch_2, INPUT_PULLUP);
    pinMode(Pin_Limited_Switch_3, INPUT_PULLUP);
}
