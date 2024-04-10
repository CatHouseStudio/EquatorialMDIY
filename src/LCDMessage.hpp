#pragma once
#include "Configuration.h"
#include "Wire.h"
#include "LiquidCrystal_I2C.h"
#include "Arduino_FreeRTOS.h"
#include "queue.h"

// functions declaration
void LCDMessage_Queue_Init(void);
// LCD Message Queue
static QueueHandle_t queueHandle_LCD;

// LCD Message Struct and Pointer
struct LCDMessage
{
    uint8_t col;
    uint8_t row;
    char msg[256];
} typedef xLCDMessage;
static xLCDMessage xLCDMsg;
static xLCDMessage *pxLCDMsg;
void LCDMessage_Queue_Init(void)
{
    pxLCDMsg = &xLCDMsg;
    queueHandle_LCD = xQueueCreate(10, sizeof(struct LCDMessage *));
    if (queueHandle_LCD == NULL)
    {
    }
}
