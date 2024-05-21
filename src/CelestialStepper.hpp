#pragma once
#include "Configuration.h"

int64_t Pluse_Horizontal(float azimuth);
int64_t Pluse_Vertical(float altitude);

static TaskHandle_t xTaskHandle_Move_Horizontal = NULL;
static TaskHandle_t xTaskHandle_Move_Vertical = NULL;

void task_Move_Horizontal(void *parameters);
void task_Move_Vertical(void *parameters);

// 相对位置偏移运动量计算
// 水平运动量计算
int64_t Pluse_Horizontal(float azimuth)
{
    // TODO Depend on Mechanical structure reduction ratio coefficient
    return (uint64_t)(azimuth * Stepper_Horizontal_K);
}
// 垂直运动量计算
int64_t Pluse_Vertical(float altitude)
{
    // TODO Depend on Mechanical structure reduction ratio coefficient
    return (int64_t)(altitude * Stepper_Vertical_K);
}

// configMAX_PRIORITIES - 3
void task_Move_Horizontal(void *parameters)
{
    int64_t pulse_count = (int64_t)parameters;
    uint32_t pulse_generated = 0;
    const uint32_t batch_size = 100; // 一次生成的脉冲数量
    if (pulse_count < 0)
    {
        digitalWrite(Pin_Stepper_Horizontal_Dir, Stepper_Horizontal_Initialize_Dir);
    }
    else
    {
        digitalWrite(Pin_Stepper_Horizontal_Dir, Stepper_Horizontal_Work_Dir);
    }
    while (pulse_generated < pulse_count)
    {
        for (uint32_t i = 0; i < batch_size && pulse_generated < pulse_count; ++i)
        {
            // 设置引脚为高电平
            digitalWrite(Pin_Stepper_Horizontal_Step, HIGH);
            delayMicroseconds(Stepper_Horizontal_DelayMs);

            // 设置引脚为低电平
            digitalWrite(Pin_Stepper_Horizontal_Step, LOW);
            delayMicroseconds(Stepper_Horizontal_DelayMs);

            pulse_generated++;
        }

        // 让出CPU时间片，防止阻塞其他任务
        vTaskDelay(pdMS_TO_TICKS(1)); // 每批脉冲后延迟1毫秒
    }
    vTaskDelete(xTaskHandle_Move_Horizontal);
    xTaskHandle_Move_Horizontal = NULL;
}
// configMAX_PRIORITIES - 3
void task_Move_Vertical(void *parameters)
{
    int64_t pulse_count = (int64_t)parameters;
    uint32_t pulse_generated = 0;
    const uint32_t batch_size = 100; // 一次生成的脉冲数量
    if (pulse_count < 0)
    {
        digitalWrite(Pin_Stepper_Vertical_Dir, Stepper_Vertical_Initialize_Dir);
    }
    else
    {
        digitalWrite(Pin_Stepper_Vertical_Dir, Stepper_Vertical_Work_Dir);
    }
    while (pulse_generated < pulse_count)
    {
        for (uint32_t i = 0; i < batch_size && pulse_generated < pulse_count; ++i)
        {
            // 设置引脚为高电平
            digitalWrite(Pin_Stepper_Vertical_Step, HIGH);
            delayMicroseconds(Stepper_Vertical_DelayMs);

            // 设置引脚为低电平
            digitalWrite(Pin_Stepper_Vertical_Step, LOW);
            delayMicroseconds(Stepper_Vertical_DelayMs);

            pulse_generated++;
        }

        // 让出CPU时间片，防止阻塞其他任务
        vTaskDelay(pdMS_TO_TICKS(1)); // 每批脉冲后延迟1毫秒
    }
    vTaskDelete(xTaskHandle_Move_Vertical);
    xTaskHandle_Move_Vertical = NULL;
}
