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
// TODO Use this function in WebServer
void CalculatePosition(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second,
                       float longitude, float latitude, // 示例经度和纬度：121.93, 30.9
                       float raHours, float decDegrees, // 示例北极星的赤经和赤纬：2.9667, 89.25
                       float &azimuth, float &altitude);// 方位角和高度角
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

void CalculatePosition(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second,
                       float longitude, float latitude, // 示例经度和纬度：121.93, 30.9
                       float raHours, float decDegrees, // 示例北极星的赤经和赤纬：2.9667, 89.25
                       float &azimuth, float &altitude)
{

    // UTC 时间
    // https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
    // initTime("CST-8"); // Set for Asia/Shanghai
    // struct tm timeinfo;
    // getLocalTime(&timeinfo);

    // float utcHours = 21 + 34.0 / 60 + 56.0 / 3600;
    //! Warning The time zone must be Asia/Shanghai(UTC+8)
    float utcHours = hour + (minute * 1.0) / 60 + (second * 1.0) / 3600;

    // 计算GST和LST
    float gst = calculateGST(year, month, day, utcHours);
    float lst = fmod(gst + longitude / 15.0, 24.0);
    float ha = fmod(lst * 15 - raHours * 15, 360); // 时角，单位转为度
    if (ha < 0)
        ha += 360;

    calculateAzimuthAndAltitude(ha, decDegrees, latitude, azimuth, altitude);
}
