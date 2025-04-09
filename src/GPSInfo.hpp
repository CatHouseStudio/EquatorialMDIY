#pragma once
#include "Configuration.h"
#include "TinyGPSPlus.h"
#include "SerialMessage.hpp"
#include "freertos/semphr.h"
typedef struct
{
    float longitude; // 经度
    float latitude;  // 纬度
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} GPSInfo;

const char *getMonthAbbreviation(uint8_t month);
const char *getMonthAbbreviation(uint8_t month)
{
    static const char *monthNames[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    if (month >= 0 && month < 12)
    {
        return monthNames[month]; // 数组索引从0开始，月份从1开始
    }
    else
    {
        return "Unknown"; // 返回一个默认值或错误标志
    }
}

// GPS Object
TinyGPSPlus gps;

GPSInfo gps_info;
// Semaphore Mutex lock
SemaphoreHandle_t semphr_gps_info_Mutex = NULL;

void task_GPSInfo(void *parameter);

void task_GPSInfo(void *parameter)
{
    HardwareSerial SerialGPS(2);
    SerialGPS.begin(Serial_GPS_Bit_Rate);
    semphr_gps_info_Mutex = xSemaphoreCreateMutex();

    for (;;)
    {
        while (SerialGPS.available() > 0)
        {
            if (gps.encode(SerialGPS.read()))
            {
                // get the mutex
                if (xSemaphoreTake(semphr_gps_info_Mutex, portMAX_DELAY) == pdTRUE)
                {
                    // Make sure info is valid
                    if (gps.location.isValid() && gps.date.isValid() && gps.time.isValid())
                    {
                        gps_info.latitude = gps.location.lat();
                        gps_info.longitude = gps.location.lng();
                        gps_info.year = gps.date.year();
                        gps_info.month = gps.date.month();
                        gps_info.day = gps.date.day();
                        gps_info.hour = gps.time.hour();
                        gps_info.minute = gps.time.minute();
                        gps_info.second = gps.time.second();
                        const char *monthAbbrev = getMonthAbbreviation(gps_info.month);
                        // YYYY MM DD HH:MM:SS
                        Serial0_Printf("Now Date and Time: %u/%s/%02u %02u:%02u:%02u\n", gps_info.year, monthAbbrev, gps_info.day, gps_info.hour, gps_info.minute, gps_info.second);
                        Serial0_Printf("Now latitude and longitude: %.9f, %.9f", gps_info.latitude, gps_info.longitude);
                        // release the mutex
                        xSemaphoreGive(semphr_gps_info_Mutex);
                    }
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}