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

  xQueueSensor = xQueueCreate(5, sizeof(SensorData));
  xSensorMutex = xSemaphoreCreateMutex();

  xBinarySemaphoreInternet = xSemaphoreCreateBinary();
  xSemaphoreLed = xSemaphoreCreateBinary();
  xSemaphoreNeoLed = xSemaphoreCreateBinary();
  xSemaphoreLCD = xSemaphoreCreateBinary();

  // Tạo task LCD trước
  xTaskCreate(wifi_connect_task, "WiFi Connect", 4096, NULL, 1, NULL);

  // Tạo task sensor sau
  xTaskCreate(temp_humi_monitor, "Sensor", 4096, NULL, 1, NULL);

  // LED task
  // xTaskCreate(led_blinky, "LED", 4096, NULL, 1, NULL);
  // xTaskCreate(neo_blinky, "NeoPixel", 4096, NULL, 1, NULL);
  // xTaskCreate(lcd_display, "LCD", 4096, NULL, 1, NULL);
  // xTaskCreate(task_fan, "Fan", 4096, NULL, 1, NULL);
  // xTaskCreate(task_fan, "Fan", 4096, NULL, 1, NULL);
  // xTaskCreate(task_pump, "Pump", 4096, NULL, 1, NULL);
  // xTaskCreate(main_server_task, "MainServer", 8192, NULL, 1, NULL);
  xTaskCreate(coreiot_task, "CoreIOT", 8192, NULL, 1, NULL);
  xTaskCreate(tiny_ml_task, "TinyML", 8192, NULL, 1, NULL);
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

// #include <WiFi.h>
// #include <PubSubClient.h>
// #include <ArduinoJson.h>

// // const char *ssid = "ACLAB";
// // const char *pass = "ACLAB2023";

// const char *coreIOT_Server = "app.coreiot.io";
// const char *coreIOT_Token = "oghoslDWPIEMWW0gg5PT";
// const int mqttPort = 1883;

// #define PUMP_PIN 8
// WiFiClient espClient;
// PubSubClient client(espClient);

// bool pumpState = false;

// void callback(char *topic, byte *payload, unsigned int length)
// {
//   Serial.print("RPC arrived [");
//   Serial.print(topic);
//   Serial.println("] ");

//   char msg[length + 1];
//   memcpy(msg, payload, length);
//   msg[length] = '\0';

//   Serial.print("Payload: ");
//   Serial.println(msg);

//   StaticJsonDocument<256> doc;
//   if (deserializeJson(doc, msg))
//   {
//     Serial.println("JSON parse error");
//     return;
//   }

//   String method = doc["method"];
//   String requestID = String(topic).substring(String(topic).lastIndexOf('/') + 1);

//   // ------------------ GET STATE ------------------
//   if (method == "getState")
//   {
//     StaticJsonDocument<64> resp;
//     resp["state"] = pumpState;

//     char buf[64];
//     serializeJson(resp, buf);

//     client.publish(("v1/devices/me/rpc/response/" + requestID).c_str(), buf);
//     Serial.println("Responded getState()");
//     return;
//   }

//   // ------------------ SET STATE -------------------
//   if (method == "setState")
//   {
//     pumpState = doc["params"];
//     digitalWrite(PUMP_PIN, pumpState);

//     client.publish(("v1/devices/me/rpc/response/" + requestID).c_str(), "{}");
//     Serial.printf("Pump → %s\n", pumpState ? "ON" : "OFF");
//     return;
//   }
// }

// void reconnect()
// {
//   while (!client.connected())
//   {
//     if (client.connect("ESP32Client", coreIOT_Token, NULL))
//     {
//       Serial.println("Connected to CoreIOT!");
//       client.subscribe("v1/devices/me/rpc/request/+");
//     }
//     else
//     {
//       Serial.println("Retry in 3 seconds…");
//       delay(3000);
//     }
//   }
// }

// void setup()
// {
//   Serial.begin(115200);
//   pinMode(PUMP_PIN, OUTPUT);

//   WiFi.begin(wifi_ssid, wifi_password);
//   Serial.print("Connecting");
//   while (WiFi.status() != WL_CONNECTED)
//   {
//     Serial.print(".");
//     delay(500);
//   }
//   Serial.println(" WiFi OK");

//   client.setServer(coreIOT_Server, mqttPort);
//   client.setCallback(callback);
// }

// void loop()
// {
//   if (!client.connected())
//     reconnect();

//   client.loop();
// }
