#include "mainserver.h"
#include "global.h"

#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>

#include "led_blinky.h"
#include "neo_blinky.h"
#include "control.h"
#include "coreiot.h"

// ================== GLOBALS ==================

static Adafruit_NeoPixel strip(LED_COUNT, NEO_PIN, NEO_GRB + NEO_KHZ800);
static WebServer server(80);

// ================== Forward declarations ==================

static void handleState(AppContext *ctx);
static void handleSet(AppContext *ctx);
static void handleSetAll(AppContext *ctx);
static void handleNeopixel(AppContext *ctx);
static void handleToggle(AppContext *ctx);
static void handleSensors(AppContext *ctx);
static void handleWifiStatus(AppContext *ctx);
static void handleConnect(AppContext *ctx);

static void startAP(AppContext *ctx);
static void connectToWiFi(AppContext *ctx);
static void setupServer(AppContext *ctx);

static String getContentType(const String &filename);
static bool serveFile(const String &path);
static void handleRootPage(AppContext *ctx);

// ================== HANDLERS ==================

static void handleState(AppContext *ctx)
{
  StaticJsonDocument<128> doc;
  doc["led1"] = ctx->led1_state ? "ON" : "OFF";
  doc["led2"] = ctx->led2_state ? "ON" : "OFF";
  doc["fan"] = ctx->fan_state ? "ON" : "OFF";
  doc["pump"] = ctx->pump_state ? "ON" : "OFF";

  char buffer[128];
  serializeJson(doc, buffer);
  server.send(200, "application/json", buffer);
}

static void handleSet(AppContext *ctx)
{
  int led = server.arg("led").toInt();
  String state = server.arg("state"); // "on" hoặc "off"
  state.toLowerCase();

  bool value = (state == "on");

  if (led == 1)
  {
    ctx->led1_state = value;
    digitalWrite(LED1_PIN, value ? HIGH : LOW);
    Serial.print("LED1 -> ");
    Serial.println(value ? "ON" : "OFF");
  }
  else if (led == 2)
  {
    ctx->led2_state = value;
    digitalWrite(LED2_PIN, value ? HIGH : LOW);
    Serial.print("LED2 -> ");
    Serial.println(value ? "ON" : "OFF");
  }
  else if (ledStr == "pump")
  {
    ctx->pump_state = value;
    digitalWrite(PUMP_PIN, value ? HIGH : LOW);
    Serial.print("PUMP -> ");
    Serial.println(value ? "ON" : "OFF");
  }
  else if (ledStr == "fan")
  {
    ctx->fan_state = value;
    digitalWrite(FAN_PIN, value ? HIGH : LOW);
    Serial.print("FAN -> ");
    Serial.println(value ? "ON" : "OFF");
  }

  String json = "{\"led1\":\"" + String(ctx->led1_state ? "ON" : "OFF") +
                "\",\"led2\":\"" + String(ctx->led2_state ? "ON" : "OFF") +
                "\",\"pump\":\"" + String(ctx->pump_state ? "ON" : "OFF") +
                "\",\"fan\":\"" + String(ctx->fan_state ? "ON" : "OFF") + "\"}";
  server.send(200, "application/json", json);
}

static void handleSetAll(AppContext *ctx)
{
  String state = server.arg("state"); // "on" hoặc "off"
  state.toLowerCase();
  bool value = (state == "on");

  ctx->led1_state = value;
  ctx->led2_state = value;
  ctx->fan_state = value;
  ctx->pump_state = value;

  digitalWrite(LED1_PIN, value ? HIGH : LOW);
  digitalWrite(LED2_PIN, value ? HIGH : LOW);
  digitalWrite(FAN_PIN, value ? HIGH : LOW);
  digitalWrite(PUMP_PIN, value ? HIGH : LOW);

  if (!value)
  {
    strip.clear();
    strip.show();
  }

  Serial.print("ALL DEVICES -> ");
  Serial.println(value ? "ON" : "OFF");
  // TODO: set luôn GPIO thực tế nếu cần

  String json = "{\"led1\":\"" + String(ctx->led1_state ? "ON" : "OFF") +
                "\",\"led2\":\"" + String(ctx->led2_state ? "ON" : "OFF") +
                "\",\"pump\":\"" + String(ctx->pump_state ? "ON" : "OFF") +
                "\",\"fan\":\"" + String(ctx->fan_state ? "ON" : "OFF") + "\"}";
  server.send(200, "application/json", json);
}

static uint32_t hexToUint32(const String &hexStr)
{
  String hex = hexStr;
  if (hex.startsWith("#"))
  {
    hex.remove(0, 1);
  }
  long number = (long)strtol(hex.c_str(), NULL, 16);

  uint8_t r = (number >> 16) & 0xFF;
  uint8_t g = (number >> 8) & 0xFF;
  uint8_t b = number & 0xFF;

  return strip.Color(r, g, b);
}

static void handleNeopixel(AppContext *ctx)
{
  (void)ctx;

  String hex = server.arg("hex"); // dạng "#RRGGBB"
  Serial.print("NEOPIXEL color: ");
  Serial.println(hex);

  if (hex.length() == 7 && hex.startsWith("#"))
  {
    uint32_t color = hexToUint32(hex);
    strip.setPixelColor(0, color);
    strip.show();
  }

  server.send(200, "text/plain", "OK");
}

static void handleToggle(AppContext *ctx)
{
  int led = server.arg("led").toInt();
  if (led == 1)
  {
    ctx->led1_state = !ctx->led1_state;
    digitalWrite(LED1_PIN, ctx->led1_state ? HIGH : LOW);
    Serial.println("TOGGLE LED1");
  }
  else if (led == 2)
  {
    ctx->led2_state = !ctx->led2_state;
    digitalWrite(LED2_PIN, ctx->led2_state ? HIGH : LOW);
    Serial.println("TOGGLE LED2");
  }

  server.send(200, "application/json",
              "{\"led1\":\"" + String(ctx->led1_state ? "ON" : "OFF") +
                  "\",\"led2\":\"" + String(ctx->led2_state ? "ON" : "OFF") + "\"}");
}

static void handleSensors(AppContext *ctx)
{
  SensorData recv;
  if (xQueuePeek(ctx->xQueueSensor, &recv, 0) == pdTRUE)
  {
    String json = "{\"temp\":" + String(recv.temperature, 1) +
                  ",\"hum\":" + String(recv.humidity, 1) + "}";
    server.send(200, "application/json", json);
  }
  else
  {
    String json = "{\"temp\":0,\"hum\":0}";
    server.send(200, "application/json", json);
  }
}

static void handleWifiStatus(AppContext *ctx)
{
  wl_status_t st = WiFi.status();

  // Đã connect OK
  if (ctx->isWifiConnected && st == WL_CONNECTED)
  {
    server.send(200, "text/plain", "connected");
  }
  // Đang trong giai đoạn thử connect
  else if (ctx->isConnecting)
  {
    server.send(200, "text/plain", "connecting");
  }
  // Còn lại: failed / chưa connect
  else
  {
    server.send(200, "text/plain", "failed");
  }
}

// ================== WiFi connect logic ==================

static void connectToWiFi(AppContext *ctx)
{
  // Xoá cấu hình cũ để tránh tự reconnect vào AP/SSID khác
  WiFi.disconnect(true, true);
  delay(200);

  Serial.print("[WiFi] Starting STA connect to: ");
  Serial.print(wifi_ssid);
  Serial.print(" Password: ");
  Serial.println(wifi_password);

  WiFi.mode(WIFI_AP_STA); // vừa AP vừa STA
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());

  ctx->wifiConnectStartMs = millis();
}

static void handleConnect(AppContext *ctx)
{
  // Debug toàn bộ arg để chắc chắn client gửi đúng
  Serial.println("[HTTP] /connect CALLED");
  Serial.print("  args count = ");
  Serial.println(server.args());
  for (int i = 0; i < server.args(); i++)
  {
    Serial.print("  ");
    Serial.print(server.argName(i));
    Serial.print(" = ");
    Serial.println(server.arg(i));
  }

  wifi_ssid = server.arg("ssid");
  wifi_password = server.arg("pass");

  Serial.print("[HTTP] Parsed SSID = ");
  Serial.print(wifi_ssid);
  Serial.print("  PASS = ");
  Serial.println(wifi_password);

  server.send(200, "text/plain", "Connecting....");

  // Chuyển trạng thái sang "đang connect"
  ctx->isAPMode = false; // tạm coi đang chuyển sang STA
  ctx->isWifiConnected = false;
  ctx->isConnecting = true;

  connectToWiFi(ctx);
}

// ================== Static files ==================

static String getContentType(const String &filename)
{
  if (filename.endsWith(".html"))
    return "text/html; charset=utf-8";
  if (filename.endsWith(".css"))
    return "text/css";
  if (filename.endsWith(".js"))
    return "application/javascript";
  if (filename.endsWith(".json"))
    return "application/json";
  if (filename.endsWith(".png"))
    return "image/png";
  if (filename.endsWith(".jpg") || filename.endsWith(".jpeg"))
    return "image/jpeg";
  if (filename.endsWith(".ico"))
    return "image/x-icon";
  return "text/plain";
}

static bool serveFile(const String &path)
{
  if (!SPIFFS.exists(path))
  {
    Serial.print("File not found: ");
    Serial.println(path);
    return false;
  }

  File file = SPIFFS.open(path, "r");
  String contentType = getContentType(path);
  server.streamFile(file, contentType);
  file.close();
  return true;
}

static void handleRootPage(AppContext *ctx)
{
  (void)ctx;

  if (WiFi.status() == WL_CONNECTED)
  {
    server.send(404, "text/plain", "index.html not found");
  }
}

// ================== AP helper ==================

static void startAP(AppContext *ctx)
{
  WiFi.mode(WIFI_AP_STA);
  // ssid / password là ssid AP mặc định (global.h / global.cpp)
  WiFi.softAP(ssid.c_str(), password.c_str());

  IPAddress ip = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(ip);

  ctx->isAPMode = true;
  ctx->isWifiConnected = false;
  ctx->isConnecting = false;
}

// ================== setupServer ==================

static void setupServer(AppContext *ctx)
{
  (void)ctx;

  server.enableCORS(true);

  // Trang chính
  server.on("/", HTTP_GET, [ctx]()
            { handleRootPage(ctx); });

  server.on("/dashboard.html", HTTP_GET, []()
            { serveFile("/dashboard.html"); });
  server.on("/control.html", HTTP_GET, []()
            { serveFile("/control.html"); });
  server.on("/cloud.html", HTTP_GET, []()
            { serveFile("/cloud.html"); });
  server.on("/system_log.html", HTTP_GET, []()
            { serveFile("/system_log.html"); });
  server.on("/wifi.html", HTTP_GET, []()
            { serveFile("/wifi.html"); });
  server.on("/login.html", HTTP_GET, []()
            { serveFile("/login.html"); });

  // Static assets
  server.on("/style.css", HTTP_GET, []()
            { serveFile("/style.css"); });
  server.on("/script.js", HTTP_GET, []()
            { serveFile("/script.js"); });
  server.on("/humidity.json", HTTP_GET, []()
            { serveFile("/humidity.json"); });
  server.on("/Thermometer.json", HTTP_GET, []()
            { serveFile("/Thermometer.json"); });

  // API
  server.on("/toggle", HTTP_GET, [ctx]()
            { handleToggle(ctx); });
  server.on("/set", HTTP_GET, [ctx]()
            { handleSet(ctx); });
  server.on("/set_all", HTTP_GET, [ctx]()
            { handleSetAll(ctx); });
  server.on("/neopixel", HTTP_GET, [ctx]()
            { handleNeopixel(ctx); });
  server.on("/sensors", HTTP_GET, [ctx]()
            { handleSensors(ctx); });
  server.on("/wifi_status", HTTP_GET, [ctx]()
            { handleWifiStatus(ctx); });
  server.on("/connect", HTTP_GET, [ctx]()
            { handleConnect(ctx); });
  server.on("/state", HTTP_GET, [ctx]()
            { handleState(ctx); });

  // 404 fallback
  server.onNotFound([]()
                    {
                      String uri = server.uri();
                      if (!uri.startsWith("/")) uri = "/" + uri;
                      if (!serveFile(uri))
                      {
                        server.send(404, "text/plain", "404: Not found");
                      } });

  server.begin();
  Serial.println("[HTTP] Server started");
}

// ================== MAIN SERVER TASK ==================

void main_server_task(void *pvParameters)
{
  AppContext *ctx = (AppContext *)pvParameters;

  pinMode(BOOT_PIN, INPUT_PULLUP);

  if (!SPIFFS.begin(true))
  {
    Serial.println("SPIFFS mount failed");
  }
  else
  {
    Serial.println("SPIFFS mounted OK");
  }

  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);

  strip.begin();
  strip.show();

  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(PUMP_PIN, LOW);

  // Khởi động trong AP mode + HTTP server
  startAP(ctx);
  setupServer(ctx); // CHỈ GỌI 1 LẦN

  for (;;)
  {
    server.handleClient();

    // Nút BOOT để force AP mode
    if (digitalRead(BOOT_PIN) == LOW)
    {
      vTaskDelay(pdMS_TO_TICKS(100));
      if (digitalRead(BOOT_PIN) == LOW)
      {
        Serial.println("[BUTTON] Force AP mode");
        startAP(ctx);
      }
    }

    // Xử lý trạng thái STA CONNECTING
    if (ctx->isConnecting)
    {
      wl_status_t st = WiFi.status();

      if (st == WL_CONNECTED)
      {
        Serial.print("[WiFi] STA IP address: ");
        Serial.println(WiFi.localIP());

        ctx->isWifiConnected = true;
        ctx->isConnecting = false;
        ctx->isAPMode = false;

        // Nếu muốn: tắt AP, chỉ giữ STA
        // WiFi.softAPdisconnect(true);
        // WiFi.mode(WIFI_STA);

        // Báo cho coreiot_task là đã có Internet
        if (ctx->xBinarySemaphoreInternet != NULL)
        {
          xSemaphoreGive(ctx->xBinarySemaphoreInternet);
        }
      }
      else
      {
        uint32_t elapsed = millis() - ctx->wifiConnectStartMs;
        // Timeout 20 giây
        if (elapsed > 20000)
        {
          Serial.println("[WiFi] Connect timeout! Back to AP.");

          ctx->isWifiConnected = false;
          ctx->isConnecting = false;
          ctx->isAPMode = true;

          // Quay về AP
          startAP(ctx);
        }
      }
    }

    vTaskDelay(1); // tránh WDT
  }
}
