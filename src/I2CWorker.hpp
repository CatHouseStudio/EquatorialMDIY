#include "TiltFusionMPU6050.hpp"
#include "INA219Sensor.hpp"
#include "OLEDScreen.hpp"

const uint8_t OLED_Text_Max_Length = 64;

enum I2CCommandType
{
    // I2C_CMD_GET_MPU,     // USE PEEK MODE
    // I2C_CMD_GET_INA,     // USE PEEK MODE
    I2C_CMD_SET_OLED_CONFIG
};

struct OLEDConfig
{
    char text[OLED_Text_Max_Length];
    uint8_t text_size = 1;
    uint16_t text_color = SSD1306_WHITE; // ✅ 字体颜色
    uint16_t bg_color = SSD1306_BLACK;   // ✅ 背景色（目前 GFX 库不支持自动背景，但可手动处理）
    uint8_t cursor_x = 0;                // ✅ 光标位置 X
    uint8_t cursor_y = 0;                // ✅ 光标位置 Y
    bool clear_screen = true;            // ✅ 是否清屏（默认 true）
};

struct I2CCommand
{
    I2CCommandType type;
    OLEDConfig oledConfig;
};

struct MPUResult
{
    float roll;
    float pitch;
    float ztilt;
};

struct INAResult
{
    float voltage;
    float current;
    float power;
};

QueueHandle_t xQueueHandle_CMD_I2CWorker = NULL;
QueueHandle_t xQueueHandle_MPU_I2CWorker = NULL;
QueueHandle_t xQueueHandle_INA_I2CWorker = NULL;
TaskHandle_t xTaskHandle_I2CWorker = NULL;

bool GetLatestMPU(MPUResult &out);
bool GetLatestINA(INAResult &out);

void taskI2CWorker(void *parameters);

void taskI2CWorker(void *parameters)
{
    xQueueHandle_CMD_I2CWorker = xQueueCreate(4, sizeof(I2CCommand));
    xQueueHandle_MPU_I2CWorker = xQueueCreate(1, sizeof(MPUResult));
    xQueueHandle_INA_I2CWorker = xQueueCreate(1, sizeof(INAResult));
    xTaskHandle_I2CWorker = xTaskGetCurrentTaskHandle();
    Wire.begin();
    InitTiltFusion();
    I2CCommand cmd;
    MPUResult mpuResult;
    INAResult inaResult;
    for (;;)
    {
        if (xQueueReceive(xQueueHandle_CMD_I2CWorker, &cmd, pdMS_TO_TICKS(20)) == pdPASS)
        {
            switch (cmd.type)
            {
            case I2C_CMD_SET_OLED_CONFIG:
                // SET OLED
                break;
            default: // YOU should never into this block
                break;
            }
        }
        safeUpdateTiltFusion();
        safeGetAngles(mpuResult.roll, mpuResult.pitch, mpuResult.ztilt);
        xQueueOverwrite(xQueueHandle_MPU_I2CWorker, &mpuResult);
        // safeUpdateINA();
        // safeGetINA();
        // xQueueOverWrite();
    }
}

bool GetLatestMPU(MPUResult &out)
{
    return xQueuePeek(xQueueHandle_MPU_I2CWorker, &out, pdMS_TO_TICKS(0)) == pdTRUE;
}

bool GetLatestINA(INAResult &out)
{
    return xQueuePeek(xQueueHandle_INA_I2CWorker, &out, pdMS_TO_TICKS(0)) == pdTRUE;
}