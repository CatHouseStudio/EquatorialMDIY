#pragma once
#include "SerialMessage.hpp"
#include <esp_wifi.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "SpiffsService.hpp"
void WiFi_AP_Init();
void WiFi_AP_Reboot(String ap_ssid, String ap_pwd);
void WiFi_STA_Init();

// 凭据合法性：SSID 1~32 字符；密码为空（开放网络）或 8~63 字符。
// 这是 WPA2 的硬约束：APClass::create() 对 1~7 位密码直接返回 false。
static bool AP_CredentialsValid(const String &ssid, const String &pwd)
{
    if (ssid.isEmpty() || ssid.length() > 32)
    {
        return false;
    }
    if (!pwd.isEmpty() && (pwd.length() < 8 || pwd.length() > 63))
    {
        return false;
    }
    return true;
}

// WiFi 连接状态机：由 WiFi_Poll() 周期驱动，不再阻塞任何任务
enum WiFiState
{
    WIFI_STATE_AP_ONLY, // 没配 STA，保持纯 AP
    WIFI_STATE_CONNECTING,
    WIFI_STATE_CONNECTED,
    WIFI_STATE_BACKOFF
};

static const uint32_t WIFI_CONNECT_TIMEOUT_MS = 8000;
static const uint32_t WIFI_BACKOFF_BASE_MS = 2000;

static WiFiState wifi_state = WIFI_STATE_AP_ONLY;
static uint32_t wifi_last_attempt = 0;
static uint8_t wifi_attempt = 0;

void WiFi_Poll();

void WiFi_STA_Init()
{
    WiFi.mode(WIFI_AP_STA);
    // 只设模式并决定初始状态，连接过程交给 WiFi_Poll()
    wifi_state = (sta_ssid[0] == '\0') ? WIFI_STATE_AP_ONLY : WIFI_STATE_BACKOFF;
    wifi_attempt = 0;
    wifi_last_attempt = 0; // 让第一次 Poll 立刻发起连接
    Serial0_Println("STA (Station) state machine armed");
}

// 由 web 任务的主循环周期调用
void WiFi_Poll()
{
    const uint32_t now = millis();
    switch (wifi_state)
    {
    case WIFI_STATE_AP_ONLY:
        break;

    case WIFI_STATE_CONNECTING:
        if (WiFi.status() == WL_CONNECTED)
        {
            wifi_attempt = 0;
            wifi_state = WIFI_STATE_CONNECTED;
            Serial0_Print("STA connected, IP: ");
            Serial0_Println(WiFi.localIP().toString());
        }
        else if (now - wifi_last_attempt > WIFI_CONNECT_TIMEOUT_MS)
        {
            Serial0_Println("STA connect timeout, backing off");
            wifi_last_attempt = now;
            wifi_state = WIFI_STATE_BACKOFF;
        }
        break;

    case WIFI_STATE_CONNECTED:
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial0_Println("STA lost, reconnecting...");
            wifi_last_attempt = now;
            wifi_state = WIFI_STATE_BACKOFF;
        }
        break;

    case WIFI_STATE_BACKOFF:
    {
        const uint32_t backoff =
            WIFI_BACKOFF_BASE_MS << (wifi_attempt > 4 ? 4 : wifi_attempt); // 2,4,8,16,32 s
        if (now - wifi_last_attempt < backoff)
        {
            return;
        }
        WiFi.begin(sta_ssid, sta_passwd);
        wifi_last_attempt = now;
        if (wifi_attempt < 4)
        {
            ++wifi_attempt;
        }
        wifi_state = WIFI_STATE_CONNECTING;
        break;
    }
    }
}

void WiFi_AP_Init()
{
    WiFi.mode(WIFI_AP);
    esp_wifi_set_ps(WIFI_PS_MIN_MODEM); // Set Wifi Power Save, if connect not stable, delete this line.

    IPAddress localIP(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);

    // 配置优先，读不到 / 不合法就用默认值
    String ap_ssid = default_ap_ssid;
    String ap_pwd = default_ap_password;
    JsonDocument configJson;
    if (ReadJsonFromFile(fs_path_config, configJson))
    {
        String s = configJson["ssid"] | "";
        String p = configJson["pwd"] | "";
        if (AP_CredentialsValid(s, p))
        {
            ap_ssid = s;
            ap_pwd = p;
            Serial0_Println("Using Customize configuration");
        }
        else
        {
            Serial0_Println("Invalid Json content, Using default configuration");
        }
    }
    else
    {
        Serial0_Println("JSON file error, Using default configuration");
    }

    Serial0_Println("Setting AP (Access Point)…");
    if (!WiFi.softAP(ap_ssid, ap_pwd))
    {
        Serial0_Println("AP start failed, fallback to default");
        WiFi.softAP(default_ap_ssid, default_ap_password);
    }
    WiFi.softAPConfig(localIP, gateway, subnet);
    Serial0_Print("AP IP address: ");
    Serial0_Println(WiFi.softAPIP().toString());
    vTaskDelay(100 / portTICK_PERIOD_MS);
    Serial0_Println("Set AP (Access Point) Finished!");
}

void WiFi_AP_Reboot(String ap_ssid, String ap_pwd)
{
    if (!AP_CredentialsValid(ap_ssid, ap_pwd))
    {
        Serial0_Println("Reject invalid AP credentials, keep current AP");
        return;
    }
    Serial0_Println("Resetting AP (Access Point)…");
    if (!WiFi.softAP(ap_ssid, ap_pwd))
    {
        Serial0_Println("AP restart failed, fallback to default");
        WiFi.softAP(default_ap_ssid, default_ap_password);
    }
}
