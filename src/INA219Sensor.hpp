#pragma once
#include <Wire.h>
#include <Adafruit_INA219.h>
#include "SerialMessage.hpp"

Adafruit_INA219 ina219;
SemaphoreHandle_t semphr_ina219Mutex = NULL;

struct INAResult
{
    float busVoltage_V;
    float current_mA;
    float power_mW;
    float shuntVoltage_mW;
};

static float busVoltage_V = 0;
static float current_mA = 0;
static float power_mW = 0;
static float shuntVoltage_mW = 0;

void InitINA219();
void updateINA();
void getINA(INAResult &inaResult);
void safeUpdateINA();
void safeGetINA(INAResult &inaResult);

void InitINA219()
{
    semphr_ina219Mutex = xSemaphoreCreateMutex();
    if (!ina219.begin())
    {
        Serial0_Println("INA219 initialize failed!");
        return;
    }
    vTaskDelay(pdMS_TO_TICKS(100));
    ina219.getBusVoltage_V();
    ina219.getCurrent_mA();
    ina219.getPower_mW();
    ina219.getShuntVoltage_mV();
}

void updateINA()
{
    busVoltage_V = ina219.getBusVoltage_V();
    current_mA = ina219.getCurrent_mA();
    power_mW = ina219.getPower_mW();
    shuntVoltage_mW = ina219.getShuntVoltage_mV();
}
void safeUpdateINA()
{
    if (xSemaphoreTake(semphr_ina219Mutex, portMAX_DELAY) == pdTRUE)
    {
        updateINA();
        xSemaphoreGive(semphr_ina219Mutex);
    }
}
void getINA(INAResult &inaResult)
{
    inaResult.busVoltage_V = busVoltage_V;
    inaResult.current_mA = current_mA;
    inaResult.power_mW = power_mW;
    inaResult.shuntVoltage_mW = shuntVoltage_mW;
}
void safeGetINA(INAResult &inaResult)
{
    if (xSemaphoreTake(semphr_ina219Mutex, portMAX_DELAY) == pdTRUE)
    {
        getINA(inaResult);
        xSemaphoreGive(semphr_ina219Mutex);
    }
}