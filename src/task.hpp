#pragma once
#include "Configuration.h"
#include "SerialMessage.hpp"
#include "WebServer.hpp"

// tasks

void task_AsyncWebServer(void *parameters);

// task handle
static TaskHandle_t xTaskHandle_AsyncWebServer = NULL;
static TaskHandle_t xTaskHandle_GPSInfo = NULL;
static TaskHandle_t xTaskHandle_TiltFusion = NULL;

// task create
void task_Create(void);

// For some reason, I modify the configMax_PRIORITIES from 4 to 8
void task_Create(void)
{
    Initialize_Pin();
    Serial0_Message_Init();

    xTaskCreate(
        task_AsyncWebServer,
        "AsyncWebServer Start",
        configMINIMAL_STACK_SIZE + 8192, // need to calculate the size of the task
        NULL,
        configMAX_PRIORITIES - 4,
        &xTaskHandle_AsyncWebServer);
    xTaskCreate(
        task_Stepper_RA,
        "Task Stepper RA",
        configMINIMAL_STACK_SIZE + 4096,
        NULL,
        configMAX_PRIORITIES - 3,
        NULL);
    xTaskCreate(
        task_Stepper_DEC,
        "Task Stepper DEC",
        configMINIMAL_STACK_SIZE + 4096,
        NULL,
        configMAX_PRIORITIES - 3,
        NULL);
    xTaskCreate(
        task_I2CWorker,
        "Task I2CWorker",
        configMINIMAL_STACK_SIZE + 4096,
        NULL,
        configMAX_PRIORITIES - 3,
        NULL);
    // xTaskCreate(
    //     task_TiltFusion,
    //     "Enable TiltFusion MPU6050",
    //     configMINIMAL_STACK_SIZE + 4096,
    //     NULL,
    //     configMAX_PRIORITIES - 3,
    //     &xTaskHandle_TiltFusion);
    // xTaskCreate(
    //     task_GPSInfo,
    //     "GPSInfo",
    //     configMINIMAL_STACK_SIZE + 8192,
    //     NULL,
    //     configMAX_PRIORITIES - 3,
    //     &xTaskHandle_GPSInfo);
}
void task_AsyncWebServer(void *parameters)
{
    WiFi_AP_Init();
    WiFi_STA_Init();
    Serial0_Println("Setting WebServerEvent");
    WebServerEvent();
    Serial0_Println("Setting ElegantOTA...");
    ElegantOTA.begin(&server); // Start ElegantOTA
    // ElegantOTA callbacks
    ElegantOTA.onStart(onOTAStart);
    ElegantOTA.onProgress(onOTAProgress);
    ElegantOTA.onEnd(onOTAEnd);
    Serial0_Println("Set ElegantOTA Finished!");
    Serial0_Println("Start Server...");
    server.begin();
    Serial0_Println("Server Started!");
    for (;;)
    {
        ElegantOTA.loop();
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

