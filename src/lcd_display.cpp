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
    SensorData recvData;
    while (1)
    {
        if (xQueueReceive(xQueueSensor, &recvData, portMAX_DELAY) == pdTRUE)
        {
            String state;
            if (recvData.temperature >= 40 || recvData.humidity <= 30)
                state = "CRITICAL";
            else if (recvData.temperature >= 35 || recvData.humidity <= 40)
                state = "WARNING";
            else
                state = "NORMAL";
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("State:       ");
            lcd.setCursor(0, 0);
            lcd.print("State: " + state);

            lcd.setCursor(0, 1);
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "T:%.1f H:%.1f ", recvData.temperature, recvData.humidity);
            lcd.print(buffer);
            Serial.printf("[LCD] T=%.1f H=%.1f\n", recvData.temperature, recvData.humidity);
        }
        else
        {
            Serial.println("[LCD] Waiting data...");
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
