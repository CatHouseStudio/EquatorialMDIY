#pragma once
#include "Configuration.h"
#include "ntp.hpp"


// =====================
// Time & Sidereal Tools
// =====================

// Julian Date calculation (UTC)
double julian_date(int year, int month, int day, int hour, int min, int sec)
{
    if (month <= 2)
    {
        year--;
        month += 12;
    }
    int A = year / 100;
    int B = 2 - A + A / 4;
    double jd_day = int(365.25 * (year + 4716)) + int(30.6001 * (month + 1)) + day + B - 1524.5;
    double jd_time = (hour + min / 60.0 + sec / 3600.0) / 24.0;
    return jd_day + jd_time;
}

// Greenwich Sidereal Time (degrees)
double greenwich_sidereal_time(double jd)
{
    double T = (jd - 2451545.0) / 36525.0;
    double theta = 280.46061837 + 360.98564736629 * (jd - 2451545.0) + 0.000387933 * T * T - T * T * T / 38710000.0;
    return fmod(theta + 360.0, 360.0);
}

// =====================
// Precession (IAU1976)
// =====================
void precess_equatorial(double ra_in, double dec_in, int year, double &ra_out, double &dec_out)
{
    double T = (year - 2000.0) / 100.0;

    // Precession angles in arcseconds
    double zeta = (2306.2181 + 1.39656 * T - 0.000139 * T * T) * T / 3600.0;
    double z = (2306.2181 + 1.39656 * T - 0.000139 * T * T) * T / 3600.0;
    double theta = (2004.3109 - 0.85330 * T - 0.000217 * T * T) * T / 3600.0;

    zeta *= DEG_TO_RAD;
    z *= DEG_TO_RAD;
    theta *= DEG_TO_RAD;

    double ra_rad = ra_in * 15.0 * DEG_TO_RAD; // Convert hours to radians
    double dec_rad = dec_in * DEG_TO_RAD;

    double A = cos(dec_rad) * sin(ra_rad + zeta);
    double B = cos(theta) * cos(dec_rad) * cos(ra_rad + zeta) - sin(theta) * sin(dec_rad);
    double C = sin(theta) * cos(dec_rad) * cos(ra_rad + zeta) + cos(theta) * sin(dec_rad);

    double ra_new = atan2(A, B) + z;
    double dec_new = asin(C);

    ra_out = fmod(ra_new * RAD_TO_DEG / 15.0 + 24.0, 24.0); // Return in hours
    dec_out = dec_new * RAD_TO_DEG;
}

// =====================
// Nutation + Obliquity
// =====================

// Mean obliquity of the ecliptic (degrees)
double mean_obliquity(double T)
{
    double seconds = 84381.448 - 46.8150 * T - 0.00059 * T * T + 0.001813 * T * T * T;
    return seconds / 3600.0;
}

// Nutation in longitude (Δψ) and obliquity (Δε), degrees
void nutation(double T, double &delta_psi, double &delta_epsilon)
{
    double omega = DEG_TO_RAD * fmod(125.04452 - 1934.136261 * T, 360.0);
    delta_psi = -17.20 * sin(omega) / 3600.0;
    delta_epsilon = 9.20 * cos(omega) / 3600.0;
}

// Apply nutation correction to RA/Dec
void apply_nutation(double &ra, double &dec, double T)
{
    double eps0 = mean_obliquity(T);
    double dpsi, deps;
    nutation(T, dpsi, deps);

    eps0 *= DEG_TO_RAD;
    dpsi *= DEG_TO_RAD;

    double ra_rad = ra * 15.0 * DEG_TO_RAD;
    double dec_rad = dec * DEG_TO_RAD;

    ra_rad += (cos(eps0) + sin(eps0) * sin(ra_rad) * tan(dec_rad)) * dpsi;
    dec_rad += sin(eps0) * cos(ra_rad) * dpsi;

    ra = fmod(ra_rad * RAD_TO_DEG / 15.0 + 24.0, 24.0);
    dec = dec_rad * RAD_TO_DEG;
}

// =====================
// Final Az/Alt Calculation
// =====================
void calculate_az_alt(
    double latitude_deg, double longitude_deg,
    int year, int month, int day, int hour, int min, int sec,
    double ra_hours, double dec_deg,
    double &azimuth_deg, double &altitude_deg)
{
    double jd = julian_date(year, month, day, hour, min, sec);
    double T = (jd - 2451545.0) / 36525.0;
    double gst = greenwich_sidereal_time(jd);
    double lst = fmod(gst + longitude_deg + 360.0, 360.0);

    double ha = fmod(lst - ra_hours * 15.0 + 360.0, 360.0);
    if (ha > 180.0)
        ha -= 360.0;

    double ha_rad = ha * DEG_TO_RAD;
    double dec_rad = dec_deg * DEG_TO_RAD;
    double lat_rad = latitude_deg * DEG_TO_RAD;

    double sin_alt = sin(dec_rad) * sin(lat_rad) + cos(dec_rad) * cos(lat_rad) * cos(ha_rad);
    double alt_rad = asin(sin_alt);
    altitude_deg = alt_rad * RAD_TO_DEG;

    double cos_az = (sin(dec_rad) - sin(alt_rad) * sin(lat_rad)) / (cos(alt_rad) * cos(lat_rad));
    cos_az = fmax(fmin(cos_az, 1.0), -1.0);
    double az_rad = acos(cos_az);
    azimuth_deg = (sin(ha_rad) > 0) ? fmod(360.0 - az_rad * RAD_TO_DEG, 360.0) : az_rad * RAD_TO_DEG;
}

void CalculatePosition(int year, int month, int day, int hour, int minute, int second, // UTC Time
                       int lon_d, int lon_m, double lon_s,                             // example lon 44° 44′ 44″
                       int lat_d, int lat_m, double lat_s,                             // example lat 55° 55′ 55″
                       int ra_h, int ra_m, double ra_s,                                // example ra 20h 41m 25.91s
                       int dec_d, int dec_m, double dec_s,                             // example dec 45° 16′ 49.2s
                       float &azimuth, float &altitude)
{

    // UTC time
    // https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
    // initTime("CST-8"); // Set for Asia/Shanghai
    // struct tm timeinfo;
    // getLocalTime(&timeinfo);

    double lat = dms_to_degrees(lat_d, lat_m, lat_s);
    double lon = dms_to_degrees(lon_d, lon_m, lon_s);

    // Eta Cas (J2000 coordinates)
    double ra_j2000 = hms_to_hours(ra_h, ra_m, ra_s);
    double dec_j2000 = dms_to_degrees(dec_d, dec_m, dec_s);

    // Step1: Apply precession and nutation
    double ra_now, dec_now;
    precess_equatorial(ra_j2000, dec_j2000, year, ra_now, dec_now);
    double jd = julian_date(year, month, day, hour, minute, second);
    double T = (jd - 2451545.0) / 36525.0;
    apply_nutation(ra_now, dec_now, T);

    // Compute Az/Alt
    double az, alt;
    calculate_az_alt(lat, lon, year, month, day, hour, minute, second, ra_now, dec_now, az, alt);
}
