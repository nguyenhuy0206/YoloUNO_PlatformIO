#include "neo_blinky.h"

void neo_blinky(void *pvParameters)
{
    Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
    strip.begin();
    strip.clear();
    strip.show();

    while (1)
    {
        if (xSemaphoreTake(xSemaphoreNeoLed, portMAX_DELAY) == pdTRUE)
        {
            if (xQueueReceive(xQueueSensor, &data, 0) != pdPASS)
            {
                if (data.humidity < 40)
                    strip.setPixelColor(0, strip.Color(255, 0, 0));
                else if (data.humidity < 70)
                    strip.setPixelColor(0, strip.Color(0, 255, 0));
                else
                    strip.setPixelColor(0, strip.Color(0, 0, 255));

                strip.show();
                Serial.printf("[NeoPixel] Color updated @H=%.1f%%\n", data.humidity);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50)); // tránh watchdog
    }
}
