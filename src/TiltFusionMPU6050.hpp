#pragma once
#include <Wire.h>
#include <MPU6050.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "SerialMessage.hpp"

MPU6050 mpu;
SemaphoreHandle_t semphr_tiltMutex = NULL; // RTOS互斥锁

struct MPUResult
{
    float roll;
    float pitch;
    float ztilt;
};


MPUResult zeroAngle;

void InitTiltFusion();
void getAngles(MPUResult &mpuResult);
void getZeroedAngles(MPUResult &mpuResult);

void getAngles(MPUResult &mpuResult)
{
    int16_t ax_raw, ay_raw, az_raw;
    mpu.getAcceleration(&ax_raw, &ay_raw, &az_raw);

    float ax = ax_raw / 16384.0f;
    float ay = ay_raw / 16384.0f;
    float az = az_raw / 16384.0f;

    mpuResult.roll  = atan2f(ay, az) * 180.0f / PI;
    mpuResult.pitch = atan2f(-ax, sqrtf(ay * ay + az * az)) * 180.0f / PI;

    float ztilt_ratio = az / sqrtf(ax * ax + ay * ay + az * az);
    mpuResult.ztilt = acosf(constrain(ztilt_ratio, -1.0f, 1.0f)) * 180.0f / PI;
}
void InitTiltFusion()
{
    semphr_tiltMutex = xSemaphoreCreateMutex();
    mpu.initialize();
    if (!mpu.testConnection())
    {
        Serial0_Println("MPU6050 initialize failed!");
        return;
    }

    Serial0_Println("MPU6050 initialized.");
    vTaskDelay(pdMS_TO_TICKS(300));
    getAngles(zeroAngle);
    Serial0_Println("Zero angle captured.");
}
void getZeroedAngles(MPUResult &mpuResult)
{
    MPUResult now;
    getAngles(now);
    mpuResult.roll  = now.roll  - zeroAngle.roll;
    mpuResult.pitch = now.pitch - zeroAngle.pitch;
    mpuResult.ztilt = now.ztilt - zeroAngle.ztilt;
}

// ✅ 线程安全接口
void safeGetAngles(MPUResult &mpuResult)
{
    if (xSemaphoreTake(semphr_tiltMutex, portMAX_DELAY) == pdTRUE)
    {
        getAngles(mpuResult);
        xSemaphoreGive(semphr_tiltMutex);
    }
}

void safeGetZeroedAngles(MPUResult &mpuResult)
{
    if (xSemaphoreTake(semphr_tiltMutex, portMAX_DELAY) == pdTRUE)
    {
        getZeroedAngles(mpuResult);
        xSemaphoreGive(semphr_tiltMutex);
    }
}