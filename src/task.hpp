#pragma once
#include "Configuration.h"
#include "SerialMessage.hpp"
#include "WiFiApSta.hpp"
#include "WebServer.hpp"
// tasks
void task_StepperMotor_Init(void *parameters);
void task_StepperMotor_Work(void *parameters);
void task_Serial0Output(void *parameter);
void task_AsyncWebServer_Start(void *parameters);
// task handle
static TaskHandle_t xTaskHandle_StepperMotor_Init;
static TaskHandle_t xTaskHandle_StepperMotor_Work;
static TaskHandle_t xTaskHandle_Serial0Output;
static TaskHandle_t xTaskHandle_AsyncWebServer_Start;

// task create
void task_Create(void);

// For some reason, I modify the configMax_PRIORITIES from 4 to 8
void task_Create(void)
{
    Initialize_Pin();
    xTaskCreate(
        task_StepperMotor_Init,
        "StepperMotor Init",
        configMINIMAL_STACK_SIZE,
        NULL,
        configMAX_PRIORITIES - 5,
        &xTaskHandle_StepperMotor_Init);
    xTaskCreate(
        task_StepperMotor_Work,
        "StepperMotor Work",
        configMINIMAL_STACK_SIZE,
        NULL,
        configMAX_PRIORITIES - 5,
        &xTaskHandle_StepperMotor_Work);
    xTaskCreate(
        task_Serial0Output,
        "Serial0 Output",
        configMINIMAL_STACK_SIZE,
        NULL,
        configMAX_PRIORITIES - 4,
        &xTaskHandle_Serial0Output);
    xTaskCreate(
        task_AsyncWebServer_Start,
        "AsyncWebServer Start",
        configMINIMAL_STACK_SIZE + 256, // need to calculate the size of the task
        NULL,
        configMAX_PRIORITIES - 3,
        &xTaskHandle_AsyncWebServer_Start);

    if (xTaskHandle_StepperMotor_Init != NULL)
    {
        // Suspend the Stepper motor
        vTaskSuspend(xTaskHandle_StepperMotor_Init);
    }
    if (xTaskHandle_StepperMotor_Work != NULL)
    {
        // Suspend the Stepper motor
        vTaskSuspend(xTaskHandle_StepperMotor_Work);
    }
}
void task_AsyncWebServer_Start(void *parameters)
{
    WiFi_AP_STA_Init();
    WebServerEvent();
    ElegantOTA.begin(&server); // Start ElegantOTA
    // ElegantOTA callbacks
    ElegantOTA.onStart(onOTAStart);
    ElegantOTA.onProgress(onOTAProgress);
    ElegantOTA.onEnd(onOTAEnd);
    server.begin();
    for (;;)
    {
        ElegantOTA.loop();
    }
}

void task_StepperMotor_Init(void *parameters)
{
    for (;;)
    {
        digitalWrite(Pin_Stepper_Motor_Dir, Stepper_Motor_Initialize_Dir);
        ledcSetup(Stepper_Motor_Channel, Stepper_Motor_Freq, Stepper_Motor_resolution); 
        ledcAttachPin(Pin_Stepper_Motor_Step, Stepper_Motor_Channel);                    
    }
}
void task_StepperMotor_Work(void *parameters)
{
    for (;;)
    {
        digitalWrite(Pin_Stepper_Motor_Dir, Stepper_Motor_Work_Dir);
        ledcSetup(Stepper_Motor_Channel, Stepper_Motor_Freq, Stepper_Motor_resolution); 
        ledcAttachPin(Pin_Stepper_Motor_Step, Stepper_Motor_Channel);  
    }
}
void task_Serial0Output(void *parameter)
{
    Serial0_Message_Queue_Init();
    for (;;)
    {
        char *readBuffer;
        if (queueHandle_Serial0 != NULL)
        {
            if (xQueueReceive(queueHandle_Serial0, &readBuffer, portMAX_DELAY) == pdPASS)
            {
                Serial.println(readBuffer);
            }
        }
        // vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}