#pragma once
#include "SerialMessage.hpp"
#include <WiFi.h>
#include <AsyncTCP.h>
void WiFi_AP_Init();
void WiFi_AP_Reboot(String ap_ssid, String ap_pwd);
void WiFi_STA_Init();
void WiFi_STA_Reboot();

void WiFi_STA_Init()
{
    WiFi.mode(WIFI_AP_STA);
    //! you must change the wifi ssid and passwd
    const char *sta_ssid = "ax3000-818";
    const char *sta_passwd = "fl123456789";
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
}

void WiFi_AP_Init()
{
    // AP
    // AP mode Ip config
    IPAddress localIP(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    const char *default_ap_ssid = "ESP32-Access-Point";
    const char *default_ap_password = "123456789";
    // Read config.json from SPIFFS
    if (SPIFFS.begin(true))
    {
        File configFile = SPIFFS.open("/config.json");
        if (!configFile)
        {
            xQueueSend(queueHandle_Serial0, &"Failed to open JSON file, Using default configuration", (TickType_t)0);
            xQueueSend(queueHandle_Serial0, &"Setting AP (Access Point)…", (TickType_t)0);
            WiFi.softAP(default_ap_ssid, default_ap_password);
            WiFi.softAPConfig(localIP, gateway, subnet);
            char ap_ip[100] = "AP IP address: ";
            strcat(ap_ip, WiFi.localIP().toString().c_str());
            const char *apIP = ap_ip;
            xQueueSend(queueHandle_Serial0, &apIP, (TickType_t)0);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        else
        {
            JsonDocument configJson;
            DeserializationError error = deserializeJson(configJson, configFile);
            if (error || !configJson["ssid"].is<String>()|| !configJson["pwd"].is<String>())
            {
                xQueueSend(queueHandle_Serial0, &"Invalid Json content, Using default configuration", (TickType_t)0);
                xQueueSend(queueHandle_Serial0, &"Setting AP (Access Point)…", (TickType_t)0);
                WiFi.softAP(default_ap_ssid, default_ap_password);
                WiFi.softAPConfig(localIP, gateway, subnet);
                char ap_ip[100] = "AP IP address: ";
                strcat(ap_ip, WiFi.localIP().toString().c_str());
                const char *apIP = ap_ip;
                xQueueSend(queueHandle_Serial0, &apIP, (TickType_t)0);
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }
            else
            {
                xQueueSend(queueHandle_Serial0, &"Using customize configuration", (TickType_t)0);
                xQueueSend(queueHandle_Serial0, &"Setting AP (Access Point)…", (TickType_t)0);

                String ap_ssid = configJson["ssid"];
                String ap_pwd = configJson["pwd"];
                WiFi.softAP(ap_ssid, ap_pwd);
                WiFi.softAPConfig(localIP, gateway, subnet);
                char ap_ip[100] = "AP IP address: ";
                strcat(ap_ip, WiFi.localIP().toString().c_str());
                const char *apIP = ap_ip;
                xQueueSend(queueHandle_Serial0, &apIP, (TickType_t)0);
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }
        }
        configFile.close();
    }
    else
    {
        xQueueSend(queueHandle_Serial0, &"Failed to initialize SPIFFS, Using default configuration", (TickType_t)0);
        xQueueSend(queueHandle_Serial0, &"Setting AP (Access Point)…", (TickType_t)0);

        WiFi.softAP(default_ap_ssid, default_ap_password);
        WiFi.softAPConfig(localIP, gateway, subnet);
        char ap_ip[100] = "AP IP address: ";
        strcat(ap_ip, WiFi.localIP().toString().c_str());
        const char *apIP = ap_ip;
        xQueueSend(queueHandle_Serial0, &apIP, (TickType_t)0);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void WiFi_AP_Reboot(String ap_ssid, String ap_pwd)
{
    xQueueSend(queueHandle_Serial0, &"Resetting AP (Access Point)…", (TickType_t)0);
    WiFi.softAPdisconnect(true);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    WiFi.softAP(ap_ssid, ap_pwd);
}