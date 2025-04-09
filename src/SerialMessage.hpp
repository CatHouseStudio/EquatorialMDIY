#pragma once
#include "Configuration.h"

// functions declaration
void Serial0_Message_Init(uint32_t baud = Serial_0_Bit_Rate);
void Serial0_Print(const char *msg);
void Serial0_Println(const char *msg);
void Serial0_Printf(const char *fmt, ...);
void task_Serial0Output(void *parameter);

// Serial Message Queue
static QueueHandle_t queueHandle_Serial0;
// Serial Message Handle
static TaskHandle_t xTaskHandle_Serial0Output = NULL;

void Serial0_Message_Init(uint32_t baud)
{
    Serial.begin(baud);
    if (!queueHandle_Serial0)
    {
        queueHandle_Serial0 = xQueueCreate(Serial0_Max_Queue_Length, Serial0_Max_Message_Length);
        if (queueHandle_Serial0)
        {
            xTaskCreate(
                task_Serial0Output,
                "Serial0 Output",
                configMINIMAL_STACK_SIZE + 4096,
                NULL,
                configMAX_PRIORITIES - 3,
                &xTaskHandle_Serial0Output);
            Serial.println("Serial0_Message_Init succeed!");
        }
        else
        {
            Serial.println("Serial0_Message_Init failed!");
        }
    }
}
void Serial0_Print(const char *msg)
{
    if (!queueHandle_Serial0 || !msg)
    {
        return;
    }
    char line[Serial0_Max_Message_Length];
    strncpy(line, msg, Serial0_Max_Message_Length - 1);
    line[Serial0_Max_Message_Length - 1] = '\0';
    xQueueSend(queueHandle_Serial0, &line, (TickType_t)0);
}
void Serial0_Println(const char *msg)
{
    if (!queueHandle_Serial0 || !msg)
    {
        return;
    }
    char line[Serial0_Max_Message_Length];
    size_t len = strlen(msg);
    if (len > Serial0_Max_Message_Length - 3)
    {
        len = Serial0_Max_Message_Length - 3;
    }

    strncpy(line, msg, len);
    line[len] = '\r';
    line[len + 1] = '\n';
    line[len + 2] = '\0';

    xQueueSend(queueHandle_Serial0, &line, (TickType_t)0);
}
void Serial0_Print(const String &msg)
{
    if (!queueHandle_Serial0)
    {
        return;
    }

    size_t len = msg.length();
    if (len > Serial0_Max_Message_Length - 1)
    {
        len = Serial0_Max_Message_Length - 1;
    }

    char line[Serial0_Max_Message_Length];
    strncpy(line, msg.c_str(), len);
    line[len] = '\0';

    xQueueSend(queueHandle_Serial0, &line, (TickType_t)0);
}
void Serial0_Println(const String &msg)
{
    if (!queueHandle_Serial0)
    {
        return;
    }

    char line[Serial0_Max_Message_Length];
    size_t len = msg.length();
    if (len > Serial0_Max_Message_Length - 3)
    {
        len = Serial0_Max_Message_Length - 3;
    }

    strncpy(line, msg.c_str(), len);
    line[len] = '\r';
    line[len + 1] = '\n';
    line[len + 2] = '\0';

    xQueueSend(queueHandle_Serial0, &line, (TickType_t)0);
}
void Serial0_Printf(const char *fmt, ...)
{
    if (!queueHandle_Serial0 || !fmt)
    {
        return;
    }
    char line[Serial0_Max_Message_Length];
    va_list args;
    va_start(args, fmt);
    vsnprintf(line, Serial0_Max_Message_Length, fmt, args);
    va_end(args);
    xQueueSend(queueHandle_Serial0, &line, (TickType_t)0);
}

void task_Serial0Output(void *parameter)
{
    char line[Serial0_Max_Message_Length];
    while (1)
    {
        if (xQueueReceive(queueHandle_Serial0, &line, portMAX_DELAY) == pdTRUE)
        {
            Serial.println(line);
        }
    }
}