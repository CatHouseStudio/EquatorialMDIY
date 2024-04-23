#include "SerialMessage.hpp"
#include <WiFi.h>
#include <AsyncTCP.h>
void WiFi_AP_STA_Init();

void WiFi_AP_STA_Init()
{
    WiFi.mode(WIFI_AP_STA);
    //! you must change the wifi ssid and passwd
    const char *sta_ssid = "ssid";
    const char *sta_passwd = "password";
    WiFi.begin(sta_ssid, sta_passwd);
    while (WiFi.status() != WL_CONNECTED)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    xQueueSend(queueHandle_Serial0, &"Setting STA (Station)…", (TickType_t)0);
    char sta_ip[100] = "STA IP address: ";
    strcat(sta_ip, WiFi.localIP().toString().c_str());
    const char *staIP = sta_ip;
    xQueueSend(queueHandle_Serial0, &staIP, (TickType_t)0);
    vTaskDelay(100 / portTICK_PERIOD_MS);

    // AP
    // Replace with your network credentials
    const char *ap_ssid = "ESP32-Access-Point";
    const char *ap_password = "123456789";
    xQueueSend(queueHandle_Serial0, &"Setting AP (Access Point)…", (TickType_t)0);
    WiFi.softAP(ap_ssid, ap_password);
    char ap_ip[100] = "AP IP address: ";
    strcat(ap_ip, WiFi.localIP().toString().c_str());
    const char *apIP = ap_ip;
    xQueueSend(queueHandle_Serial0, &apIP, (TickType_t)0);
}