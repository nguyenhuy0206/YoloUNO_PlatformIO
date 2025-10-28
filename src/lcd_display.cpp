#include "lcd_display.h"
LiquidCrystal_I2C lcd(33, 16, 2);

void lcd_display(void *pvParameters)
{
    lcd.init();
    lcd.backlight();

    while (1)
    {
        if (xSemaphoreTake(xBinarySemaphoreInternet, portMAX_DELAY) == pdTRUE)
        {
            if (xQueueReceive(xQueueSensor, &data, 0) == pdPASS)
            {

                String state;
                if (data.temperature >= 40 || data.humidity <= 30)
                    state = "CRITICAL";
                else if (data.temperature >= 35 || data.humidity <= 40)
                    state = "WARNING";
                else
                    state = "NORMAL";

                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("State: " + state);
                lcd.setCursor(0, 1);
                lcd.printf("T:%.1f H:%.1f", data.temperature, data.humidity);

                Serial.printf("[LCD] %s | T=%.1f H=%.1f\n",
                              state.c_str(), data.temperature, data.humidity);
            }
        }
    }
}
