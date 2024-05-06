// RTC demo for ESP32, that includes TZ and DST adjustments
// Get the POSIX style TZ format string from  https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
// Created by Hardy Maxa
// Complete project details at: https://RandomNerdTutorials.com/esp32-ntp-timezones-daylight-saving/
#pragma once
#include "time.h"
#include "Configuration.h"
#include "WiFiApSta.hpp"
void setTimezone(String timezone);
void initTime(String timezone);
void setTime(int yr, int month, int mday, int hr, int minute, int sec, int isDst);

void setTimezone(String timezone)
{
    setenv("TZ", timezone.c_str(), 1); //  Now adjust the TZ.  Clock settings are adjusted to show the new local time
    tzset();
}

void initTime(String timezone)
{
    struct tm timeinfo;
    // Use ntp, we must connect to internet
    WiFi_STA_Init();
    configTime(0, 0, "ntp.ntsc.ac.cn"); // First connect to NTP server, with 0 TZ offset
    if (!getLocalTime(&timeinfo))
    {
        return;
    }
    // Now we can set the real timezone
    setTimezone(timezone);
}

// bool LocalTime(struct tm *timeinfo)
// {

//     if (!getLocalTime(timeinfo))
//     {
//         return false;
//     }
//     return true;
// }

void setTime(int yr, int month, int mday, int hr, int minute, int sec, int isDst)
{
    struct tm tm;

    tm.tm_year = yr - 1900; // Set date
    tm.tm_mon = month - 1;
    tm.tm_mday = mday;
    tm.tm_hour = hr; // Set time
    tm.tm_min = minute;
    tm.tm_sec = sec;
    tm.tm_isdst = isDst; // 1 or 0
    time_t t = mktime(&tm);
    struct timeval now = {.tv_sec = t};
    settimeofday(&now, NULL);
}