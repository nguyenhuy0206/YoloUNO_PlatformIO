#include "global.h"
#include <Arduino.h>

#include "led_blinky.h"
#include "neo_blinky.h"
#include "temp_humi_monitor.h"
#include "mainserver.h"
#include "tinyml.h"
#include "coreiot.h"
#include "lcd_display.h"
void setup()
{
  Serial.begin(115200);

  xQueueSensor = xQueueCreate(5, sizeof(SensorData));
  xBinarySemaphoreInternet = xSemaphoreCreateBinary();
  xSemaphoreLed = xSemaphoreCreateMutex();
  xSemaphoreNeoLed = xSemaphoreCreateMutex();
  xSemaphoreLCD = xSemaphoreCreateMutex();

  // Give internet semaphore ready
  xSemaphoreGive(xBinarySemaphoreInternet);

  xTaskCreate(led_blinky, "LED", 4096, NULL, 1, NULL);
  // xTaskCreate(lcd_display, "LCD", 4096, NULL, 1, NULL);
  // xTaskCreate(neo_blinky, "Neo", 2048, NULL, 1, NULL);
  xTaskCreate(temp_humi_monitor, "Sensor", 4096, NULL, 1, NULL);
  vTaskDelay(pdMS_TO_TICKS(500));
}

void loop()
{
}
// #include <Wire.h>
// #include <Arduino.h>
// void setup()
// {
//   Serial.begin(115200);
//   Wire.begin(11, 12); // SDA, SCL như bạn dùng
//   Serial.println("I2C Scanner running...");
// }

// void loop()
// {
//   for (uint8_t addr = 1; addr < 127; addr++)
//   {
//     Wire.beginTransmission(addr);
//     if (Wire.endTransmission() == 0)
//     {
//       Serial.printf("Found I2C device at 0x%02X\n", addr);
//     }
//   }
// }