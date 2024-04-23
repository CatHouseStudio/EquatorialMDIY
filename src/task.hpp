#pragma once
#include "Configuration.h"
#include "SerialMessage.hpp"
#include "WebServer.hpp"
// tasks
void task_StepperMotor_Init(void *parameters);
void task_StepperMotor_Work(void *parameters);
void task_System_Toggle(void *parameters);
void task_Serial0Output(void *parameter);
void task_AsyncWebServer_Start(void *parameters);
// task handle
TaskHandle_t xTaskHandle_StepperMotor_Init;
TaskHandle_t xTaskHandle_StepperMotor_Work;
TaskHandle_t xTaskHandle_System_Toggle;
TaskHandle_t xTaskHandle_Serial0Output;
TaskHandle_t xTaskHandle_AsyncWebServer_Start;
// Declaring a global variable of type SemaphoreHandle_t
SemaphoreHandle_t interruptSemaphore;
void Semaphore_Init(void);
void interruptHandler();
void Semaphore_Init(void)
{
    interruptSemaphore = xSemaphoreCreateBinary();
    if (interruptSemaphore != NULL)
    {
        // Attach interrupt for Arduino digital pin
        attachInterrupt(digitalPinToInterrupt(Pin_Button), interruptHandler, CHANGE);
    }
}
void interruptHandler()
{
    /**
     * Give semaphore in the interrupt handler
     * https://www.freertos.org/a00124.html
     */
    //! maybe need mutex
    // todo use xQueueSendFromISR xSemaphoreGiveFromISR
    xSemaphoreGiveFromISR(interruptSemaphore, NULL);
}
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
        task_System_Toggle,
        "System Toggle",
        configMINIMAL_STACK_SIZE,
        NULL,
        configMAX_PRIORITIES - 1,
        &xTaskHandle_System_Toggle);
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
        configMINIMAL_STACK_SIZE+256,   // need to calculate the size of the task
        NULL,
        configMAX_PRIORITIES - 3,
        &xTaskHandle_AsyncWebServer_Start);
}
void task_AsyncWebServer_Start(void *parameters)
{
    WiFi.mode(WIFI_STA);
    //! you must change the wifi ssid and passwd
    const char *ssid = "ssid";
    const char *passwd = "password";
    WiFi.begin(ssid, passwd);
    while (WiFi.status() != WL_CONNECTED)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    xQueueSend(queueHandle_Serial0, &"server", (TickType_t)0);
    char ip[100] = "IP address: ";
    strcat(ip, WiFi.localIP().toString().c_str());
    const char *outIP = ip;
    xQueueSend(queueHandle_Serial0, &outIP, (TickType_t)0);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    //   initWebSocket();
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", "Hi! This is ElegantOTA AsyncDemo."); });

    ElegantOTA.begin(&server); // Start ElegantOTA
    // ElegantOTA callbacks
    ElegantOTA.onStart(onOTAStart);
    ElegantOTA.onProgress(onOTAProgress);
    ElegantOTA.onEnd(onOTAEnd);
    server.begin();
}

void task_StepperMotor_Init(void *parameters)
{
    for (;;)
    {
        /* code */
    }
}
void task_StepperMotor_Work(void *parameters)
{
    for (;;)
    {
        /* code */
    }
}
void task_System_Toggle(void *parameters)
{
    for (;;)
    {
        if (xSemaphoreTake(interruptSemaphore, portMAX_DELAY) == pdPASS)
        {
            if (digitalRead(Pin_Button) == Button_Trig_Status)
            { // button has been pressed

                const char *msg = "Button Pressed";
                xQueueSend(queueHandle_Serial0, (void *)&msg, (TickType_t)0);
                //! should change the system status
            }
            else
            {
                // button has been pressed

                const char *msg = "Button Released";
                xQueueSend(queueHandle_Serial0, (void *)&msg, (TickType_t)0);
                //! should change the system status
            }
        }
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