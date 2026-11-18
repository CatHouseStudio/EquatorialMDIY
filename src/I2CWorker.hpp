#pragma once

#include "TiltFusionMPU6050.hpp"
#include "INASensor.hpp"
#include "OLEDScreen.hpp"

enum I2CCommandType
{
    // I2C_CMD_GET_MPU,     // USE PEEK MODE
    // I2C_CMD_GET_INA,     // USE PEEK MODE
    I2C_CMD_SET_OLED_CONFIG
};

struct I2CCommand
{
    I2CCommandType type;
    OLEDConfig oledConfig;
};

QueueHandle_t xQueueHandle_CMD_I2CWorker = NULL;
QueueHandle_t xQueueHandle_MPU_I2CWorker = NULL;
QueueHandle_t xQueueHandle_INA_I2CWorker = NULL;
TaskHandle_t xTaskHandle_I2CWorker = NULL;

bool GetLatestMPU(MPUResult &out);
// bool GetLatestINA(INAResult &out);

void task_I2CWorker(void *parameters);

void task_I2CWorker(void *parameters)
{
    xQueueHandle_CMD_I2CWorker = xQueueCreate(4, sizeof(I2CCommand));
    xQueueHandle_MPU_I2CWorker = xQueueCreate(1, sizeof(MPUResult));
    // xQueueHandle_INA_I2CWorker = xQueueCreate(1, sizeof(INAResult));
    xTaskHandle_I2CWorker = xTaskGetCurrentTaskHandle();

    Wire.begin();
    InitOLEDScreen();
    playBootAnimation();
    InitTiltFusion();
    InitINA();
    I2CCommand cmd;
    MPUResult mpuResult;
    INAResult inaResult;
    for (;;)
    {
        if (xQueueReceive(xQueueHandle_CMD_I2CWorker, &cmd, pdMS_TO_TICKS(50)) == pdPASS)
        {
            switch (cmd.type)
            {
            case I2C_CMD_SET_OLED_CONFIG:
            //! even if I keep this method, please do not use it, unless you know what are you doing!!!
                // safeUpdateOLEDScreen(cmd.oledConfig);
                // Serial0_Println("Update OLED Screen");
                break;
            default: // YOU should never into this block
                break;
            }
        }
        safeUpdateINA();
        safeGetAngles(mpuResult);                                    // 每轮读一次姿态
        if (xQueueHandle_MPU_I2CWorker != NULL)
        {
            xQueueOverwrite(xQueueHandle_MPU_I2CWorker, &mpuResult); // 队列长度 1，覆盖写入
        }
        drawOLEDStatusBar();
        drawOLEDMainData();
    }
}

bool GetLatestMPU(MPUResult &out)
{
    if (xQueueHandle_MPU_I2CWorker == NULL) return false; // 队列由 I2C 任务创建，可能还没就绪
    return xQueuePeek(xQueueHandle_MPU_I2CWorker, &out, pdMS_TO_TICKS(0)) == pdTRUE;
}

// bool GetLatestINA(INAResult &out)
// {
//     return xQueuePeek(xQueueHandle_INA_I2CWorker, &out, pdMS_TO_TICKS(0)) == pdTRUE;
// }

bool SendI2CCommand(const I2CCommand &cmd)
{
    if (xQueueHandle_CMD_I2CWorker == NULL) return false;
    return xQueueSend(xQueueHandle_CMD_I2CWorker, &cmd, pdMS_TO_TICKS(0)) == pdPASS;
}