#pragma once
#include "Configuration.h"
#include "CelestialPositioning.hpp"
int64_t Pluse_RA(float azimuth);
int64_t Pluse_DEC(float altitude);

static TaskHandle_t xTaskHandle_Move_RA = NULL;
static TaskHandle_t xTaskHandle_Move_DEC = NULL;

void task_Move_RA(void *parameters);
void task_Move_DEC(void *parameters);

// 相对位置偏移运动量计算

int64_t Pluse_RA(float ra)
{
    // TODO Depend on Mechanical structure reduction ratio coefficient
    return (uint64_t)(ra * Stepper_RA_K);
}

int64_t Pluse_DEC(float dec)
{
    // TODO Depend on Mechanical structure reduction ratio coefficient
    return (int64_t)(dec * Stepper_DEC_K);
}

// configMAX_PRIORITIES - 3

// To call this task, you should use pvPortMalloc to malloc mem on heap
void task_Move_RA(void *parameters)
{
    // int32_t param =(int32_t*)pvPortMalloc(sizeof(int32_t));
    // *param =pulse;

    int32_t pulse_count = *(int32_t *)parameters;
    vPortFree(parameters);
    uint32_t pulse_generated = 0;
    const uint32_t batch_size = 100; // 一次生成的脉冲数量
    if (pulse_count < 0)
    {
        digitalWrite(Pin_Stepper_RA_Dir, Stepper_RA_Initialize_Dir);
        pulse_count = -pulse_count;
    }
    else
    {
        digitalWrite(Pin_Stepper_RA_Dir, Stepper_RA_Work_Dir);
    }
    while (pulse_generated < pulse_count)
    {
        for (uint32_t i = 0; i < batch_size && pulse_generated < pulse_count; ++i)
        {
            // 设置引脚为高电平
            digitalWrite(Pin_Stepper_RA_Step, HIGH);
            delayMicroseconds(Stepper_RA_DelayMs);

            // 设置引脚为低电平
            digitalWrite(Pin_Stepper_RA_Step, LOW);
            delayMicroseconds(Stepper_RA_DelayMs);

            pulse_generated++;
        }

        // 让出CPU时间片，防止阻塞其他任务
        vTaskDelay(pdMS_TO_TICKS(1)); // 每批脉冲后延迟1毫秒
    }
    xTaskHandle_Move_RA = NULL;
    vTaskDelete(NULL);
}
// configMAX_PRIORITIES - 3
// To call this task, you should use pvPortMalloc to malloc mem on heap
void task_Move_DEC(void *parameters)
{
    // int32_t param =(int32_t*)pvPortMalloc(sizeof(int32_t));
    // *param =pulse;

    int32_t pulse_count = *(int32_t *)parameters;
    uint32_t pulse_generated = 0;
    const uint32_t batch_size = 100; // 一次生成的脉冲数量
    if (pulse_count < 0)
    {
        digitalWrite(Pin_Stepper_DEC_Dir, Stepper_DEC_Initialize_Dir);
        pulse_count = -pulse_count;
    }
    else
    {
        digitalWrite(Pin_Stepper_DEC_Dir, Stepper_DEC_Work_Dir);
    }
    while (pulse_generated < pulse_count)
    {
        for (uint32_t i = 0; i < batch_size && pulse_generated < pulse_count; ++i)
        {
            // 设置引脚为高电平
            digitalWrite(Pin_Stepper_DEC_Step, HIGH);
            delayMicroseconds(Stepper_DEC_DelayMs);

            // 设置引脚为低电平
            digitalWrite(Pin_Stepper_DEC_Step, LOW);
            delayMicroseconds(Stepper_DEC_DelayMs);

            pulse_generated++;
        }

        // 让出CPU时间片，防止阻塞其他任务
        vTaskDelay(pdMS_TO_TICKS(1)); // 每批脉冲后延迟1毫秒
    }
    xTaskHandle_Move_DEC = NULL;
    vTaskDelete(NULL);
}
