#include "global.h"
#include <Arduino.h>

#include "led_blinky.h"
#include "neo_blinky.h"
#include "temp_humi_monitor.h"
#include "mainserver.h"
#include "tinyml.h"
#include "coreiot.h"

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
  // xTaskCreate(neo_blinky, "Neo", 2048, NULL, 1, NULL);
  xTaskCreate(temp_humi_monitor, "Sensor", 4096, NULL, 1, NULL);
}

void loop()
{
}