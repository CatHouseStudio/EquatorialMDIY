#pragma once
#include "Configuration.h"
#include "SerialMessage.hpp"
#include "WebServer.hpp"

// tasks
void task_Serial0Output(void *parameter);
void task_AsyncWebServer(void *parameters);
void task_TiltFusion(void *parameters);

// task handle
static TaskHandle_t xTaskHandle_Serial0Output;
static TaskHandle_t xTaskHandle_AsyncWebServer;
static TaskHandle_t xTaskHandle_GPSInfo;
static TaskHandle_t xTaskHandle_TiltFusion;

// task create
void task_Create(void);

// For some reason, I modify the configMax_PRIORITIES from 4 to 8
void task_Create(void)
{
    Initialize_Pin();
    xTaskCreate(
        task_Serial0Output,
        "Serial0 Output",
        configMINIMAL_STACK_SIZE + 8192,
        NULL,
        configMAX_PRIORITIES - 3,
        &xTaskHandle_Serial0Output);
    xTaskCreate(
        task_AsyncWebServer,
        "AsyncWebServer Start",
        configMINIMAL_STACK_SIZE + 40960, // need to calculate the size of the task
        NULL,
        configMAX_PRIORITIES - 4,
        &xTaskHandle_AsyncWebServer);
    xTaskCreate(
        task_TiltFusion,
        "Enable TiltFusion MPU6050",
        configMINIMAL_STACK_SIZE + 4096,
        NULL,
        configMAX_PRIORITIES - 3,
        &xTaskHandle_TiltFusion);
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
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}
void task_TiltFusion(void *parameters)
{
    InitTiltFusion();
    for (;;)
    {
        safeUpdateTiltFusion();
        vTaskDelay(pdMS_TO_TICKS(20)); // too low may block "thread"
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
        // vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
