// #include <Arduino.h>
// #include "global.h"
// #include "led_blinky.h"

// void led_blinky(void *pvParameters)
// {
//   AppContext *ctx = (AppContext *)pvParameters;

//   pinMode(LED_GPIO, OUTPUT);
//   SensorData recv;

//   while (1)
//   {
//     // Đợi signal từ sensor
//     if (xSemaphoreTake(ctx->xSemaphoreLed, portMAX_DELAY) == pdTRUE)
//     {
//       if (xQueuePeek(ctx->xQueueSensor, &recv, 0) == pdTRUE)
//       {
//         int delayTime;
//         if (recv.temperature < 25)
//           delayTime = 1000;
//         else if (recv.temperature < 35)
//           delayTime = 500;
//         else
//           delayTime = 200;

//         digitalWrite(LED_GPIO, HIGH);
//         vTaskDelay(pdMS_TO_TICKS(delayTime));
//         digitalWrite(LED_GPIO, LOW);
//         vTaskDelay(pdMS_TO_TICKS(delayTime));

//         Serial.printf("[LED] Blinked @T=%.1f°C\n", recv.temperature);
//       }
//     }
//   }
// }

// void setFanSpeed(int percent)
// {
//   // 0-100%
//   percent = constrain(percent, 0, 100);
//   int duty = map(percent, 0, 100, 0, 1023);
//   ledcWrite(0, duty);
// }

// led_blinky.cpp

#include "led_blinky.h"
#include "global.h"

void led_blinky(void *pvParameters)
{
  AppContext *ctx = (AppContext *)pvParameters;

  pinMode(LED_GPIO, OUTPUT);
  SensorData recv;

  while (1)
  {
    if (xSemaphoreTake(ctx->xSemaphoreLed, portMAX_DELAY) == pdTRUE)
    {

      if (xQueuePeek(ctx->xQueueSensor, &recv, 0) == pdTRUE)
      {
        int delayTime;
        if (recv.temperature < 25)
          delayTime = 1000;
        else if (recv.temperature < 35)
          delayTime = 500;
        else
          delayTime = 200;

        Serial.printf("[LED] Blink with delay %d ms @T=%.1f°C\n",
                      delayTime, recv.temperature);

        digitalWrite(LED_GPIO, HIGH);
        vTaskDelay(pdMS_TO_TICKS(delayTime));
        digitalWrite(LED_GPIO, LOW);
        vTaskDelay(pdMS_TO_TICKS(delayTime));
      }
      else
      {
        Serial.println("[LED] Queue empty");
      }
    }
  }
}
