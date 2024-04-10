#pragma once
#include "Configuration.h"
#include "Arduino_FreeRTOS.h"
#include "queue.h"
#include <semphr.h>
#include "LCDMessage.hpp"
#include "SerialMessage.hpp"
// tasks
void task_StepperMotor_Init(void *parameters);
void task_StepperMotor_Work(void *parameters);
void task_System_Toggle(void *parameters);
void task_LiquidCrystal_I2C(void *parameters);
void task_Serial0Output(void *parameter);
// task handle
TaskHandle_t xTaskHandle_StepperMotor_Init;
TaskHandle_t xTaskHandle_StepperMotor_Work;
TaskHandle_t xTaskHandle_System_Toggle;
TaskHandle_t xTaskHandle_LiquidCrystal_I2C;
TaskHandle_t xTaskHandle_Serial0Output;
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
        configMAX_PRIORITIES - 3,
        &xTaskHandle_StepperMotor_Init);
    xTaskCreate(
        task_StepperMotor_Work,
        "StepperMotor Work",
        configMINIMAL_STACK_SIZE,
        NULL,
        configMAX_PRIORITIES - 3,
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
        configMAX_PRIORITIES - 2,
        &xTaskHandle_Serial0Output);
    xTaskCreate(
        task_LiquidCrystal_I2C,
        "LiquidCrystal I2C",
        configMINIMAL_STACK_SIZE,
        NULL,
        configMAX_PRIORITIES - 2,
        &xTaskHandle_LiquidCrystal_I2C);
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
                xLCDMsg.col = 0;
                xLCDMsg.row = 0;
                strcpy(xLCDMsg.msg, "Button Pressed");
                xQueueSend(queueHandle_LCD, (void *)&pxLCDMsg, (TickType_t)0);
                const char *msg = "Button Pressed";
                xQueueSend(queueHandle_Serial0, (void *)&msg, (TickType_t)0);
                //! should change the system status
                digitalWrite(LED_BUILTIN, LOW);
            }
            else
            {
                // button has been pressed
                xLCDMsg.col = 0;
                xLCDMsg.row = 0;
                strcpy(xLCDMsg.msg, "Button Released");
                xQueueSend(queueHandle_LCD, (void *)&pxLCDMsg, (TickType_t)0);
                const char *msg = "Button Released";
                xQueueSend(queueHandle_Serial0, (void *)&msg, (TickType_t)0);
                //! should change the system status
                digitalWrite(LED_BUILTIN, HIGH);
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
void task_LiquidCrystal_I2C(void *parameters)
{
    LiquidCrystal_I2C lcd(0x27, 16, 2);
    lcd.init();
    lcd.backlight();
    LCDMessage_Queue_Init();
    for (;;)
    {
        if (queueHandle_LCD != NULL)
        {
            if (xQueueReceive(queueHandle_LCD, &pxLCDMsg, portMAX_DELAY))
            {
                lcd.clear();
                lcd.setCursor(pxLCDMsg->col, pxLCDMsg->row);
                lcd.print(pxLCDMsg->msg);
            }
        }
    }
}