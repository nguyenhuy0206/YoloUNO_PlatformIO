#include "led_blinky.h"

void led_blinky(void *pvParameters)
{
  pinMode(LED_GPIO, OUTPUT);

  while (1)
  {
    for (;;)
    {
      if (xSemaphoreTake(xBinarySemaphoreInternet, portMAX_DELAY) == pdTRUE)
      {
        if (xQueueReceive(xQueueSensor, &data, 0) == pdPASS)
        {
          int delayTime;
          if (glob_temperature < 25)
            delayTime = 1000;
          else if (glob_temperature < 35)
            delayTime = 500;
          else
            delayTime = 200;

          digitalWrite(LED_GPIO, HIGH);
          vTaskDelay(pdMS_TO_TICKS(delayTime));
          digitalWrite(LED_GPIO, LOW);
          vTaskDelay(pdMS_TO_TICKS(delayTime));
          Serial.printf("[LED] Blinked @T=%.1f°C\n", data.temperature);
        }
      }
    }
  }
}
