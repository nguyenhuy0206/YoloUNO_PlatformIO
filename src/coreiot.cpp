#include "coreiot.h"
#include "global.h"
#include "control.h"

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ----------- CONFIGURE THESE! -----------
const char *coreIOT_Server = "app.coreiot.io";
const char *coreIOT_Token = "oghoslDWPIEMWW0gg5PT"; // Device Access Token
const int mqttPort = 1883;
// ----------------------------------------

WiFiClient espClient;
PubSubClient client(espClient);

// Con trỏ giữ AppContext cho callback MQTT
static AppContext *g_ctx = nullptr;

// Forward declare
static void reconnect();
static void mqttCallback(char *topic, byte *payload, unsigned int length);
static void setup_coreiot(AppContext *ctx);

//==================================================
//  MQTT reconnect
//==================================================

static void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect (username = token, password = empty)
    if (client.connect("ESP32Client", coreIOT_Token, nullptr))
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
<<<<<<< Updated upstream
bool pumpState = false;
bool fanState = false;
void callback(char *topic, byte *payload, unsigned int length)
=======

//==================================================
//  MQTT callback (RPC from CoreIoT)
//==================================================

static void mqttCallback(char *topic, byte *payload, unsigned int length)
>>>>>>> Stashed changes
{
  AppContext *ctx = g_ctx;
  if (ctx == nullptr)
  {
    Serial.println("MQTT callback: ctx is null");
    return;
  }

  Serial.print("RPC arrived [");
  Serial.print(topic);
  Serial.println("] ");

  // Copy payload vào buffer C-string
  char msg[256];
  if (length >= sizeof(msg))
    length = sizeof(msg) - 1;
  memcpy(msg, payload, length);
  msg[length] = '\0';

  Serial.print("Payload: ");
  Serial.println(msg);

  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, msg);
  if (err)
  {
    Serial.print("JSON parse error: ");
    Serial.println(err.c_str());
    return;
  }

  String method = doc["method"] | "";
  String requestID = String(topic).substring(String(topic).lastIndexOf('/') + 1);

  // ------------------ GET STATE ------------------
  if (method == "getPumpState")
  {
    StaticJsonDocument<64> resp;
<<<<<<< Updated upstream
    resp["state"] = pumpState;
=======
    resp["state"] = ctx->pump_state;
>>>>>>> Stashed changes

    char buf[64];
    serializeJson(resp, buf);

    client.publish(("v1/devices/me/rpc/response/" + requestID).c_str(), buf);
    Serial.println("Responded getPumpState()");
    return;
  }

  if (method == "getFanState")
  {
    StaticJsonDocument<64> resp;
<<<<<<< Updated upstream
    resp["state"] = fanState;
=======
    resp["state"] = ctx->fan_state;
>>>>>>> Stashed changes

    char buf[64];
    serializeJson(resp, buf);

    client.publish(("v1/devices/me/rpc/response/" + requestID).c_str(), buf);
    Serial.println("Responded getFanState()");
    return;
  }

  // ------------------ SET STATE -------------------
  if (method == "setPumpState")
  {
    pumpState = doc["params"];
    digitalWrite(PUMP_PIN, pumpState);

<<<<<<< Updated upstream
    client.publish(("v1/devices/me/rpc/response/" + requestID).c_str(), "{}");
    Serial.printf("Pump → %s\n", pumpState ? "ON" : "OFF");
=======
    ctx->pump_state = new_state;
    digitalWrite(PUMP_PIN, ctx->pump_state ? HIGH : LOW);

    reportStateToCoreIOT("pump_state", ctx->pump_state);
    client.publish(("v1/devices/me/rpc/response/" + requestID).c_str(), "{}");
    Serial.printf("Pump → %s\n", ctx->pump_state ? "ON" : "OFF");
>>>>>>> Stashed changes
    return;
  }

  if (method == "setFanState")
  {
<<<<<<< Updated upstream
    fanState = doc["params"];
    digitalWrite(FAN_PIN, fanState);

    client.publish(("v1/devices/me/rpc/response/" + requestID).c_str(), "{}");
    Serial.printf("Fan → %s\n", fanState ? "ON" : "OFF");
    return;
  }
}

void setup_coreiot()
=======
    bool new_state = doc["params"];

    ctx->fan_state = new_state;
    digitalWrite(FAN_PIN, ctx->fan_state ? HIGH : LOW);

    reportStateToCoreIOT("fan_state", ctx->fan_state);
    client.publish(("v1/devices/me/rpc/response/" + requestID).c_str(), "{}");
    Serial.printf("Fan → %s\n", ctx->fan_state ? "ON" : "OFF");
    return;
  }
}

//==================================================
//  Gửi telemetry 1 key bool lên CoreIoT
//==================================================

void reportStateToCoreIOT(const char *key, bool state)
{
  if (!client.connected())
    return;

  StaticJsonDocument<64> doc;
  doc[key] = state; // Ví dụ: {"pump_state": true}

  char buffer[64];
  serializeJson(doc, buffer);
  client.publish("v1/devices/me/telemetry", buffer);
  Serial.printf("[CoreIOT] Telemetry: %s -> %s\n", key, state ? "ON" : "OFF");
}

//==================================================
//  Setup MQTT (không đụng tới WiFi.begin nữa)
//==================================================

static void setup_coreiot(AppContext *ctx)
>>>>>>> Stashed changes
{
  (void)ctx; // hiện chưa dùng thêm thông tin trong ctx

  pinMode(PUMP_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
<<<<<<< Updated upstream
  WiFi.begin(wifi_ssid, wifi_password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println(" WiFi OK");
=======
>>>>>>> Stashed changes

  // MQTT client
  client.setServer(coreIOT_Server, mqttPort);
  client.setCallback(mqttCallback);
}

<<<<<<< Updated upstream
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

=======
//==================================================
//  coreiot_task – task FreeRTOS
//==================================================

void coreiot_task(void *pvParameters)
{
  AppContext *ctx = (AppContext *)pvParameters;
  g_ctx = ctx;

  Serial.println("CoreIOT: Waiting for Internet connection...");

  // Chờ main_server_task báo Internet ready qua semaphore
  if (ctx->xBinarySemaphoreInternet != nullptr)
  {
    xSemaphoreTake(ctx->xBinarySemaphoreInternet, portMAX_DELAY);
  }

  Serial.println("CoreIOT: Internet connected! Starting MQTT loop...");
  setup_coreiot(ctx);

>>>>>>> Stashed changes
  SensorData recv;
  StaticJsonDocument<128> doc;
  char buffer[128];

  for (;;)
  {
    if (!client.connected())
      reconnect();

    client.loop();

    // Lấy sensor data mới nhất: ưu tiên queue
    bool haveData = false;

    if (ctx->xQueueSensor != nullptr &&
        xQueuePeek(ctx->xQueueSensor, &recv, 0) == pdTRUE)
    {
      haveData = true;
    }
    else if (ctx->xSensorMutex != nullptr &&
             xSemaphoreTake(ctx->xSensorMutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
      // Fallback nếu bạn vẫn cập nhật ctx->data ở task sensor
      recv = ctx->data;
      xSemaphoreGive(ctx->xSensorMutex);
      haveData = true;
    }

    if (haveData)
    {
      doc.clear();
      doc["temperature"] = recv.temperature;
      doc["humidity"] = recv.humidity;

      serializeJson(doc, buffer);
      Serial.printf("[CoreIOT] T=%.1f H=%.1f -> %s\n",
                    recv.temperature, recv.humidity, buffer);

      if (client.publish("v1/devices/me/telemetry", buffer))
        Serial.println("Published telemetry OK");
      else
        Serial.println("Failed to publish telemetry");
    }

    vTaskDelay(pdMS_TO_TICKS(1000)); // 1s publish một lần
  }
}
