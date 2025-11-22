#include "temp_humi_monitor.h"
#include <Arduino.h>

DHT20 dht20;

void temp_humi_monitor(void *pvParameters)
{
    Serial.begin(115200);
    dht20.begin();
    AppContext *ctx = (AppContext *)pvParameters;
    SensorData recv;
    while (1)
    {
        dht20.read();
        float temperature = dht20.getTemperature();
        float humidity = dht20.getHumidity();

        if (isnan(temperature) || isnan(humidity))
        {
            Serial.println("Failed to read from DHT sensor!");
            temperature = humidity = -1;
        }

        recv.temperature = temperature;
        recv.humidity = humidity;

        xQueueOverwrite(ctx->xQueueSensor, &recv);
        xSemaphoreGive(ctx->xSemaphoreLCD);
        xSemaphoreGive(ctx->xSemaphoreLed);
        xSemaphoreGive(ctx->xSemaphoreNeoLed);
        xSemaphoreGive(ctx->xSemaphoreTinyML);
        Serial.printf("[Sensor] Sent to Queue: T=%.1f H=%.1f\n", recv.temperature, recv.humidity);

        vTaskDelay(1000);
    }
}