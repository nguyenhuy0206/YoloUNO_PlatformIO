#include <Arduino.h>
#include "global.h"
#include "led_blinky.h"

void led_blinky(void *pvParameters)
{
  pinMode(LED_GPIO, OUTPUT);
  SensorData data;

  while (1)
  {

    if (xQueueReceive(xQueueSensor, &data, 0) == pdPASS)
    {
      int delayTime;

      if (data.temperature < 25)
        delayTime = 1000;
      else if (data.temperature < 35)
        delayTime = 500;
      else
        delayTime = 200;

      digitalWrite(LED_GPIO, HIGH);
      vTaskDelay(pdMS_TO_TICKS(delayTime));
      digitalWrite(LED_GPIO, LOW);
      vTaskDelay(pdMS_TO_TICKS(delayTime));

      Serial2.printf("[LED] Blinked @T=%.1f°C\n", data.temperature);
    }
  }
}
