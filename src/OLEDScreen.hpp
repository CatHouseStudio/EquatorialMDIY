#pragma once
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

const uint8_t OLED_Text_Max_Length = 64;

struct OLEDConfig
{
    char text[OLED_Text_Max_Length];
    uint8_t text_size = 1;
    uint16_t text_color = SSD1306_WHITE; // ✅ 字体颜色
    uint16_t bg_color = SSD1306_BLACK;   // ✅ 背景色（目前 GFX 库不支持自动背景，但可手动处理）
    uint8_t cursor_x = 0;                // ✅ 光标位置 X
    uint8_t cursor_y = 0;                // ✅ 光标位置 Y
    bool clear_screen = true;            // ✅ 是否清屏（默认 true）
};

const uint8_t OLED_Screen_Height = 64;
const uint8_t OLED_Screen_Width = 128;

Adafruit_SSD1306 display(OLED_Screen_Width, OLED_Screen_Height, &Wire, -1);
SemaphoreHandle_t semphr_OLEDScreenMutex = NULL;
void InitOLEDScreen();

void InitOLEDScreen()
{
    semphr_OLEDScreenMutex = xSemaphoreCreateMutex();
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("System Booting...");
    display.display();
}

void updateOLEDScreen(const OLEDConfig &cfg);
void safeUpdateOLEDScreen(const OLEDConfig &cfg);

void updateOLEDScreen(const OLEDConfig &cfg)
{
    if (cfg.clear_screen)
    {
        display.clearDisplay();
    }
    display.setCursor(cfg.cursor_x, cfg.cursor_y);
    display.setTextSize(cfg.text_size > 0 ? cfg.text_size : 1);
    display.setTextColor(cfg.text_color);
    display.println(strlen(cfg.text) > 0 ? cfg.text : "(no text)");
    display.display();
}

void safeUpdateOLEDScreen(const OLEDConfig &cfg)
{
    if (xSemaphoreTake(semphr_OLEDScreenMutex, portMAX_DELAY) == pdTRUE)
    {
        updateOLEDScreen(cfg);
        xSemaphoreGive(semphr_OLEDScreenMutex);
    }
}