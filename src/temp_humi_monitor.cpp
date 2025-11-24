// #include "temp_humi_monitor.h"
// #include <Arduino.h>

// DHT20 dht20;
// // LiquidCrystal_I2C lcd(33, 16, 2);
// // LiquidCrystal_I2C lcd(0x21, 16, 2);

// // void temp_humi_monitor(void *pvParameters)
// // {
// //     // Wire.begin(11, 12);
// //     Serial.begin(115200);
// //     dht20.begin();
// //     while (1)
// //     {
// //         dht20.read();
// //         // Reading temperature in Celsius
// //         float temperature = dht20.getTemperature();
// //         // Reading humidity
// //         float humidity = dht20.getHumidity();

// //         // Check if any reads failed and exit early
// //         if (isnan(temperature) || isnan(humidity))
// //         {
// //             Serial.println("Failed to read from DHT sensor!");
// //             temperature = humidity = -1;
// //             // return;
// //         }

// //         data.temperature = temperature;
// //         data.humidity = humidity;

// //         xSemaphoreGive(xSensorMutex);
// //         xQueueSend(xQueueSensor, &data, portMAX_DELAY);
// //         xSemaphoreGive(xSemaphoreLCD);
// //         xSemaphoreGive(xSemaphoreLed);
// //         xSemaphoreGive(xSemaphoreNeoLed);

// //         Serial.printf("[Sensor] Sent to Queue: T=%.1f H=%.1f\n", data.temperature, data.humidity);

// //         vTaskDelay(1000);
// //     }
// // }

// void temp_humi_monitor(void *pvParameters)
// {
//     AppContext *ctx = (AppContext *)pvParameters;

//     DHT20 dht20;
//     dht20.begin();

//     SensorData localData;

//     while (1)
//     {
//         dht20.read();
//         float temperature = dht20.getTemperature();
//         float humidity = dht20.getHumidity();

//         if (isnan(temperature) || isnan(humidity))
//         {
//             Serial.println("Failed to read from DHT sensor!");
//             temperature = humidity = -1;
//         }

//         localData.temperature = temperature;
//         localData.humidity = humidity;

//         xQueueSend(ctx->xQueueSensor, &localData, portMAX_DELAY);

//         xSemaphoreGive(ctx->xSemaphoreLCD);
//         xSemaphoreGive(ctx->xSemaphoreLed);
//         xSemaphoreGive(ctx->xSemaphoreNeoLed);

//         Serial.printf("[Sensor] Sent: T=%.1f H=%.1f\n",
//                       localData.temperature, localData.humidity);

//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }
// }
// temp_humi_monitor.cpp

#include "temp_humi_monitor.h"
#include "global.h"
#include <DHT20.h>

DHT20 dht20;

void temp_humi_monitor(void *pvParameters)
{
    AppContext *ctx = (AppContext *)pvParameters;

    dht20.begin();
    Serial.println("[Sensor] Task started");

    while (1)
    {
        dht20.read();
        float temperature = dht20.getTemperature();
        float humidity = dht20.getHumidity();

        if (isnan(temperature) || isnan(humidity))
        {
            Serial.println("[Sensor] Failed to read from DHT sensor!");
            temperature = humidity = -1;
        }

        // Ghi vào ctx->data (không bắt buộc, chỉ nếu muốn dùng)
        ctx->data.temperature = temperature;
        ctx->data.humidity = humidity;

        // Gửi qua queue (nên dùng local để tránh lẫn)
        SensorData packet;
        packet.temperature = temperature;
        packet.humidity = humidity;

        xQueueOverwrite(ctx->xQueueSensor, &packet);

        // Đánh thức các task khác
        xSemaphoreGive(ctx->xSemaphoreLCD);
        xSemaphoreGive(ctx->xSemaphoreLed);
        xSemaphoreGive(ctx->xSemaphoreNeoLed);

        Serial.printf("[Sensor] Sent: T=%.1f H=%.1f\n", temperature, humidity);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
