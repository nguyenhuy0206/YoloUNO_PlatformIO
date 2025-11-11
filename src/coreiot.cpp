#include "coreiot.h"

// ----------- CONFIGURE THESE! -----------
const char *coreIOT_Server = "app.coreiot.io";
const char *coreIOT_Token = "oEFW8VVF96o8qkNI0ylb"; // Device Access Token
const int mqttPort = 1883;
// ----------------------------------------

WiFiClient espClient;
PubSubClient client(espClient);

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect (username=token, password=empty)
    if (client.connect("ESP32Client", coreIOT_Token, NULL))
    {
      Serial.println("connected to CoreIOT Server!");
      client.subscribe("v1/devices/me/rpc/request/+");
      Serial.println("Subscribed to v1/devices/me/rpc/request/+");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");

  // Allocate a temporary buffer for the message
  char message[length + 1];
  memcpy(message, payload, length);
  message[length] = '\0';
  Serial.print("Payload: ");
  Serial.println(message);

  // Parse JSON
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  const char *method = doc["method"];
  if (strcmp(method, "setStateLED") == 0)
  {
    // Check params type (could be boolean, int, or string according to your RPC)
    // Example: {"method": "setValueLED", "params": "ON"}
    const char *params = doc["params"];

    if (strcmp(params, "ON") == 0)
    {
      Serial.println("Device turned ON.");
      // TODO
    }
    else
    {
      Serial.println("Device turned OFF.");
      // TODO
    }
  }
  else
  {
    Serial.print("Unknown method: ");
    Serial.println(method);
  }
}

void setup_coreiot()
{

  // Serial.print("Connecting to WiFi...");
  // WiFi.begin(wifi_ssid, wifi_password);
  // while (WiFi.status() != WL_CONNECTED) {

  // while (isWifiConnected == false) {
  //   delay(500);
  //   Serial.print(".");
  // }

  while (1)
  {
    if (xSemaphoreTake(xBinarySemaphoreInternet, portMAX_DELAY))
    {
      break;
    }
    delay(500);
    Serial.print(".");
  }

  Serial.println(" Connected!");

  client.setServer(coreIOT_Server, mqttPort);
  client.setCallback(callback);
}
void wifi_connect_task(void *pvParameters)
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    vTaskDelay(pdMS_TO_TICKS(500));
  }

  Serial.println("\nWiFi Connected, IP: " + WiFi.localIP().toString());

  // Signal các task khác WiFi ready
  xSemaphoreGive(xBinarySemaphoreInternet);

  // Task xong → tự xóa
  vTaskDelete(NULL);
}

void coreiot_task(void *pvParameters)
{
  // Chờ WiFi
  xSemaphoreTake(xBinarySemaphoreInternet, portMAX_DELAY);

  String clientId = "ESP32Client-" + String(random(0xffff), HEX);
  if (!client.connect(clientId.c_str(), coreIOT_Token, NULL))
  {
    Serial.println("MQTT connect failed");
    vTaskDelay(pdMS_TO_TICKS(5000));
  }

  client.setServer(coreIOT_Server, mqttPort);
  client.setCallback(callback);

  SensorData recv;
  StaticJsonDocument<128> doc;
  char buffer[128];

  while (1)
  {
    if (!client.connected())
      reconnect();

    client.loop();

    if (xQueueReceive(xQueueSensor, &recv, 0) == pdPASS)
    {
      doc.clear();
      doc["temperature"] = recv.temperature;
      doc["humidity"] = recv.humidity;
      serializeJson(doc, buffer);

      if (client.publish("v1/devices/me/telemetry", buffer))
        Serial.println("Published: " + String(buffer));
      else
        Serial.println("Failed to publish");
    }

    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}
