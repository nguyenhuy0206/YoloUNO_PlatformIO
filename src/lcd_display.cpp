#include "lcd_display.h"

LiquidCrystal_I2C lcd(0x21, 16, 2);

void lcd_display(void *pvParameters)
{
    Serial.println("[LCD] Task started");
    vTaskDelay(pdMS_TO_TICKS(100));

    lcd.begin();
    lcd.backlight();
    lcd.clear();
    lcd.print("LCD Ready");
    Serial.println("[LCD] Init done");
    vTaskDelay(pdMS_TO_TICKS(1000));

    while (1)
    {
        if (xSemaphoreTake(xSemaphoreLCD, portMAX_DELAY) == pdTRUE)
        {
            if (xQueueReceive(xQueueSensor, &data, 0) == pdPASS)
            {
                float temperature = data.temperature;
                float humidity = data.humidity;

                const char *state;
                if (temperature >= 40 || humidity <= 30)
                    state = "CRITICAL";
                else if (temperature >= 35 || humidity <= 40)
                    state = "WARNING";
                else
                    state = "NORMAL";

                // Cập nhật LCD
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("State: ");
                lcd.print(state);

                lcd.setCursor(0, 1);
                char buffer[32];
                snprintf(buffer, sizeof(buffer), "T:%.1f H:%.1f", temperature, humidity);
                lcd.print(buffer);

                Serial.printf("[LCD] T=%.1f H=%.1f (%s)\n", temperature, humidity, state);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // tránh chiếm CPU
    }
}
