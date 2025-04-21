#include "TiltFusionMPU6050.hpp"
// #include "INA219Sensor.hpp"
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
    InitTiltFusion();
    // InitINA219();
    InitOLEDScreen();

    I2CCommand cmd;
    MPUResult mpuResult;
    // INAResult inaResult;
    for (;;)
    {
        if (xQueueReceive(xQueueHandle_CMD_I2CWorker, &cmd, pdMS_TO_TICKS(20)) == pdPASS)
        {
            switch (cmd.type)
            {
            case I2C_CMD_SET_OLED_CONFIG:
                safeUpdateOLEDScreen(cmd.oledConfig);
                Serial0_Println("Update OLED Screen");
                break;
            default: // YOU should never into this block
                break;
            }
        }
        safeUpdateTiltFusion();
        safeGetAngles(mpuResult);
        xQueueOverwrite(xQueueHandle_MPU_I2CWorker, &mpuResult);
        // safeUpdateINA();
        // safeGetINA(inaResult);
        // xQueueOverwrite(xQueueHandle_INA_I2CWorker, &inaResult);
    }
}

bool GetLatestMPU(MPUResult &out)
{
    return xQueuePeek(xQueueHandle_MPU_I2CWorker, &out, pdMS_TO_TICKS(0)) == pdTRUE;
}

// bool GetLatestINA(INAResult &out)
// {
//     return xQueuePeek(xQueueHandle_INA_I2CWorker, &out, pdMS_TO_TICKS(0)) == pdTRUE;
// }

bool SendI2CCommand(const I2CCommand &cmd)
{
    return xQueueSend(xQueueHandle_CMD_I2CWorker, &cmd, pdMS_TO_TICKS(0)) == pdPASS;
}