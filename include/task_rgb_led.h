#pragma once

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "global.h"
// Chân và số lượng LED
#define PIN_NEO_PIXEL 6 // Chân của Yolobit được kết nối với NeoPixel
#define NUM_PIXELS 4    // Số LED trên NeoPixel

// Khai báo task
void task_rgb_led(void *pvParameters);

// Các mode LED
enum LedMode
{
    LED_OFF = 0,
    LED_STATIC,
    LED_BREATHING,
    LED_RAINBOW,
    LED_WARNING
};

// Cấu trúc 1 khung giờ
typedef struct
{
    int startHour;   // giờ bắt đầu (0–23)
    int endHour;     // giờ kết thúc (0–23)
    LedMode mode;    // kiểu hiển thị
    uint8_t r, g, b; // màu nếu là STATIC
} LedSchedule;

// Các biến toàn cục liên quan tới LED (được định nghĩa ở .cpp)
extern Adafruit_NeoPixel NeoPixel;
extern LedSchedule ledSchedules[];
extern const int LED_SCHEDULE_COUNT;
