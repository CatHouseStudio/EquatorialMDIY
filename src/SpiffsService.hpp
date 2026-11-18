#pragma once
#include "Configuration.h"

// SPIFFS 不是线程安全的，用一把互斥量串行化即可。
// 不再需要"请求队列 + 服务任务 + 任务通知"三层同步：
//   - 去掉 String 成员被 xQueueSend 浅拷贝的隐患
//   - 去掉 portMAX_DELAY 的无限等待
//   - 每次调用都能拿到真实的成功/失败

static SemaphoreHandle_t spiffs_mutex = NULL;

static bool Spiffs_Lock()
{
    return spiffs_mutex != NULL &&
           xSemaphoreTake(spiffs_mutex, pdMS_TO_TICKS(2000)) == pdTRUE;
}

static void Spiffs_Unlock()
{
    xSemaphoreGive(spiffs_mutex);
}

bool WriteFile(const char *path, const String &content)
{
    if (!Spiffs_Lock())
    {
        return false;
    }
    bool ok = false;
    File f = SPIFFS.open(path, "w");
    if (f)
    {
        ok = (f.print(content) == content.length());
        f.close();
    }
    Spiffs_Unlock();
    return ok;
}

bool ReadFile(const char *path, String &out)
{
    if (!Spiffs_Lock())
    {
        return false;
    }
    bool ok = false;
    File f = SPIFFS.open(path, "r");
    if (f)
    {
        out = f.readString();
        ok = true;
        f.close();
    }
    Spiffs_Unlock();
    return ok;
}

bool WriteJsonToFile(const char *path, JsonDocument &doc)
{
    if (!Spiffs_Lock())
    {
        return false;
    }
    bool ok = false;
    File f = SPIFFS.open(path, "w");
    if (f)
    {
        ok = (serializeJson(doc, f) > 0);
        f.close();
    }
    Spiffs_Unlock();
    return ok;
}

bool ReadJsonFromFile(const char *path, JsonDocument &doc)
{
    if (!Spiffs_Lock())
    {
        return false;
    }
    bool ok = false;
    File f = SPIFFS.open(path, "r");
    if (f)
    {
        ok = (deserializeJson(doc, f) == DeserializationError::Ok);
        f.close();
    }
    Spiffs_Unlock();
    return ok;
}

void InitSpiffsService()
{
    if (!SPIFFS.begin(true))
    {
        ets_printf("[PANIC] SPIFFS mount failed\n");
        abort();
    }
    if (spiffs_mutex == NULL)
    {
        spiffs_mutex = xSemaphoreCreateMutex();
    }
    if (spiffs_mutex == NULL)
    {
        ets_printf("[PANIC] Failed to create spiffs mutex\n");
        abort();
    }
}
