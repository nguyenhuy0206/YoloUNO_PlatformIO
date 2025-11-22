#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

typedef struct
{
    float temperature;
    float humidity;
} SensorData;
// Config wifi
extern String ssid;
extern String password;
extern String wifi_ssid;
extern String wifi_password;
typedef struct
{
    SensorData data;
    boolean isWifiConnected;
    SemaphoreHandle_t xBinarySemaphoreInternet;
    SemaphoreHandle_t xSensorMutex;
    QueueHandle_t xQueueSensor;
    SemaphoreHandle_t xSemaphoreLed;
    SemaphoreHandle_t xSemaphoreNeoLed;
    SemaphoreHandle_t xSemaphoreLCD;
    SemaphoreHandle_t xSemaphoreTinyML;

<<<<<<< Updated upstream
extern SemaphoreHandle_t xBinarySemaphoreInternet;
=======
    bool led1_state;
    bool led2_state;
    bool pump_state;
    bool fan_state;
    bool isAPMode;
    bool isConnecting;
    uint32_t wifiConnectStartMs;
} AppContext;

>>>>>>> Stashed changes
#endif