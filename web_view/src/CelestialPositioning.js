// =====================
// Utility Conversions
// =====================
const DEG_TO_RAD = Math.PI / 180.0;
const RAD_TO_DEG = 180.0 / Math.PI;

// Convert h:m:s → hours
function hmsToHours(h, m, s) {
    return h + m / 60.0 + s / 3600.0;
}

// Convert d:m:s → decimal degrees
function dmsToDegrees(d, m, s) {
    const sign = d < 0 || m < 0 || s < 0 ? -1 : 1;
    return sign * (Math.abs(d) + Math.abs(m) / 60 + Math.abs(s) / 3600);
}

// Convert decimal degrees → degrees, minutes, seconds
function degreesToDMS(deg) {
    const sign = deg >= 0 ? 1 : -1;
    deg = Math.abs(deg);
    const d = Math.floor(deg);
    const m = Math.floor((deg - d) * 60);
    const s = ((deg - d) * 60 - m) * 60;
    return {
        d: d * sign,
        m,
        s
    };
}

// Convert degrees → h:m:s (for Azimuth display)
function degreesToHMS(deg) {
    deg = (deg + 360) % 360;
    const hours = deg / 15.0;
    const h = Math.floor(hours);
    const m = Math.floor((hours - h) * 60);
    const s = ((hours - h) * 60 - m) * 60;
    return {
        h,
        m,
        s
    };
}

// =====================
// Time & Sidereal Tools
// =====================

// Julian Date calculation (UTC)
function julianDate(year, month, day, hour, min, sec) {
    if (month <= 2) {
        year--;
        month += 12;
    }
    const A = Math.floor(year / 100);
    const B = 2 - A + Math.floor(A / 4);
    const jdDay = Math.floor(365.25 * (year + 4716)) + Math.floor(30.6001 * (month + 1)) + day + B - 1524.5;
    const jdTime = (hour + min / 60 + sec / 3600) / 24;
    return jdDay + jdTime;
}

// Greenwich Sidereal Time (degrees)
function greenwichSiderealTime(jd) {
    const T = (jd - 2451545.0) / 36525.0;
    let theta = 280.46061837 + 360.98564736629 * (jd - 2451545.0)
        + 0.000387933 * T * T - T * T * T / 38710000.0;
    return (theta % 360 + 360) % 360;
}

// =====================
// Precession (IAU1976)
// =====================
function precessEquatorial(raIn, decIn, year) {
    const T = (year - 2000) / 100.0;
    const zeta = DEG_TO_RAD * ((2306.2181 + 1.39656 * T - 0.000139 * T * T) * T / 3600);
    const z = DEG_TO_RAD * ((2306.2181 + 1.39656 * T - 0.000139 * T * T) * T / 3600);
    const theta = DEG_TO_RAD * ((2004.3109 - 0.85330 * T - 0.000217 * T * T) * T / 3600);

    const raRad = raIn * 15 * DEG_TO_RAD;
    const decRad = decIn * DEG_TO_RAD;

    const A = Math.cos(decRad) * Math.sin(raRad + zeta);
    const B = Math.cos(theta) * Math.cos(decRad) * Math.cos(raRad + zeta) - Math.sin(theta) * Math.sin(decRad);
    const C = Math.sin(theta) * Math.cos(decRad) * Math.cos(raRad + zeta) + Math.cos(theta) * Math.sin(decRad);

    const raOut = (Math.atan2(A, B) + z) * RAD_TO_DEG / 15;
    const decOut = Math.asin(C) * RAD_TO_DEG;

    return { ra: (raOut + 24) % 24, dec: decOut };
}

// =====================
// Nutation + Obliquity
// =====================

// Mean obliquity of the ecliptic (degrees)
function meanObliquity(T) {
    const seconds = 84381.448 - 46.8150 * T - 0.00059 * T ** 2 + 0.001813 * T ** 3;
    return seconds / 3600;
}

// Nutation in longitude (Δψ) and obliquity (Δε), degrees
function nutation(T) {
    const omega = DEG_TO_RAD * ((125.04452 - 1934.136261 * T) % 360);
    const deltaPsi = -17.20 * Math.sin(omega) / 3600;
    const deltaEpsilon = 9.20 * Math.cos(omega) / 3600;
    return { deltaPsi, deltaEpsilon };
}

// Apply nutation correction to RA/Dec
function applyNutation(ra, dec, T) {
    let eps0 = meanObliquity(T) * DEG_TO_RAD;
    const { deltaPsi } = nutation(T);
    const dpsi = deltaPsi * DEG_TO_RAD;

    let raRad = ra * 15 * DEG_TO_RAD;
    let decRad = dec * DEG_TO_RAD;

    raRad += (Math.cos(eps0) + Math.sin(eps0) * Math.sin(raRad) * Math.tan(decRad)) * dpsi;
    decRad += Math.sin(eps0) * Math.cos(raRad) * dpsi;

    return {
        ra: ((raRad * RAD_TO_DEG) / 15 + 24) % 24,
        dec: decRad * RAD_TO_DEG
    };
}

// =====================
// Final Az/Alt Calculation
// =====================
function calculateAzAlt(lat, lon, year, month, day, hour, min, sec, raHours, decDeg) {
    const jd = julianDate(year, month, day, hour, min, sec);
    const T = (jd - 2451545.0) / 36525.0;
    const gst = greenwichSiderealTime(jd);
    const lst = (gst + lon + 360) % 360;

    let ha = (lst - raHours * 15 + 360) % 360;
    if (ha > 180) ha -= 360;

    const haRad = ha * DEG_TO_RAD;
    const decRad = decDeg * DEG_TO_RAD;
    const latRad = lat * DEG_TO_RAD;

    const sinAlt = Math.sin(decRad) * Math.sin(latRad) + Math.cos(decRad) * Math.cos(latRad) * Math.cos(haRad);
    const altRad = Math.asin(sinAlt);
    const altitude = altRad * RAD_TO_DEG;

    let cosAz = (Math.sin(decRad) - Math.sin(altRad) * Math.sin(latRad)) / (Math.cos(altRad) * Math.cos(latRad));
    cosAz = Math.max(-1.0, Math.min(1.0, cosAz));
    const azRad = Math.acos(cosAz);

    const azimuth = (Math.sin(haRad) > 0) ? (360 - azRad * RAD_TO_DEG) % 360 : azRad * RAD_TO_DEG;

    return { azimuth, altitude };
}

// Final position calculation function in JavaScript
function calculatePosition(
    year, month, day, hour, minute, second, // UTC time
    lon_d, lon_m, lon_s,                    // Longitude: 44, 44, 44
    lat_d, lat_m, lat_s,                    // Latitude: 55, 55, 55
    ra_h, ra_m, ra_s,                      // RA: 20h 41m 25.91s
    dec_d, dec_m, dec_s                    // Dec: 45° 16′ 49.2″
) {
    const lat = dmsToDegrees(lat_d, lat_m, lat_s);
    const lon = dmsToDegrees(lon_d, lon_m, lon_s);

    const raJ2000 = hmsToHours(ra_h, ra_m, ra_s);
    const decJ2000 = dmsToDegrees(dec_d, dec_m, dec_s);

    // Step 1: Precession
    let { ra: raNow, dec: decNow } = precessEquatorial(raJ2000, decJ2000, year);

    // Step 2: Nutation
    const jd = julianDate(year, month, day, hour, minute, second);
    const T = (jd - 2451545.0) / 36525.0;
    ({ ra: raNow, dec: decNow } = applyNutation(raNow, decNow, T));

    // Step 3: Azimuth and Altitude
    const { azimuth, altitude } = calculateAzAlt(lat, lon, year, month, day, hour, minute, second, raNow, decNow);

    return { azimuth, altitude };
}



const result = calculatePosition(
    2025, 4, 12, 4, 30, 53,     // UTC
    121, 14, 1,                // Lon
    31, 22, 5,                // Lat
    0, 40, 30.44,             // RA
    56, 32, 14.4               // Dec
);

console.log(`Raw: Azimuth: ${result.azimuth.toFixed(2)}°, Altitude: ${result.altitude.toFixed(2)}°`);

// 格式化为 DMS 和 HMS
const azDMS = degreesToDMS(result.azimuth);
const altDMS = degreesToDMS(result.altitude);

console.log(`Azimuth (DMS): ${azDMS.d}° ${azDMS.m}′ ${azDMS.s.toFixed(2)}″`);
console.log(`Altitude (DMS): ${altDMS.d}° ${altDMS.m}′ ${altDMS.s.toFixed(2)}″`);