#pragma once
#include "Configuration.h"

const uint8_t Max_Message_Length=100;
// functions declaration
void Serial0_Message_Queue_Init(void);
// LCD Message Queue
static QueueHandle_t queueHandle_Serial0;

void Serial0_Message_Queue_Init(void)
{
    queueHandle_Serial0 = xQueueCreate(Max_Message_Length, sizeof(char));
    Serial.begin(Serial_0_Bit_Rate);
    if (queueHandle_Serial0 == NULL)
    {
        Serial.println("Queue init error!!");
        while (1)
        {
        }
    }
    Serial.println("Queue init success!");
}