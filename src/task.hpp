#pragma once
#include "Configuration.h"
#include "SerialMessage.hpp"
#include "WebServer.hpp"


// tasks
void task_Serial0Output(void *parameter);
void task_AsyncWebServer(void *parameters);

// task handle
static TaskHandle_t xTaskHandle_Serial0Output;
static TaskHandle_t xTaskHandle_AsyncWebServer;
static TaskHandle_t xTaskHandle_GPSInfo;



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
        task_GPSInfo,
        "GPSInfo",
        configMINIMAL_STACK_SIZE + 8192,
        NULL,
        configMAX_PRIORITIES - 3,
        &xTaskHandle_GPSInfo);
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
        vTaskDelay(1000 / portTICK_PERIOD_MS);
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

