#include "global.h"
#include <Arduino.h>

#include "led_blinky.h"
#include "neo_blinky.h"
#include "temp_humi_monitor.h"
#include "mainserver.h"
#include "tinyml.h"
#include "coreiot.h"
#include "lcd_display.h"
#include "control.h"
void setup()
{
  Serial.begin(115200);
  Wire.begin(11, 12);

  xBinarySemaphoreInternet = xSemaphoreCreateBinary();

  static AppContext app;

  app.xQueueSensor = xQueueCreate(1, sizeof(SensorData));
  app.xSemaphoreLCD = xSemaphoreCreateBinary();
  app.xSemaphoreLed = xSemaphoreCreateBinary();
  app.xSemaphoreNeoLed = xSemaphoreCreateBinary();

  xTaskCreate(temp_humi_monitor, "Sensor", 4096, &app, 2, NULL);
  xTaskCreate(led_blinky, "LED", 4096, &app, 1, NULL);
  xTaskCreate(neo_blinky, "Neo", 4096, &app, 1, NULL);
  xTaskCreate(lcd_display, "LCD", 4096, &app, 1, NULL);
  xTaskCreate(main_server_task, "MainServer", 8192, &app, 1, NULL);
  xTaskCreate(coreiot_task, "CoreIOT", 8192, &app, 1, NULL);
  xTaskCreate(tiny_ml_task, "TinyML", 8192, &app, 1, NULL);
}

void loop()
{
}
