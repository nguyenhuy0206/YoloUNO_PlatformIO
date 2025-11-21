#include "coreiot.h"
#include "control.h"
// ----------- CONFIGURE THESE! -----------
const char *coreIOT_Server = "app.coreiot.io";
const char *coreIOT_Token = "oghoslDWPIEMWW0gg5PT"; // Device Access Token
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
bool pumpState = false;
bool fanState = false;
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("RPC arrived [");
  Serial.print(topic);
  Serial.println("] ");

  char msg[length + 1];
  memcpy(msg, payload, length);
  msg[length] = '\0';

  Serial.print("Payload: ");
  Serial.println(msg);

  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, msg))
  {
    Serial.println("JSON parse error");
    return;
  }

  String method = doc["method"];
  String requestID = String(topic).substring(String(topic).lastIndexOf('/') + 1);

  // ------------------ GET STATE ------------------
  if (method == "getPumpState")
  {
    StaticJsonDocument<64> resp;
    resp["state"] = pumpState;

    char buf[64];
    serializeJson(resp, buf);

    client.publish(("v1/devices/me/rpc/response/" + requestID).c_str(), buf);
    Serial.println("Responded getState()");
    return;
  }
  if (method == "getFanState")
  {
    StaticJsonDocument<64> resp;
    resp["state"] = fanState;

    char buf[64];
    serializeJson(resp, buf);

    client.publish(("v1/devices/me/rpc/response/" + requestID).c_str(), buf);
    Serial.println("Responded getState()");
    return;
  }

  // ------------------ SET STATE -------------------
  if (method == "setPumpState")
  {
    pumpState = doc["params"];
    digitalWrite(PUMP_PIN, pumpState);

    client.publish(("v1/devices/me/rpc/response/" + requestID).c_str(), "{}");
    Serial.printf("Pump → %s\n", pumpState ? "ON" : "OFF");
    return;
  }
  if (method == "setFanState")
  {
    fanState = doc["params"];
    digitalWrite(FAN_PIN, fanState);

    client.publish(("v1/devices/me/rpc/response/" + requestID).c_str(), "{}");
    Serial.printf("Fan → %s\n", fanState ? "ON" : "OFF");
    return;
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
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  WiFi.begin(wifi_ssid, wifi_password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println(" WiFi OK");

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
  // xSemaphoreTake(xBinarySemaphoreInternet, portMAX_DELAY);
  // client.setServer(coreIOT_Server, mqttPort);
  // client.setCallback(callback);
  // String clientId = "ESP32Client-" + String(random(0xffff), HEX);
  // if (!client.connect(clientId.c_str(), coreIOT_Token, NULL))
  // {
  //   Serial.println("MQTT connect failed");
  //   vTaskDelay(pdMS_TO_TICKS(5000));
  // }

  SensorData recv;
  StaticJsonDocument<128> doc;
  char buffer[128];
  setup_coreiot();
  while (1)
  {
    if (!client.connected())
      reconnect();

    client.loop();

    if (xSemaphoreTake(xSensorMutex, portMAX_DELAY) == pdTRUE)
    {
      recv = data;
      xSemaphoreGive(xSensorMutex);
      doc.clear();
      doc["temperature"] = recv.temperature;
      doc["humidity"] = recv.humidity;
      Serial.printf("[CoreIOT] T=%.1f H=%.1f \n", recv.temperature, recv.humidity);
      serializeJson(doc, buffer);

      if (client.publish("v1/devices/me/telemetry", buffer))
        Serial.println("Published: " + String(buffer));
      else
        Serial.println("Failed to publish");
    }

    vTaskDelay(pdMS_TO_TICKS(10000));
  }
  // if (!client.connected())
  //   reconnect();

  // client.loop();
}
