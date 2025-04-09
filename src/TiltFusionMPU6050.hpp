#pragma once
#include <Wire.h>
#include <MPU6050.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

MPU6050 mpu;
SemaphoreHandle_t semphr_tiltMutex = NULL; // RTOS互斥锁

const float AcceRatio = 16384.0f;
const float GyroRatio = 131.0f;
const int FilterWindow = 8;

static float aaxs[FilterWindow] = {0}, aays[FilterWindow] = {0}, aazs[FilterWindow] = {0};
static float aax, aay, aaz;
static float agx = 0, agy = 0, agz = 0;
static float axo = 0, ayo = 0, azo = 0;
static float gxo = 0, gyo = 0, gzo = 0;
static float Px = 1, Py = 1, Pz = 1;
static float offset_roll = 0, offset_pitch = 0, offset_ztilt = 0;

static unsigned long last_time = 0;
static bool useRadians = false;

void InitTiltFusion()
{
    semphr_tiltMutex = xSemaphoreCreateMutex(); // 初始化互斥锁
    mpu.initialize();
    delay(100);
    for (int i = 0; i < 200; i++)
    {
        int16_t ax, ay, az, gx, gy, gz;
        mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
        axo += ax;
        ayo += ay;
        azo += az;
        gxo += gx;
        gyo += gy;
        gzo += gz;
        delay(2);
    }
    axo /= 200;
    ayo /= 200;
    azo /= 200;
    gxo /= 200;
    gyo /= 200;
    gzo /= 200;
    last_time = millis();
}

void setOutputRadians(bool rad)
{
    useRadians = rad;
}

void calibrateZero()
{
    offset_roll = agx;
    offset_pitch = agy;
    offset_ztilt = agz;
}

void updateTiltFusion()
{
    unsigned long now = millis();
    float dt = (now - last_time) / 1000.0f;
    last_time = now;

    int16_t ax_raw, ay_raw, az_raw, gx_raw, gy_raw, gz_raw;
    mpu.getMotion6(&ax_raw, &ay_raw, &az_raw, &gx_raw, &gy_raw, &gz_raw);

    float ax = (ax_raw - axo) / AcceRatio;
    float ay = (ay_raw - ayo) / AcceRatio;
    float az = (az_raw - azo) / AcceRatio;
    float gx = (gx_raw - gxo) / GyroRatio;
    float gy = (gy_raw - gyo) / GyroRatio;
    float gz = (gz_raw - gzo) / GyroRatio;

    float accel_roll = atan2f(ay, az) * 180.0f / PI;
    float accel_pitch = atan2f(-ax, sqrtf(ay * ay + az * az)) * 180.0f / PI;
    float accel_ztilt = acosf(az / sqrtf(ax * ax + ay * ay + az * az)) * 180.0f / PI;

    float sum_roll = 0, sum_pitch = 0, sum_ztilt = 0;
    for (int i = 1; i < FilterWindow; i++)
    {
        aaxs[i - 1] = aaxs[i];
        sum_roll += aaxs[i] * i;
        aays[i - 1] = aays[i];
        sum_pitch += aays[i] * i;
        aazs[i - 1] = aazs[i];
        sum_ztilt += aazs[i] * i;
    }
    aaxs[FilterWindow - 1] = accel_roll;
    aays[FilterWindow - 1] = accel_pitch;
    aazs[FilterWindow - 1] = accel_ztilt;
    sum_roll += accel_roll * FilterWindow;
    sum_pitch += accel_pitch * FilterWindow;
    sum_ztilt += accel_ztilt * FilterWindow;

    aax = (sum_roll / (11 * FilterWindow / 2.0f)) * 9 / 7.0f;
    aay = (sum_pitch / (11 * FilterWindow / 2.0f)) * 9 / 7.0f;
    aaz = (sum_ztilt / (11 * FilterWindow / 2.0f)) * 9 / 7.0f;

    float gyro_roll = -gx * dt;
    float gyro_pitch = -gy * dt;
    float gyro_ztilt = -gz * dt;

    agx += gyro_roll;
    agy += gyro_pitch;
    agz += gyro_ztilt;

    float Rx = sq(aax - agx), Ry = sq(aay - agy), Rz = sq(aaz - agz);

    Px += 0.0025f;
    float Kx = Px / (Px + Rx);
    agx += Kx * (aax - agx);
    Px *= (1 - Kx);

    Py += 0.0025f;
    float Ky = Py / (Py + Ry);
    agy += Ky * (aay - agy);
    Py *= (1 - Ky);

    Pz += 0.0025f;
    float Kz = Pz / (Pz + Rz);
    agz += Kz * (aaz - agz);
    Pz *= (1 - Kz);
}

void getAngles(float &roll, float &pitch, float &ztilt)
{
    roll = agx;
    pitch = agy;
    ztilt = agz;
    if (useRadians)
    {
        roll *= PI / 180.0f;
        pitch *= PI / 180.0f;
        ztilt *= PI / 180.0f;
    }
}

void getZeroedAngles(float &roll, float &pitch, float &ztilt)
{
    getAngles(roll, pitch, ztilt);
    roll -= offset_roll;
    pitch -= offset_pitch;
    ztilt -= offset_ztilt;
}

// ✅ 线程安全接口
void safeUpdateTiltFusion()
{
    if (xSemaphoreTake(semphr_tiltMutex, portMAX_DELAY) == pdTRUE)
    {
        updateTiltFusion();
        xSemaphoreGive(semphr_tiltMutex);
    }
}

void safeGetAngles(float &roll, float &pitch, float &ztilt)
{
    if (xSemaphoreTake(semphr_tiltMutex, portMAX_DELAY) == pdTRUE)
    {
        getAngles(roll, pitch, ztilt);
        xSemaphoreGive(semphr_tiltMutex);
    }
}

void safeGetZeroedAngles(float &roll, float &pitch, float &ztilt)
{
    if (xSemaphoreTake(semphr_tiltMutex, portMAX_DELAY) == pdTRUE)
    {
        getZeroedAngles(roll, pitch, ztilt);
        xSemaphoreGive(semphr_tiltMutex);
    }
}