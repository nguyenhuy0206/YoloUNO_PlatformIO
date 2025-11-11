// #include "lcd_display.h"

// // Sử dụng địa chỉ bạn đã tìm thấy
// // LiquidCrystal_I2C lcd(0x21, 16, 2);

// void lcd_display(void *pvParameters)
// {
//     Serial.println("[LCD] Task started");

//     // Wire.begin(11, 12);
//     vTaskDelay(pdMS_TO_TICKS(100));

//     lcd.begin();
//     lcd.backlight();
//     lcd.clear();
//     lcd.print("LCD Ready");
//     Serial.println("[LCD] Init done");

//     vTaskDelay(pdMS_TO_TICKS(1000));

//     while (1)
//     {
//         // Serial.printf("[LCD] Queue addr: %p\n", xQueueSensor);

//         if (xQueueReceive(xQueueSensor, &data, pdMS_TO_TICKS(3000)) == pdPASS)
//         {
//             String state;
//             if (data.temperature >= 40 || data.humidity <= 30)
//                 state = "CRITICAL";
//             else if (data.temperature >= 35 || data.humidity <= 40)
//                 state = "WARNING";
//             else
//                 state = "NORMAL";

//             lcd.clear();
//             lcd.setCursor(0, 0);
//             lcd.print("State: " + state);
//             lcd.setCursor(0, 1);

//             char buffer[32];
//             snprintf(buffer, sizeof(buffer), "T:%.1f H:%.1f",
//                      data.temperature, data.humidity);
//             lcd.print(buffer);

//             Serial.printf("[LCD] %s | T=%.1f H=%.1f\n",
//                           state.c_str(), data.temperature, data.humidity);
//         }
//         else
//         {
//             Serial.println("[LCD] Waiting data...");
//         }

//         vTaskDelay(pdMS_TO_TICKS(500));
//     }
// }
