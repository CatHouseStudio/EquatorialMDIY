#pragma once
#include "SerialMessage.hpp"
#include <esp_wifi.h>
#include <WiFi.h>
#include <AsyncTCP.h>
void WiFi_AP_Init();
void WiFi_AP_Reboot(String ap_ssid, String ap_pwd);
void WiFi_STA_Init();

void WiFi_STA_Init()
{
    WiFi.mode(WIFI_AP_STA);
    // esp_wifi_set_ps(WIFI_PS_MIN_MODEM); // Set Wifi Power Save, if connect not stable, delete this line.
    //! you must change the wifi ssid and passwd
    const char *sta_ssid = "ax3000-818";
    const char *sta_passwd = "fl123456789";
    // esp_wifi_set_ps(WIFI_PS_MIN_MODEM); // Set Wifi Power Save, if connect not stable, delete this line.
    WiFi.begin(sta_ssid, sta_passwd);

    uint8_t retry_times = 0;

    Serial0_Println("Setting STA (Station)…");
    while (WiFi.status() != WL_CONNECTED && retry_times < 5)
    {
        Serial0_Println("Retry Connect to STA...");
        ++retry_times;
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    Serial0_Print("STA IP address: ");
    Serial0_Println(WiFi.localIP().toString());
    vTaskDelay(100 / portTICK_PERIOD_MS);
}

void WiFi_AP_Init()
{
    // AP
    // AP mode Ip config
    WiFi.mode(WIFI_AP);
    esp_wifi_set_ps(WIFI_PS_MIN_MODEM); // Set Wifi Power Save, if connect not stable, delete this line.
    IPAddress localIP(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    const char *default_ap_ssid = "ESP32-Access-Point";
    const char *default_ap_password = "123456789";
    // Read config.json from SPIFFS
    JsonDocument configJson;
    bool ok = ReadJsonFromFile(fs_path_config, configJson);
    if (ok)
    {
        if (!configJson["ssid"].is<String>() || !configJson["pwd"].is<String>())
        {
            Serial0_Println("Invalid Json content, Using default configuration");
            Serial0_Println("Setting AP (Access Point)…");
            WiFi.softAP(default_ap_ssid, default_ap_password);
            WiFi.softAPConfig(localIP, gateway, subnet);
            Serial0_Print("AP IP address: ");
            Serial0_Println(WiFi.softAPIP().toString());
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }
    else
    {
        // If in this case, SPIFFS is OK, but the JSON file is error
        Serial0_Println("JSON file error, Using default configuration");
        Serial0_Println("Setting AP (Access Point)…");
        WiFi.softAP(default_ap_ssid, default_ap_password);
        WiFi.softAPConfig(localIP, gateway, subnet);
        Serial0_Print("AP IP address: ");
        Serial0_Println(WiFi.softAPIP().toString());
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    Serial0_Println("Set AP (Access Point) Finished!");
}

void WiFi_AP_Reboot(String ap_ssid, String ap_pwd)
{
    Serial0_Println("Resetting AP (Access Point)…");
    WiFi.softAPdisconnect(true);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    WiFi.softAP(ap_ssid, ap_pwd);
}