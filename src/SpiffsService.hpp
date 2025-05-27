#pragma once
#include "Configuration.h"

enum SpiffsOpType
{
    READ,
    WRITE,
    READ_JSON,
    WRITE_JSON
};

struct SpiffsRequest
{
    SpiffsOpType type;
    const char *path;
    String data;           // 写字符串内容用
    String *result;        // 读字符串用
    JsonDocument *jsonDoc; // 读/写 JSON 时使用
    TaskHandle_t replyTo;
};
QueueHandle_t spiffsQueue;
void task_SpiffsService(void *parameters);
bool WriteFile(const char *path, const String &content);
bool ReadFile(const char *path, String &out);
bool WriteJsonToFile(const char *path, JsonDocument &doc);
bool ReadJsonFromFile(const char *path, JsonDocument &doc);
void InitSpiffsService();


void task_SpiffsService(void *parameters)
{
    SpiffsRequest req;
    while (true)
    {
        if (xQueueReceive(spiffsQueue, &req, portMAX_DELAY) == pdTRUE)
        {
            if (req.type == READ && req.result)
            {
                File f = SPIFFS.open(req.path, "r");
                if (f)
                {
                    *(req.result) = f.readString();
                    f.close();
                }
                else
                {
                    *(req.result) = "";
                }
            }
            else if (req.type == WRITE)
            {
                File f = SPIFFS.open(req.path, "w");
                if (f)
                {
                    f.print(req.data);
                    f.close();
                }
            }
            else if (req.type == READ_JSON && req.jsonDoc)
            {
                File f = SPIFFS.open(req.path, "r");
                if (f)
                {
                    deserializeJson(*(req.jsonDoc), f);
                    f.close();
                }
            }
            else if (req.type == WRITE_JSON && req.jsonDoc)
            {
                File f = SPIFFS.open(req.path, "w");
                if (f)
                {
                    serializeJson(*(req.jsonDoc), f);
                    f.close();
                }
            }
            if (req.replyTo)
            {
                xTaskNotifyGive(req.replyTo);
            }
        }
    }
}

bool WriteFile(const char *path, const String &content)
{
    SpiffsRequest req = {
        .type = WRITE,
        .path = path,
        .data = content,
        .result = nullptr,
        .replyTo = xTaskGetCurrentTaskHandle()};

    if (xQueueSend(spiffsQueue, &req, portMAX_DELAY) != pdTRUE)
        return false;

    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    return true;
}
bool ReadFile(const char *path, String &out)
{
    SpiffsRequest req = {
        .type = READ,
        .path = path,
        .data = "",
        .result = &out,
        .replyTo = xTaskGetCurrentTaskHandle()};

    if (xQueueSend(spiffsQueue, &req, portMAX_DELAY) != pdTRUE)
        return false;

    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    return true;
}
bool WriteJsonToFile(const char *path, JsonDocument &doc)
{
    SpiffsRequest req = {
        .type = WRITE_JSON,
        .path = path,
        .data = "",
        .result = nullptr,
        .jsonDoc = &doc,
        .replyTo = xTaskGetCurrentTaskHandle()};
    if (xQueueSend(spiffsQueue, &req, portMAX_DELAY) != pdTRUE)
        return false;

    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    return true;
}
bool ReadJsonFromFile(const char *path, JsonDocument &doc)
{
    SpiffsRequest req = {
        .type = READ_JSON,
        .path = path,
        .data = "",
        .result = nullptr,
        .jsonDoc = &doc,
        .replyTo = xTaskGetCurrentTaskHandle()};
    if (xQueueSend(spiffsQueue, &req, portMAX_DELAY) != pdTRUE)
        return false;

    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    return true;
}

void InitSpiffsService()
{
    if (!SPIFFS.begin(true))
    {
        ets_printf("[PANIC] SPIFFS mount failed\n");
        abort();
    }

    spiffsQueue = xQueueCreate(10, sizeof(SpiffsRequest));
    if (!spiffsQueue)
    {
        ets_printf("[PANIC] Failed to create spiffsQueue\n");
        abort();
    }
    xTaskCreate(
        task_SpiffsService,
        "Task SpiffsService",
        configMINIMAL_STACK_SIZE + 4096,
        NULL,
        configMAX_PRIORITIES - 2,
        NULL);
}