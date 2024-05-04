#pragma once
#include "Configuration.h"
#include "ntp.hpp"
/* 示例内容
日期和时间：2024/5/4 21:34:56 UTC
位置：上海-滴水湖（大约的经度为121.93度，纬度为30.9度）
北极星赤经：2小时58分钟（约2.9667小时）
北极星赤纬：+89度15分（约89.25度）
*/

// 角度与弧度转换常数
const float degToRad = PI / 180.0;
const float radToDeg = 180.0 / PI;
float calculateGST(int year, int month, int day, float utcHours);
void calculateAzimuthAndAltitude(float HA, float Dec, float Latitude, float &Azimuth, float &Altitude);
void CalculatePosition();


// 计算GST
float calculateGST(int year, int month, int day, float utcHours)
{
    // 简化的Julian Date计算，适用于近期年份
    int a = (14 - month) / 12;
    int y = year + 4800 - a;
    int m = month + 12 * a - 3;
    long jdn = day + ((153 * m + 2) / 5) + 365 * y + y / 4 - y / 100 + y / 400 - 32045;

    float jd = jdn + (utcHours - 12) / 24;
    float d = jd - 2451545.0; // 从2000年1月1日12:00起的天数
    float t = d / 36525.0;
    float gst = 6.697374558 + 24.06570982441908 * d + 0.000026 * t * t;
    return fmod(gst, 24.0); // 确保结果在0到24小时之间
}

// 计算方位角和高度角
void calculateAzimuthAndAltitude(float HA, float Dec, float Latitude, float &Azimuth, float &Altitude)
{
    float HA_rad = HA * degToRad;
    float Dec_rad = Dec * degToRad;
    float Latitude_rad = Latitude * degToRad;

    float sin_Alt = sin(Dec_rad) * sin(Latitude_rad) + cos(Dec_rad) * cos(Latitude_rad) * cos(HA_rad);
    Altitude = asin(sin_Alt) * radToDeg;

    float cos_Az = (sin(Dec_rad) - sin(Altitude * degToRad) * sin(Latitude_rad)) / (cos(Altitude * degToRad) * cos(Latitude_rad));
    Azimuth = acos(cos_Az) * radToDeg;
    if (sin(HA_rad) > 0)
    {
        Azimuth = 360 - Azimuth;
    }
}

void CalculatePosition()
{

    // UTC 时间
    // https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
    initTime("CST-8"); // Set for Asia/Shanghai
    struct tm timeinfo;
    getLocalTime(&timeinfo);

    // float utcHours = 21 + 34.0 / 60 + 56.0 / 3600;
    float utcHours = timeinfo.tm_hour + (timeinfo.tm_min * 1.0) / 60 + (timeinfo.tm_sec * 1.0) / 3600;

    // 赤经和赤纬
    // get from api
    float raHours = 2 + 58.0 / 60;     // 北极星的赤经
    float decDegrees = 89 + 15.0 / 60; // 北极星的赤纬
    // 地理位置
    // get from gps
    float longitude = 121.93; // 经度
    float latitude = 30.9;    // 纬度

    // 计算GST和LST
    float gst = calculateGST(timeinfo.tm_year, timeinfo.tm_mon, timeinfo.tm_mday, utcHours);
    float lst = fmod(gst + longitude / 15.0, 24.0);
    float ha = fmod(lst * 15 - raHours * 15, 360); // 时角，单位转为度
    if (ha < 0)
        ha += 360;

    float azimuth, altitude;
    calculateAzimuthAndAltitude(ha, decDegrees, latitude, azimuth, altitude);
    // Serial.print("方位角: ");
    // Serial.println(azimuth);
    // Serial.print("高度角: ");
    // Serial.println(altitude);
}
