#pragma once
#include <Wire.h>
#include "INA226.h"
#include "SerialMessage.hpp"

INA226 ina(0x40);
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

static float batteryPercent = 100.0;

void InitINA();
void updateINA();
void getINA(INAResult &inaResult);
void safeUpdateINA();
void safeGetINA(INAResult &inaResult);
void safeGetBatteryLevel(float &blv);

float estimateSOC(float voltage);

float estimateSOC(float voltage)
{
    const float voltage_points[] = {18.0, 19.2, 20.4, 21.6, 22.8, 23.6, 24.4, 25.2};
    const float soc_percent[]    = { 0.0, 10.0, 25.0, 50.0, 70.0, 80.0, 90.0, 100.0};
    const int count = sizeof(voltage_points) / sizeof(voltage_points[0]);

    if (voltage <= voltage_points[0]) return 0.0;
    if (voltage >= voltage_points[count - 1]) return 100.0;

    for (int i = 1; i < count; ++i) {
        if (voltage <= voltage_points[i]) {
            float v1 = voltage_points[i - 1];
            float v2 = voltage_points[i];
            float soc1 = soc_percent[i - 1];
            float soc2 = soc_percent[i];
            float ratio = (voltage - v1) / (v2 - v1);
            return soc1 + ratio * (soc2 - soc1);
        }
    }
    return 100.0;
}
void InitINA()
{
    semphr_ina219Mutex = xSemaphoreCreateMutex();
    if (!ina.begin())
    {
        Serial0_Println("INA initialize failed!");
        return;
    }
    Serial0_Println("INA initialize succeed!");
    vTaskDelay(pdMS_TO_TICKS(100));
    ina.setMaxCurrentShunt(1, 0.002);
    ina.getBusVoltage();
    ina.getCurrent_mA();
    ina.getPower_mW();
    ina.getShuntVoltage_mV();
}

void updateINA()
{
    busVoltage_V = ina.getBusVoltage();
    current_mA = ina.getCurrent_mA();
    power_mW = ina.getPower_mW();
    shuntVoltage_mW = ina.getShuntVoltage_mV();
    batteryPercent=estimateSOC(busVoltage_V);  
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
void safeGetBatteryLevel(float &blv){
    if (xSemaphoreTake(semphr_ina219Mutex, portMAX_DELAY) == pdTRUE)
    {
        blv=batteryPercent;
        xSemaphoreGive(semphr_ina219Mutex);
    }
}