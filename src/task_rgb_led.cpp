#include "task_rgb_led.h"
#include "global.h"
#include <Adafruit_NeoPixel.h>

// NeoPixel được define global ở mainserver.cpp:
// extern Adafruit_NeoPixel NeoPixel;

// Mô tả schedule (giữ để viết report / mở rộng sau)
LedSchedule ledSchedules[] = {
    {8, 9, LED_STATIC, 255, 0, 0},   // 8–9h: Morning scene (2 red pixels)
    {9, 17, LED_STATIC, 0, 0, 255},  // 9–17h: Day scene (2 blue + 2 purple)
    {17, 22, LED_STATIC, 255, 0, 0}, // 17–22h: Evening scene (1 red + 1 green)
    {22, 24, LED_OFF, 0, 0, 0},      // 22–24h: Off
    {0, 8, LED_OFF, 0, 0, 0}         // 0–8h: Off
};

const int LED_SCHEDULE_COUNT = sizeof(ledSchedules) / sizeof(ledSchedules[0]);

// Lấy giờ hiện tại (0–23); nếu chưa sync NTP thì fallback 12h trưa để demo
static int getCurrentHour()
{
    if (isWifiConnected)
    {
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo))
            return 0;
        return timeinfo.tm_hour;
    }
    else
    {
        return 12;
    }
}

// ===============================
// Các scene cụ thể cho NeoPixel
// ===============================

// 8–9h: 2 bóng đỏ
static void applySceneMorning()
{
    NeoPixel.clear();

    uint32_t red = NeoPixel.Color(255, 0, 0);
    if (NUM_PIXELS > 0)
        NeoPixel.setPixelColor(0, red);
    if (NUM_PIXELS > 1)
        NeoPixel.setPixelColor(1, red);

    NeoPixel.show();
}

// 9–17h: 2 blue + 2 purple
static void applySceneDay()
{
    NeoPixel.clear();

    uint32_t blue = NeoPixel.Color(0, 0, 255);
    uint32_t purple = NeoPixel.Color(128, 0, 128);

    if (NUM_PIXELS > 0)
        NeoPixel.setPixelColor(0, blue);
    if (NUM_PIXELS > 1)
        NeoPixel.setPixelColor(1, blue);
    if (NUM_PIXELS > 2)
        NeoPixel.setPixelColor(2, purple);
    if (NUM_PIXELS > 3)
        NeoPixel.setPixelColor(3, purple);

    NeoPixel.show();
}

// 17–22h: 1 đỏ + 1 xanh lá
static void applySceneEvening()
{
    NeoPixel.clear();

    uint32_t red = NeoPixel.Color(255, 0, 0);
    uint32_t green = NeoPixel.Color(0, 255, 0);

    if (NUM_PIXELS > 0)
        NeoPixel.setPixelColor(0, red);
    if (NUM_PIXELS > 1)
        NeoPixel.setPixelColor(1, green);

    NeoPixel.show();
}

// Các giờ còn lại: tắt hết
static void applySceneOff()
{
    NeoPixel.clear();
    NeoPixel.show();
}

// Hàm chọn scene theo giờ hiện tại
static void updateLedScheduleByTime()
{
    int hour = getCurrentHour(); // 0..23

    if (hour >= 8 && hour < 9)
    {
        applySceneMorning();
    }
    else if (hour >= 9 && hour < 17)
    {
        applySceneDay();
    }
    else if (hour >= 17 && hour < 22)
    {
        applySceneEvening();
    }
    else
    {
        applySceneOff();
    }
}

// =======================================
// FreeRTOS task điều khiển NeoPixel
// =======================================
void task_rgb_led(void *pvParameters)
{
    AppContext *ctx = (AppContext *)pvParameters;
    Serial.println("[RGB LED] Task started");

    // NeoPixel là global, chỉ cần init 1 lần
    NeoPixel.begin();
    NeoPixel.clear();
    NeoPixel.show();

    // In time nếu đã đồng bộ NTP
    if (isWifiConnected)
    {
        struct tm timeinfo;
        if (getLocalTime(&timeinfo))
        {
            char buffer[32];
            strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);
            Serial.print("[RGB LED] Current time: ");
            Serial.println(buffer);
        }
    }

    while (1)
    {
        if (currentMode == AUTO)
        {
            // Chờ sensor task bắn semaphore mỗi lần sample (10s),
            // timeout 5s để tránh bị kẹt nếu quên Give.
            xSemaphoreTake(ctx->xSemaphoreNeoLed, pdMS_TO_TICKS(5000));

            // Cập nhật scene theo giờ
            updateLedScheduleByTime();
        }
        else
        {
            // MANUAL: để user/webserver điều khiển, không auto ghi đè NeoPixel
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}
