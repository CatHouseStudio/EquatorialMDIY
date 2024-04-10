#pragma once
#include "Configuration.h"
#include "Arduino_FreeRTOS.h"
#include "queue.h"

// functions declaration
void Serial0_Message_Queue_Init(void);
// LCD Message Queue
static QueueHandle_t queueHandle_Serial0;

void Serial0_Message_Queue_Init(void)
{
    queueHandle_Serial0 = xQueueCreate(100, sizeof(char *));
    Serial.begin(115200);
    if (queueHandle_Serial0 == NULL)
    {
        Serial.println("Queue init error!!");
        while (1)
        {
        }
    }
    Serial.println("Queue init success!");
}