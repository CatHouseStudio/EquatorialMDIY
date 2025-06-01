#pragma once
#include "Configuration.h"
#include "CelestialPositioning.hpp"

// State Machine
enum StepperState
{
    STEPPER_IDLE,
    STEPPER_RUNNING
};

enum MoveAction
{
    ACTION_POSITION,   // 有限步数
    ACTION_CONTINUOUS, // 持续运行
    ACTION_STOP        // 中断当前运行, 这边判断用notify而不是cmd
};

enum StepperDirection
{
    DIR_WORK,
    DIR_INIT
};

struct MoveCommand
{
    MoveAction action;
    StepperDirection dir;
    uint32_t pulse_count;
    uint32_t delay_us;
    bool tracking;
};

uint32_t Pulse_RA(float azimuth);
uint32_t Pulse_DEC(float altitude);

static QueueHandle_t queueHandle_Stepper_RA = NULL;
static QueueHandle_t queueHandle_Stepper_DEC = NULL;

static TaskHandle_t xTaskHandle_Stepper_RA = NULL;
static TaskHandle_t xTaskHandle_Stepper_DEC = NULL;

static bool is_RA_running = false;
static bool is_DEC_running = false;

bool Is_Stepper_RA_running();
bool Is_Stepper_DEC_running();

bool Stepper_RA_SendCommand(const MoveCommand &cmd);
bool Stepper_DEC_SendCommand(const MoveCommand &cmd);

void Stepper_RA_Stop();
void Stepper_DEC_Stop();

void set_Stepper_RA_running(bool state);
void set_Stepper_DEC_running(bool state);

static SemaphoreHandle_t semphr_Stepper_RA_Mutex = NULL;
static SemaphoreHandle_t semphr_Stepper_DEC_Mutex = NULL;

void task_Stepper_RA(void *parameters);
void task_Stepper_DEC(void *parameters);

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

void task_Stepper_RA(void *parameters)
{
    queueHandle_Stepper_RA = xQueueCreate(1, sizeof(MoveCommand));
    xTaskHandle_Stepper_RA = xTaskGetCurrentTaskHandle();
    MoveCommand cmd;
    for (;;)
    {
        if (xQueueReceive(queueHandle_Stepper_RA, &cmd, portMAX_DELAY) == pdPASS)
        {
            // ACTION_STOP use notify, not normal CMD
            if (cmd.action != ACTION_POSITION && cmd.action != ACTION_CONTINUOUS)
            {
                continue;
            }
            set_Stepper_RA_running(true);
            // CONTINUOUS or POSITION
            if (cmd.dir == DIR_INIT)
            {
                digitalWrite(Pin_Stepper_RA_Dir, Stepper_RA_Initialize_Dir);
            }
            else
            {
                digitalWrite(Pin_Stepper_RA_Dir, Stepper_RA_Work_Dir);
            }

            uint32_t notifyValue = 0;

            uint32_t i = 0;
            while (cmd.action == ACTION_POSITION && i < cmd.pulse_count)
            {
                if (xTaskNotifyWait(0, 0, &notifyValue, 0) == pdPASS)
                { // We got a ACTION_STOP notify!
                    if (notifyValue == ACTION_STOP)
                    {
                        break;
                    }
                }
                digitalWrite(Pin_Stepper_RA_Step, HIGH);
                DelayUs(Stepper_RA_DelayMs);
                digitalWrite(Pin_Stepper_RA_Step, LOW);
                DelayUs(Stepper_RA_DelayMs);
                ++i;
            }

            while ((cmd.action == ACTION_CONTINUOUS) || (cmd.action == ACTION_POSITION && cmd.tracking))
            {
                if (xTaskNotifyWait(0, 0, &notifyValue, 0) == pdPASS)
                { // We got a ACTION_STOP notify!
                    if (notifyValue == ACTION_STOP)
                    {
                        break;
                    }
                }
                digitalWrite(Pin_Stepper_RA_Step, HIGH);
                DelayUs(cmd.delay_us);
                digitalWrite(Pin_Stepper_RA_Step, LOW);
                DelayUs(cmd.delay_us);
            }
            xTaskNotifyStateClear(NULL);
            set_Stepper_RA_running(false);
        }
    }
}
// configMAX_PRIORITIES - 3
void task_Stepper_DEC(void *parameters)
{
    queueHandle_Stepper_DEC = xQueueCreate(1, sizeof(MoveCommand));
    xTaskHandle_Stepper_DEC = xTaskGetCurrentTaskHandle();
    MoveCommand cmd;
    for (;;)
    {
        if (xQueueReceive(queueHandle_Stepper_DEC, &cmd, portMAX_DELAY) == pdPASS)
        {
            // ACTION_STOP use notify, not normal CMD
            if (cmd.action != ACTION_POSITION && cmd.action != ACTION_CONTINUOUS)
            {
                continue;
            }
            set_Stepper_DEC_running(true);
            // CONTINUOUS or POSITION
            if (cmd.dir == DIR_INIT)
            {
                digitalWrite(Pin_Stepper_DEC_Dir, Stepper_DEC_Initialize_Dir);
            }
            else
            {
                digitalWrite(Pin_Stepper_DEC_Dir, Stepper_DEC_Work_Dir);
            }

            uint32_t notifyValue = 0;

            uint32_t i = 0;
            while (cmd.action == ACTION_POSITION && i < cmd.pulse_count)
            {
                if (xTaskNotifyWait(0, 0, &notifyValue, 0) == pdPASS)
                { // We got a ACTION_STOP notify!
                    if (notifyValue == ACTION_STOP)
                    {
                        break;
                    }
                }
                digitalWrite(Pin_Stepper_DEC_Step, HIGH);
                DelayUs(Stepper_DEC_DelayMs);
                digitalWrite(Pin_Stepper_DEC_Step, LOW);
                DelayUs(Stepper_DEC_DelayMs);
                ++i;
            }

            while ((cmd.action == ACTION_CONTINUOUS) || (cmd.action == ACTION_POSITION && cmd.tracking))
            {
                if (xTaskNotifyWait(0, 0, &notifyValue, 0) == pdPASS)
                { // We got a ACTION_STOP notify!
                    if (notifyValue == ACTION_STOP)
                    {
                        break;
                    }
                }
                digitalWrite(Pin_Stepper_DEC_Step, HIGH);
                DelayUs(cmd.delay_us);
                digitalWrite(Pin_Stepper_DEC_Step, LOW);
                DelayUs(cmd.delay_us);
            }
            xTaskNotifyStateClear(NULL);
            set_Stepper_DEC_running(false);
        }
    }
}

bool Is_Stepper_RA_running()
{
    bool result = false;
    if (xSemaphoreTake(semphr_Stepper_RA_Mutex, portMAX_DELAY))
    {
        result = is_RA_running;
        xSemaphoreGive(semphr_Stepper_RA_Mutex);
    }
    return result;
}
bool Is_Stepper_DEC_running()
{
    bool result = false;
    if (xSemaphoreTake(semphr_Stepper_DEC_Mutex, portMAX_DELAY))
    {
        result = is_DEC_running;
        xSemaphoreGive(semphr_Stepper_DEC_Mutex);
    }
    return result;
}

void set_Stepper_RA_running(bool state)
{
    if (xSemaphoreTake(semphr_Stepper_RA_Mutex, portMAX_DELAY))
    {
        is_RA_running = state;
        xSemaphoreGive(semphr_Stepper_RA_Mutex);
    }
}
void set_Stepper_DEC_running(bool state)
{
    if (xSemaphoreTake(semphr_Stepper_DEC_Mutex, portMAX_DELAY))
    {
        is_DEC_running = state;
        xSemaphoreGive(semphr_Stepper_DEC_Mutex);
    }
}
bool Stepper_RA_SendCommand(const MoveCommand &cmd)
{
    if (Is_Stepper_RA_running())
    {
        // task is running, can not send command
        return false;
    }
    return xQueueSend(queueHandle_Stepper_RA, &cmd, (TickType_t)0) == pdPASS; // if queue is full, will return errQUEUE_FULL
}
bool Stepper_DEC_SendCommand(const MoveCommand &cmd)
{
    if (Is_Stepper_DEC_running())
    {
        // task is running, can not send command
        return false;
    }
    return xQueueSend(queueHandle_Stepper_DEC, &cmd, (TickType_t)0) == pdPASS; // if queue is full, will return errQUEUE_FULL
}

void Stepper_RA_Stop()
{
    xTaskNotify(xTaskHandle_Stepper_RA, ACTION_STOP, eSetValueWithOverwrite);
}
void Stepper_DEC_Stop()
{
    xTaskNotify(xTaskHandle_Stepper_DEC, ACTION_STOP, eSetValueWithOverwrite);
}