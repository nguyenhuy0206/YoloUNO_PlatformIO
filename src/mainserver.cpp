#include "mainserver.h"
#include <WiFi.h>
#include <WebServer.h>

bool led1_state = false;
bool led2_state = false;
bool isAPMode = true;

WebServer server(80);

unsigned long connect_start_ms = 0;
bool connecting = false;

void handleSet()
{
  int led = server.arg("led").toInt();
  String state = server.arg("state"); // "on" hoặc "off"
  state.toLowerCase();

  bool value = (state == "on");

  if (led == 1)
  {
    led1_state = value;
    Serial.print("LED1 -> ");
    Serial.println(led1_state ? "ON" : "OFF");
    // TODO: thêm YOUR CODE TO CONTROL LED1 ở đây, ví dụ:
    // digitalWrite(LED1_PIN, led1_state ? HIGH : LOW);
  }
  else if (led == 2)
  {
    led2_state = value;
    Serial.print("LED2 -> ");
    Serial.println(led2_state ? "ON" : "OFF");
    // TODO: YOUR CODE TO CONTROL LED2
  }

  String json = "{\"led1\":\"" + String(led1_state ? "ON" : "OFF") +
                "\",\"led2\":\"" + String(led2_state ? "ON" : "OFF") + "\"}";
  server.send(200, "application/json", json);
}
void handleSetAll()
{
  String state = server.arg("state"); // "on" hoặc "off"
  state.toLowerCase();
  bool value = (state == "on");

  led1_state = value;
  led2_state = value;

  Serial.print("ALL LEDs -> ");
  Serial.println(value ? "ON" : "OFF");
  // TODO: set luôn GPIO thực tế nếu cần

  String json = "{\"led1\":\"" + String(led1_state ? "ON" : "OFF") +
                "\",\"led2\":\"" + String(led2_state ? "ON" : "OFF") + "\"}";
  server.send(200, "application/json", json);
}
void handleNeopixel()
{
  String hex = server.arg("hex"); // dạng "#RRGGBB"
  Serial.print("NEOPIXEL color: ");
  Serial.println(hex);
  // TODO: parse hex -> R,G,B rồi set NeoPixel
  server.send(200, "text/plain", "OK");
}
void handleToggle()
{
  int led = server.arg("led").toInt();
  if (led == 1)
  {
    led1_state = !led1_state;
    Serial.println("YOUR CODE TO CONTROL LED1");
  }
  else if (led == 2)
  {
    led2_state = !led2_state;
    Serial.println("YOUR CODE TO CONTROL LED2");
  }
  server.send(200, "application/json",
              "{\"led1\":\"" + String(led1_state ? "ON" : "OFF") +
                  "\",\"led2\":\"" + String(led2_state ? "ON" : "OFF") + "\"}");
}

void handleSensors()
{
  float t = data.temperature;
  float h = data.humidity;
  String json = "{\"temp\":" + String(t) + ",\"hum\":" + String(h) + "}";
  server.send(200, "application/json", json);
}

void handleSettings() { server.send(200, "text/html; charset=utf-8", settingsPage()); }

void handleWifiStatus()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    server.send(200, "text/plain", "connected");
  }
  else if (connecting)
  {
    server.send(200, "text/plain", "connecting");
  }
  else
  {
    server.send(200, "text/plain", "failed");
  }
}

void handleConnect()
{
  wifi_ssid = server.arg("ssid");
  wifi_password = server.arg("pass");
  server.send(200, "text/plain", "Connecting....");
  isAPMode = false;
  connecting = true;
  connect_start_ms = millis();
  connectToWiFi();
}

String getContentType(const String &filename)
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

bool serveFile(const String &path)
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

// ========== WiFi ==========
void handleRootPage()
{ // GET "/"
  if (!serveFile("/index.html"))
  {
    server.send(404, "text/plain", "index.html not found");
  }
}

void setupServer()
{
  // ==== Trang chính & các subpage ====
  server.on("/", HTTP_GET, handleRootPage);

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

  // ==== Static assets: CSS / JS / JSON ====
  server.on("/style.css", HTTP_GET, []()
            { serveFile("/style.css"); });
  server.on("/script.js", HTTP_GET, []()
            { serveFile("/script.js"); });
  server.on("/humidity.json", HTTP_GET, []()
            { serveFile("/humidity.json"); });
  server.on("/Thermometer.json", HTTP_GET, []()
            { serveFile("/Thermometer.json"); });

  // ==== API động bạn đã có sẵn ====
  server.on("/toggle", HTTP_GET, handleToggle);
  server.on("/set", HTTP_GET, handleSet);
  server.on("/set_all", HTTP_GET, handleSetAll);
  server.on("/neopixel", HTTP_GET, handleNeopixel);
  server.on("/sensors", HTTP_GET, handleSensors);
  server.on("/wifi_status", HTTP_GET, handleWifiStatus);
  server.on("/connect", HTTP_GET, handleConnect);

  // fallback: nếu path nào chưa khai báo mà trùng tên file trong SPIFFS thì vẫn serve được
  server.onNotFound([]()
                    {
      String uri = server.uri();
      if (!uri.startsWith("/")) uri = "/" + uri;
      if (!serveFile(uri)) {
        server.send(404, "text/plain", "404: Not found");
      } });

  server.begin();
}

void startAP()
{
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ssid.c_str(), password.c_str());
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  isAPMode = true;
  connecting = false;
}

void connectToWiFi()
{
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
  Serial.print("Connecting to: ");
  Serial.print(wifi_ssid.c_str());

  Serial.print(" Password: ");
  Serial.print(wifi_password.c_str());
}

// ========== Main task ==========
void main_server_task(void *pvParameters)
{
  pinMode(BOOT_PIN, INPUT_PULLUP);

  if (!SPIFFS.begin(true))
  { // true = format nếu mount fail (tùy bạn)
    Serial.println("SPIFFS mount failed");
  }
  else
  {
    Serial.println("SPIFFS mounted OK");
  }

  startAP();
  setupServer();

  while (1)
  {
    server.handleClient();

    // BOOT Button to switch to AP Mode
    if (digitalRead(BOOT_PIN) == LOW)
    {
      vTaskDelay(100);
      if (digitalRead(BOOT_PIN) == LOW)
      {
        if (!isAPMode)
        {
          startAP();
          setupServer();
        }
      }
    }

    // STA Mode
    if (connecting)
    {
      if (WiFi.status() == WL_CONNECTED)
      {
        Serial.print("STA IP address: ");
        Serial.println(WiFi.localIP());
        isWifiConnected = true; // Internet access

        xSemaphoreGive(xBinarySemaphoreInternet);

        isAPMode = false;
        connecting = false;
      }
      else if (millis() - connect_start_ms > 10000)
      { // timeout 10s
        Serial.println("WiFi connect failed! Back to AP.");
        startAP();
        setupServer();
        connecting = false;
        isWifiConnected = false;
      }
    }

    vTaskDelay(20); // avoid watchdog reset
  }
}