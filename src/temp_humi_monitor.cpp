#include "temp_humi_monitor.h"
#include <Arduino.h>

DHT20 dht20;
// LiquidCrystal_I2C lcd(33, 16, 2);
// LiquidCrystal_I2C lcd(0x21, 16, 2);

void temp_humi_monitor(void *pvParameters)
{
    // Wire.begin(11, 12);
    Serial.begin(115200);
    dht20.begin();
    // SensorData data;
    // lcd.begin();
    // lcd.backlight();
    // lcd.clear();
    // lcd.print("LCD Ready");
    // Serial.println("[LCD] Init done");
    while (1)
    {
        dht20.read();
        // Reading temperature in Celsius
        float temperature = dht20.getTemperature();
        // Reading humidity
        float humidity = dht20.getHumidity();

        // Check if any reads failed and exit early
        if (isnan(temperature) || isnan(humidity))
        {
            Serial.println("Failed to read from DHT sensor!");
            temperature = humidity = -1;
            // return;
        }
        data.humidity = humidity;
        data.temperature = temperature;

        xQueueSend(xQueueSensor, &data, portMAX_DELAY);

        xSemaphoreGive(xSemaphoreLed);
        xSemaphoreGive(xSemaphoreNeoLed);
        xSemaphoreGive(xSemaphoreLCD);

        // Update global variables for temperature and humidity
        // glob_temperature = temperature;
        // glob_humidity = humidity;
        // String state;
        // if (data.temperature >= 40 || data.humidity <= 30)
        //     state = "CRITICAL";
        // else if (data.temperature >= 35 || data.humidity <= 40)
        //     state = "WARNING";
        // else
        //     state = "NORMAL";

        // lcd.clear();
        // lcd.setCursor(0, 0);
        // lcd.print("State: " + state);
        // lcd.setCursor(0, 1);

        // char buffer[32];
        // snprintf(buffer, sizeof(buffer), "T:%.1f H:%.1f",
        //          data.temperature, data.humidity);
        // lcd.print(buffer);

        // Serial.printf("[LCD] %s | T=%.1f H=%.1f\n",
        //               state.c_str(), data.temperature, data.humidity);
        // // Print the results
        Serial.printf("[Sensor] Sent to Queue: T=%.1f H=%.1f\n", data.temperature, data.humidity);

        // Serial.print("Humidity: ");
        // Serial.print(humidity);
        // Serial.print("%  Temperature: ");
        // Serial.print(temperature);
        // Serial.println("°C");

        vTaskDelay(2000);
    }
}