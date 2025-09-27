#include "mainserver.h"

bool led1_state = false;
bool led2_state = false;
bool isAPMode = true;

WebServer server(80);

String ssid = "ESP32-AP";
String password = "12345678";
// These will be replaced with user input after "Settings"
String wifi_ssid = "";
String wifi_password = "";


String mainPage() {
  float temperature = 10;// dht.getTemperature(); // Replace these with actual readings
  float humidity = 20; //dht.getHumidity();
  String led1 = led1_state ? "ON" : "OFF";
  String led2 = led2_state ? "ON" : "OFF";

  return R"rawliteral(
    <!DOCTYPE html><html><head>
      <meta name='viewport' content='width=device-width, initial-scale=1.0'>
      <title>ESP32 Dashboard</title>
      <style>
        body { font-family: Arial;text-align:center; margin:0;}
        .container { margin:20px auto; max-width:350px; background:#f9f9f9; border-radius:10px; box-shadow:0 2px 10px #ccc;padding:20px;}
        button { padding:10px 15px; margin:10px; font-size:18px;}
        #settings { float:right; background:#007bff;color:white; border-radius:4px;}
      </style>
    </head>
    <body>
      <div class='container'>
        <h2>ESP32 Dashboard</h2>
        <div>
          <b>Temperature:</b> <span id='temp'>)rawliteral" + String(temperature) + R"rawliteral(</span> &deg;C<br>
          <b>Humidity:</b> <span id='hum'>)rawliteral" + String(humidity) + R"rawliteral(</span> %<br>
        </div>
        <div>
            <button onclick='toggleLED(1)'>LED1: <span id="l1">)rawliteral" + led1 + R"rawliteral(</span></button>
            <button onclick='toggleLED(2)'>LED2: <span id="l2">)rawliteral" + led2 + R"rawliteral(</span></button>
        </div>
        <button id="settings" onclick="window.location='/settings'">&#9881; Settings</button>
      </div>
      <script>
        function toggleLED(id) {
          fetch('/toggle?led='+id)
          .then(response=>response.json())
          .then(json=>{
            document.getElementById('l1').innerText=json.led1;
            document.getElementById('l2').innerText=json.led2;
          });
        }
        setInterval(()=>{
          fetch('/sensors')
           .then(res=>res.json())
           .then(d=>{
             document.getElementById('temp').innerText=d.temp;
             document.getElementById('hum').innerText=d.hum;
           });
        },3000);
      </script>
    </body></html>
  )rawliteral";
}


String settingsPage() {
  return R"rawliteral(
    <!DOCTYPE html><html><head>
      <meta name='viewport' content='width=device-width, initial-scale=1.0'>
      <title>Settings</title>
      <style>
        body { font-family: Arial; text-align:center; margin:0;}
        .container { margin:20px auto; max-width:350px;background:#f9f9f9;border-radius:10px;box-shadow:0 2px 10px #ccc;padding:20px;}
        input[type=text], input[type=password]{width:90%;padding:10px;}
        button { padding:10px 15px; margin:10px; font-size:18px;}
      </style>
    </head>
    <body>
      <div class='container'>
        <h2>Wi-Fi Settings</h2>
        <form id="wifiForm">
          <input name="ssid" id="ssid" placeholder="SSID" required><br>
          <input name="password" id="pass" type="password" placeholder="Password" required><br><br>
          <button type="submit">Connect</button>
          <button type="button" onclick="window.location='/'">Back</button>
        </form>
        <div id="msg"></div>
      </div>
      <script>
        document.getElementById('wifiForm').onsubmit = function(e){
          e.preventDefault();
          let ssid = document.getElementById('ssid').value;
          let pass = document.getElementById('pass').value;
          fetch('/connect?ssid='+encodeURIComponent(ssid)+'&pass='+encodeURIComponent(pass))
            .then(r=>r.text())
            .then(msg=>{
              document.getElementById('msg').innerText=msg;
            });
        };
      </script>
    </body></html>
  )rawliteral";
}



void handleRoot() {
  server.send(200, "text/html", mainPage());
}

void handleToggle() {
  int led = server.arg("led").toInt();
  if (led == 1) {
    led1_state = !led1_state;
    //TODO: Control your LED1
    //digitalWrite(LED1_PIN, led1_state);
  } else if (led == 2) {
    led2_state = !led2_state;
    //TODO: Control your LED2
    //digitalWrite(LED2_PIN, led2_state);
  }
  server.send(200, "application/json", "{\"led1\":\"" + String(led1_state ? "ON":"OFF") + "\",\"led2\":\"" + String(led2_state ? "ON":"OFF") + "\"}");
}

void handleSensors() {
  float t = 10; //dht.getTemperature();
  float h = 20; //dht.getHumidity();
  String json = "{\"temp\":"+String(t)+",\"hum\":"+String(h)+"}";
  server.send(200, "application/json", json);
}

void handleSettings() {
  server.send(200, "text/html", settingsPage());
}

void handleConnect() {
  wifi_ssid = server.arg("ssid");
  wifi_password = server.arg("pass");
  server.send(200, "text/plain", "Connecting....");
  // Switch to station mode after response, see below
  isAPMode = false;
}

void setupServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/toggle", HTTP_GET, handleToggle);
  server.on("/sensors", HTTP_GET, handleSensors);
  server.on("/settings", HTTP_GET, handleSettings);
  server.on("/connect", HTTP_GET, handleConnect);
  server.begin();
}


void startAP() {
  WiFi.softAP(ssid.c_str(), password.c_str());
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  isAPMode = true;
}

void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  for (int i=0; i<20 && WiFi.status() != WL_CONNECTED; i++) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED){
    Serial.print("STA IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi Connect failed!");
    startAP(); // fallback to AP
  }
}

void main_server_task(void *pvParameters){
  while(1){
    server.handleClient();

    // On-the-fly mode switching (check every loop)
    if (digitalRead(BOOT_PIN) == LOW) { // pressed (active low)
      delay(100); // debounce
      if (digitalRead(BOOT_PIN) == LOW) {
        if (!isAPMode) {
          startAP();
          setupServer();
        }
      }
    }
    if (!isAPMode && WiFi.status() != WL_CONNECTED) {
      connectToWiFi();
    }
  }

}