#pragma once
#include "Configuration.h"
#include "CelestialPositioning.hpp"

enum MoveMode
{
    MODE_POSITION,  // 有限步数
    MODE_CONTINUOUS // 持续运行
};

enum StepperDirection
{
    DIR_INIT,
    DIR_WORK
};

struct MoveCommand
{
    MoveMode mode;
    StepperDirection dir;
    uint32_t pulse_count;
    uint32_t delay_us;
};

uint32_t Pulse_RA(float azimuth);
uint32_t Pulse_DEC(float altitude);

static TaskHandle_t xTaskHandle_Move_RA = NULL;
static TaskHandle_t xTaskHandle_Move_DEC = NULL;

void task_Move_RA(void *parameters);
void task_Move_DEC(void *parameters);

// 相对位置偏移运动量计算

uint32_t Pulse_RA(float ra)
{
    // TODO Depend on Mechanical structure reduction ratio coefficient
    return (uint32_t)(ra * Stepper_RA_K);
}

uint32_t Pulse_DEC(float dec)
{
    // TODO Depend on Mechanical structure reduction ratio coefficient
    return (uint32_t)(dec * Stepper_DEC_K);
}

// configMAX_PRIORITIES - 3

// To call this task, you should use pvPortMalloc to malloc mem on heap
void task_Move_RA(void *parameters)
{
    // MoveCommand *cmd =(MoveCommand*)pvPortMalloc(sizeof(MoveCommand));
    // cmd->mode = MODE_CONTINUOUS;
    // cmd->dir = DIR_WORK
    // cmd->pulse_count = 1;          // only use under MODE_POSITION
    // cmd->delay_us = 187500;        // only use under MODE_CONTINUOUS
    // xTaskCreate(task_Move_RA, "Move RA", 2048, cmd, configMAX_PRIORITIES - 3, &xTaskHandle_Move_RA);
    MoveCommand *cmd = (MoveCommand *)parameters;
    MoveMode mode = cmd->mode;
    StepperDirection dir = cmd->dir;
    uint32_t pulse_count = cmd->pulse_count;
    uint32_t delay_us = cmd->delay_us;
    vPortFree(parameters);

    uint32_t pulse_generated = 0;
    const uint32_t batch_size = 100; // 一次生成的脉冲数量
    if (dir == DIR_INIT)
    {
        digitalWrite(Pin_Stepper_RA_Dir, Stepper_RA_Initialize_Dir);
    }
    else
    {
        digitalWrite(Pin_Stepper_RA_Dir, Stepper_RA_Work_Dir);
    }
    switch (mode)
    {
    case MODE_CONTINUOUS:
        for (;;)
        {
            digitalWrite(Pin_Stepper_RA_Step, HIGH);
            DelayUs(delay_us);
            digitalWrite(Pin_Stepper_RA_Step, LOW);
            DelayUs(delay_us);
        }
        break;

    case MODE_POSITION:
        while (pulse_generated < pulse_count)
        {
            for (uint32_t i = 0; i < batch_size && pulse_generated < pulse_count; ++i)
            {
                // 设置引脚为高电平
                digitalWrite(Pin_Stepper_RA_Step, HIGH);
                DelayUs(Stepper_RA_DelayMs);

                // 设置引脚为低电平
                digitalWrite(Pin_Stepper_RA_Step, LOW);
                DelayUs(Stepper_RA_DelayMs);

                pulse_generated++;
            }

            // 让出CPU时间片，防止阻塞其他任务
            vTaskDelay(pdMS_TO_TICKS(1)); // 每批脉冲后延迟1毫秒
        }
        break;
    }

    xTaskHandle_Move_RA = NULL;
    vTaskDelete(NULL);
}
// configMAX_PRIORITIES - 3
// To call this task, you should use pvPortMalloc to malloc mem on heap
void task_Move_DEC(void *parameters)
{
    // MoveCommand *cmd =(MoveCommand*)pvPortMalloc(sizeof(MoveCommand));
    // cmd->mode = MODE_CONTINUOUS;
    // cmd->dir = DIR_WORK
    // cmd->pulse_count = 1;          // only use under MODE_POSITION
    // cmd->delay_us = 187500;        // only use under MODE_CONTINUOUS
    // xTaskCreate(task_Move_RA, "Move RA", 2048, cmd, configMAX_PRIORITIES - 3, &xTaskHandle_Move_RA);
    MoveCommand *cmd = (MoveCommand *)parameters;
    MoveMode mode = cmd->mode;
    StepperDirection dir = cmd->dir;
    uint32_t pulse_count = cmd->pulse_count;
    uint32_t delay_us = cmd->delay_us;
    vPortFree(parameters);

    uint32_t pulse_generated = 0;
    const uint32_t batch_size = 100; // 一次生成的脉冲数量
    if (dir == DIR_INIT)
    {
        digitalWrite(Pin_Stepper_RA_Dir, Stepper_RA_Initialize_Dir);
    }
    else
    {
        digitalWrite(Pin_Stepper_RA_Dir, Stepper_RA_Work_Dir);
    }
    switch (mode)
    {
    case MODE_CONTINUOUS:
        for (;;)
        {
            digitalWrite(Pin_Stepper_DEC_Step, HIGH);
            DelayUs(delay_us);
            digitalWrite(Pin_Stepper_DEC_Step, LOW);
            DelayUs(delay_us);
        }
        break;
    case MODE_POSITION:
        while (pulse_generated < pulse_count)
        {
            for (uint32_t i = 0; i < batch_size && pulse_generated < pulse_count; ++i)
            {
                // 设置引脚为高电平
                digitalWrite(Pin_Stepper_DEC_Step, HIGH);
                DelayUs(Stepper_DEC_DelayMs);

                // 设置引脚为低电平
                digitalWrite(Pin_Stepper_DEC_Step, LOW);
                DelayUs(Stepper_DEC_DelayMs);

                pulse_generated++;
            }

            // 让出CPU时间片，防止阻塞其他任务
            vTaskDelay(pdMS_TO_TICKS(1)); // 每批脉冲后延迟1毫秒
        }
        break;
    }

    xTaskHandle_Move_DEC = NULL;
    vTaskDelete(NULL);
}
