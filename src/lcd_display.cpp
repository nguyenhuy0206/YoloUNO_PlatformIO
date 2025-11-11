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
        if (xSemaphoreTake(xSemaphoreLCD, pdMS_TO_TICKS(1000)) == pdPASS)
        {   
            
            lcd.setCursor(0, 0);
            lcd.print("T: " + String(data.temperature));
            lcd.setCursor(0, 1);
            lcd.print("H: " + String(data.humidity));
            Serial.printf("[LCD] T=%.1f H=%.1f\n", data.temperature, data.humidity);
        }
        else
        {
            Serial.println("[LCD] Waiting data...");
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
