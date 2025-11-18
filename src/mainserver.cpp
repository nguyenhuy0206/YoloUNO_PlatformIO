#include "mainserver.h"
#include <WiFi.h>
#include <WebServer.h>

bool led1_state = false;
bool led2_state = false;
bool isAPMode = true;

WebServer server(80);



unsigned long connect_start_ms = 0;
bool connecting = false;

String mainPage() {
  float temperature = glob_temperature;
  float humidity    = glob_humidity; // nếu bạn chưa có biến này, giữ nguyên glob_temperature
  String led1 = led1_state ? "ON" : "OFF";
  String led2 = led2_state ? "ON" : "OFF";

  return R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <meta charset="UTF-8">
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>ESP32 Dashboard</title>
    <style>
      :root{
        --orange1:#ff6a00; --orange2:#ff7f11;
        --blue1:#0057c2;   --blue2:#0076ff;
        --card:#0e4770;    --bg:#f2f5f9;
        --btn-red:#ff4d4f; --btn-dark:#2e2f36;
        --white:#ffffff;   --muted:#9fb3c8;
        --radius:22px;
        --shadow:0 10px 25px rgba(0,0,0,.18);
      }
      *{box-sizing:border-box}
      html,body{height:100%}
      body{
        margin:0; font-family:system-ui,Segoe UI,Arial; background:var(--bg);
        display:flex; justify-content:center; align-items:flex-start; overflow:hidden;
      }
      /* giữ nguyên 1 trang, scale 90% như yêu cầu */
      .wrap{
        width:1180px; padding:18px 18px 26px;
        transform:scale(.9); transform-origin: top center;
      }
      .row{display:grid; gap:18px}
      .row.top{grid-template-columns:1fr 1fr; margin-bottom:18px}
      .kpi{
        color:#fff; border-radius:26px; box-shadow:var(--shadow); position:relative; overflow:hidden;
        min-height:150px; display:flex; align-items:center; padding:26px 32px;
      }
      .kpi .icon{
        width:120px; height:120px; border-radius:60px; background:rgba(255,255,255,.18);
        margin-right:26px; display:flex; align-items:center; justify-content:center; font-size:44px;
        box-shadow:inset 0 10px 30px rgba(0,0,0,.15);
      }
      .kpi .title{font-weight:800; letter-spacing:.5px; opacity:.95}
      .kpi .val{font-size:48px; font-weight:900; margin-top:8px}
      .temp{background:linear-gradient(135deg,var(--orange1),var(--orange2))}
      .hum {background:linear-gradient(135deg,var(--blue1),var(--blue2))}

      .row.cards{grid-template-columns:1fr 1fr 1fr}
      .card{
        background:#0b4a75; border-radius:30px; color:#e9f4ff; box-shadow:var(--shadow);
        padding:28px; text-align:center; position:relative; min-height:360px;
      }
      .card .bulb{
        width:120px; height:120px; border-radius:60px; background:#3d4a57; margin:0 auto 14px;
        display:flex; align-items:center; justify-content:center; font-size:44px; color:#f4f4f4;
        box-shadow:inset 0 8px 24px rgba(0,0,0,.18);
      }
      .card .name{font-size:28px; font-weight:800; letter-spacing:.6px; margin-top:6px}
      .state{font-size:36px; font-weight:900; margin:18px 0 10px}
      .btn{
        display:inline-block; border:none; padding:14px 22px; border-radius:14px; font-size:18px;
        margin:10px 8px 0; cursor:pointer; transition:transform .12s ease, box-shadow .12s ease, opacity .2s;
        box-shadow:0 6px 16px rgba(0,0,0,.18); color:#fff;
      }
      .btn:hover{transform:scale(1.05)}
      .btn:active{transform:scale(.98)}
      .btn-red{background:var(--btn-red)}
      .btn-dark{background:var(--btn-dark)}
      .btn-wide{padding:16px 30px; font-weight:700}
      .footer{display:flex; justify-content:center; gap:18px; margin-top:16px}

      /* Color Picker */
      .picker-box{
        width:140px; height:56px; border-radius:12px; background:#00ff3a; margin:22px auto 0;
        border:6px solid #e6eef7; box-shadow:var(--shadow); cursor:pointer;
      }
      .palette{
        position:absolute; left:50%; transform:translateX(-50%);
        bottom:28px; width:320px; background:#143e5f; border-radius:18px; padding:14px;
        box-shadow:var(--shadow); display:none;
      }
      .palette.show{display:block}
      .grid{
        display:grid; grid-template-columns:repeat(8, 1fr); gap:8px;
      }
      .sw{width:32px; height:32px; border-radius:8px; cursor:pointer; border:2px solid rgba(255,255,255,.5)}
      .picker-title{font-weight:800; letter-spacing:.6px; color:#cfe7ff; margin-bottom:10px; font-size:24px}
      .neo-title{font-size:24px; font-weight:800; letter-spacing:.6px; color:#cfe7ff; margin-bottom:12px}

      /* top settings button */
      #settings{
        position:fixed; right:16px; top:14px; background:#007bff; color:#fff; border:none;
        border-radius:10px; padding:10px 14px; cursor:pointer; box-shadow:var(--shadow);
        transition:transform .12s ease;
      }
      #settings:hover{transform:scale(1.05)}
      /* nhãn nhỏ góc KPI */
      .unit{opacity:.95; font-weight:700}
    </style>
  </head>
  <body>
    <button id="settings" onclick="window.location='/settings'">&#9881; Settings</button>

    <div class="wrap">

      <!-- KPI temperature & humidity -->
      <div class="row top">
        <div class="kpi temp">
          <div class="icon">🌡️</div>
          <div>
            <div class="title">TEMPERATURE</div>
            <div class="val"><span id="temp">)rawliteral" + String(temperature,1) + R"rawliteral(</span> <span class="unit">°C</span></div>
          </div>
        </div>
        <div class="kpi hum">
          <div class="icon">💧</div>
          <div>
            <div class="title">HUMIDITY</div>
            <div class="val"><span id="hum">)rawliteral" + String(humidity,1) + R"rawliteral(</span> <span class="unit">%</span></div>
          </div>
        </div>
      </div>

      <!-- LED & NeoPixel -->
      <div class="row cards">
        <!-- LED 1 -->
        <div class="card" id="card1">
          <div class="bulb">💡</div>
          <div class="name">LED 1</div>
          <div class="state" id="s1">)rawliteral" + led1 + R"rawliteral(</div>
          <div>
            <button class="btn btn-red btn-wide"  onclick="setLED(1,'on')">Turn ON</button>
            <button class="btn btn-dark btn-wide" onclick="setLED(1,'off')">Turn OFF</button>
          </div>
        </div>

        <!-- LED 2 -->
        <div class="card" id="card2">
          <div class="bulb">💡</div>
          <div class="name">LED 2</div>
          <div class="state" id="s2">)rawliteral" + led2 + R"rawliteral(</div>
          <div>
            <button class="btn btn-red btn-wide"  onclick="setLED(2,'on')">Turn ON</button>
            <button class="btn btn-dark btn-wide" onclick="setLED(2,'off')">Turn OFF</button>
          </div>
        </div>

        <!-- NeoPixel Color Picker -->
        <div class="card" id="neo">
          <div class="neo-title">NEOPIXEL</div>
          <div class="picker-title">Color Picker</div>
          <div class="picker-box" id="swatch" onclick="togglePalette()"></div>

          <div class="palette" id="palette">
            <div class="grid" id="grid"></div>
          </div>
        </div>
      </div>

      <!-- ALL ON / ALL OFF -->
      <div class="footer">
        <button class="btn btn-red btn-wide" onclick="setAll('on')">All ON</button>
        <button class="btn btn-dark btn-wide" onclick="setAll('off')">All OFF</button>
      </div>
    </div>

    <script>
      // ---- LED controls (đổi endpoint nếu backend khác) ----
      function setLED(id, state){
        fetch('/set?led='+id+'&state='+state)
          .then(r=>r.json())
          .then(j=>{
            if(j.led1) document.getElementById('s1').innerText = j.led1;
            if(j.led2) document.getElementById('s2').innerText = j.led2;
          })
          .catch(()=>{ // fallback đổi UI tạm
            const s = (state==='on')?'ON':'OFF';
            if(id===1) document.getElementById('s1').innerText = s;
            if(id===2) document.getElementById('s2').innerText = s;
          });
      }
      function setAll(state){
        fetch('/set_all?state='+state)
          .then(r=>r.json())
          .then(j=>{
            if(j.led1) document.getElementById('s1').innerText = j.led1;
            if(j.led2) document.getElementById('s2').innerText = j.led2;
          })
          .catch(()=>{
            const s=(state==='on')?'ON':'OFF';
            document.getElementById('s1').innerText=s;
            document.getElementById('s2').innerText=s;
          });
      }

      // ---- Sensors polling ----
      setInterval(()=>{
        fetch('/sensors')
          .then(res=>res.json())
          .then(d=>{
            if(d.temp!==undefined) document.getElementById('temp').innerText=d.temp;
            if(d.hum!==undefined)  document.getElementById('hum').innerText=d.hum;
          })
          .catch(()=>{});
      },3000);

      // ---- Color palette ----
      const COLORS = [
        // 8 x 8 = 64 màu RGB cơ bản (có cả đen/trắng)
        "#000000","#303030","#606060","#909090","#c0c0c0","#e0e0e0","#ffffff","#ff00ff",
        "#ff0000","#ff4000","#ff8000","#ffbf00","#ffff00","#bfff00","#80ff00","#40ff00",
        "#00ff00","#00ff40","#00ff80","#00ffbf","#00ffff","#00bfff","#0080ff","#0040ff",
        "#0000ff","#4000ff","#8000ff","#bf00ff","#ff00bf","#ff0080","#ff0040","#ff8080",
        "#cc0000","#cc3300","#cc6600","#cc9900","#cccc00","#99cc00","#66cc00","#33cc00",
        "#00cc00","#00cc33","#00cc66","#00cc99","#00cccc","#0099cc","#0066cc","#0033cc",
        "#0000cc","#3300cc","#6600cc","#9900cc","#cc00cc","#cc0099","#cc0066","#cc0033",
        "#ff6666","#ff9966","#ffcc66","#ffff66","#ccff66","#99ff66","#66ff66","#66ffff"
      ];
      const grid = document.getElementById('grid');
      COLORS.forEach(hex=>{
        const d=document.createElement('div');
        d.className='sw'; d.style.background=hex;
        d.title=hex;
        d.onclick=()=>pickColor(hex);
        grid.appendChild(d);
      });
      function togglePalette(){
        const p = document.getElementById('palette');
        p.classList.toggle('show');
      }
      function pickColor(hex){
        document.getElementById('swatch').style.background = hex;
        fetch('/neopixel?hex='+encodeURIComponent(hex)).catch(()=>{});
        document.getElementById('palette').classList.remove('show');
      }
  
      document.addEventListener('click', (e)=>{
        const pal = document.getElementById('palette');
        const neo = document.getElementById('neo');
        if(!neo.contains(e.target)) pal.classList.remove('show');
      });
    </script>
  </body>
  </html>
  )rawliteral";
}


String settingsPage() {
  return R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>
  <title>Settings</title>
  <style>
    :root{
      --orange1:#ff6a00; --orange2:#ff7f11;
      --blue1:#0057c2;   --blue2:#0076ff;
      --bg:#f2f5f9;      --card:#ffffff;
      --text:#0f1b2d;    --muted:#6b7a90;
      --primary:#ff4d4f; --dark:#2e2f36;
      --radius:22px;     --shadow:0 12px 28px rgba(16,24,40,.16);
    }
    *{box-sizing:border-box}
    body{
      margin:0; font-family:system-ui, Segoe UI, Roboto, Arial;
      background:var(--bg); color:var(--text);
      min-height:100vh; display:flex; align-items:center; justify-content:center;
    }
    .card{
      width:min(92vw, 460px); background:var(--card); border-radius:26px;
      box-shadow:var(--shadow); overflow:hidden;
    }
    .header{
      padding:26px 26px 20px; color:#fff;
      background:linear-gradient(135deg,var(--orange1),var(--orange2));
      position:relative;
    }
    .header h2{margin:0 0 6px; font-size:28px; letter-spacing:.4px}
    .header p{margin:0; opacity:.9}
    .gear{
      position:absolute; right:18px; top:16px; font-size:26px; opacity:.9
    }
    .body{padding:24px 22px 8px}
    .field{margin-bottom:16px}
    label{display:block; font-weight:700; margin:0 0 8px}
    .input-wrap{
      position:relative; display:flex; align-items:center;
      border:1.5px solid #e5e8ef; border-radius:14px; background:#fbfdff;
      padding:10px 14px;
    }
    .input-wrap:focus-within{border-color:#b4c6ff; box-shadow:0 0 0 4px rgba(0,118,255,.12)}
    .icon{margin-right:10px; font-size:18px; opacity:.85}
    input[type=text], input[type=password]{border:none; outline:none; width:100%; font-size:16px; background:transparent}
    .eye{
      cursor:pointer; user-select:none; font-size:18px; opacity:.7;
      transition:transform .12s ease
    }
    .eye:hover{transform:scale(1.05)}
    .actions{display:flex; gap:12px; margin-top:6px}
    .btn{
      flex:1; border:none; padding:14px 16px; border-radius:14px; font-size:16px;
      cursor:pointer; transition:transform .12s ease, box-shadow .12s ease, opacity .2s;
      box-shadow:0 8px 20px rgba(0,0,0,.12); font-weight:700; color:#fff;
    }
    .btn:hover{transform:scale(1.03)}
    .btn:active{transform:scale(.98)}
    .primary{background:var(--primary)}
    .secondary{background:var(--dark)}
    .msg{margin:14px 4px 2px; min-height:22px; color:var(--muted); font-weight:600}
    .footer{padding:16px; text-align:center; color:var(--muted); font-size:13px}

    /* loading spinner trong nút */
    .spinner{
      width:18px; height:18px; border-radius:50%;
      border:2.5px solid rgba(255,255,255,.5); border-top-color:#fff;
      animation:spin .9s linear infinite; display:inline-block; vertical-align:middle; margin-right:8px;
    }
    @keyframes spin{to{transform:rotate(360deg)}}

    /* toast */
    .toast{
      position:fixed; left:50%; transform:translateX(-50%);
      bottom:22px; background:#0f5132; color:#d1f7e3;
      padding:12px 16px; border-radius:12px; box-shadow:var(--shadow); display:none;
    }
    .toast.error{background:#5f1a1a; color:#ffd6d6}
    .toast.show{display:block}
  </style>
</head>
<body>
  <div class="card">
    <div class="header">
      <div class="gear">⚙️</div>
      <h2>Wi-Fi Settings</h2>
      <p>Nhập SSID và mật khẩu để kết nối mạng</p>
    </div>

    <div class="body">
      <form id="wifiForm">
        <div class="field">
          <label for="ssid">SSID</label>
          <div class="input-wrap">
            <input type="text" id="ssid" name="ssid" placeholder="Ví dụ: MyHome_2.4G" required>
          </div>
        </div>

        <div class="field">
          <label for="pass">Password</label>
          <div class="input-wrap">
            <input type="password" id="pass" name="password" placeholder="Tối thiểu 8 ký tự" required>
            <div class="eye" id="togglePass" title="Hiện/ẩn mật khẩu">👁️</div>
          </div>
        </div>

        <div class="actions">
          <button type="submit" class="btn primary" id="btnConnect">
            Connect
          </button>
          <button type="button" class="btn secondary" onclick="window.location='/'">Back</button>
        </div>
        <div id="msg" class="msg"></div>
      </form>
    </div>

    <div class="footer">ESP32 • Settings</div>
  </div>

  <div id="toast" class="toast">Đã kết nối Wi-Fi!</div>

  <script>
    // Toggle hiện/ẩn mật khẩu
    document.getElementById('togglePass').onclick = function(){
      const p = document.getElementById('pass');
      p.type = (p.type === 'password') ? 'text' : 'password';
    };

    const form = document.getElementById('wifiForm');
    const btn = document.getElementById('btnConnect');
    const msg = document.getElementById('msg');
    const toast = document.getElementById('toast');

    form.onsubmit = function(e){
      e.preventDefault();
      const ssid = document.getElementById('ssid').value.trim();
      const pass = document.getElementById('pass').value;

      // kiểm tra nhanh
      if(!ssid){ msg.textContent = "Vui lòng nhập SSID."; return; }
      if(pass.length < 8){ msg.textContent = "Mật khẩu tối thiểu 8 ký tự."; return; }

      // trạng thái loading
      const oldHTML = btn.innerHTML;
      btn.disabled = true;
      btn.innerHTML = '<span class="spinner"></span>Connecting...';
      msg.textContent = "";

      fetch('/connect?ssid='+encodeURIComponent(ssid)+'&pass='+encodeURIComponent(pass))
        .then(r=>r.text())
        .then(text=>{
          btn.disabled = false;
          btn.innerHTML = oldHTML;

          // Hiển thị message + toast
          msg.textContent = text;
          if(/ok|success|connected|thanh cong/i.test(text)){
            toast.className = 'toast show';
            toast.textContent = 'Kết nối Wi-Fi thành công!';
            setTimeout(()=>{ toast.classList.remove('show'); }, 2200);
            // Tùy ý: tự quay lại dashboard sau 1.5s
            setTimeout(()=>{ window.location = '/'; }, 1500);
          }else{
            toast.className = 'toast error show';
            toast.textContent = 'Kết nối thất bại. Kiểm tra SSID/mật khẩu.';
            setTimeout(()=>{ toast.classList.remove('show'); }, 2500);
          }
        })
        .catch(err=>{
          btn.disabled = false;
          btn.innerHTML = oldHTML;
          msg.textContent = "Lỗi kết nối: " + err;
          toast.className = 'toast error show';
          toast.textContent = 'Không gửi được yêu cầu đến thiết bị.';
          setTimeout(()=>{ toast.classList.remove('show'); }, 2500);
        });
    };
  </script>
</body>
</html>
)rawliteral";
}


// ========== Handlers ==========
void handleRoot() { server.send(200, "text/html; charset=utf-8", mainPage()); }

void handleSet() {
  int led = server.arg("led").toInt();
  String state = server.arg("state");  // "on" hoặc "off"
  state.toLowerCase();

  bool value = (state == "on");

  if (led == 1) {
    led1_state = value;
    Serial.print("LED1 -> ");
    Serial.println(led1_state ? "ON" : "OFF");
    // TODO: thêm YOUR CODE TO CONTROL LED1 ở đây, ví dụ:
    // digitalWrite(LED1_PIN, led1_state ? HIGH : LOW);
  } else if (led == 2) {
    led2_state = value;
    Serial.print("LED2 -> ");
    Serial.println(led2_state ? "ON" : "OFF");
    // TODO: YOUR CODE TO CONTROL LED2
  }

  String json = "{\"led1\":\"" + String(led1_state ? "ON" : "OFF") +
                "\",\"led2\":\"" + String(led2_state ? "ON" : "OFF") + "\"}";
  server.send(200, "application/json", json);
}
void handleSetAll() {
  String state = server.arg("state");  // "on" hoặc "off"
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
void handleNeopixel() {
  String hex = server.arg("hex");  // dạng "#RRGGBB"
  Serial.print("NEOPIXEL color: ");
  Serial.println(hex);
  // TODO: parse hex -> R,G,B rồi set NeoPixel
  server.send(200, "text/plain", "OK");
}
void handleToggle() {
  int led = server.arg("led").toInt();
  if (led == 1) {
    led1_state = !led1_state;
    Serial.println("YOUR CODE TO CONTROL LED1");
  }
  else if (led == 2){
    led2_state = !led2_state;
    Serial.println("YOUR CODE TO CONTROL LED2");
  }
  server.send(200, "application/json",
    "{\"led1\":\"" + String(led1_state ? "ON":"OFF") +
    "\",\"led2\":\"" + String(led2_state ? "ON":"OFF") + "\"}");
}

void handleSensors() {
  float t = glob_temperature;
  float h = glob_humidity;
  String json = "{\"temp\":"+String(t)+",\"hum\":"+String(h)+"}";
  server.send(200, "application/json", json);
}

void handleSettings() { server.send(200, "text/html; charset=utf-8", settingsPage()); }

void handleConnect() {
  wifi_ssid = server.arg("ssid");
  wifi_password = server.arg("pass");
  server.send(200, "text/plain", "Connecting....");
  isAPMode = false;
  connecting = true;
  connect_start_ms = millis();
  connectToWiFi();
}

// ========== WiFi ==========
void setupServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/toggle", HTTP_GET, handleToggle);
  server.on("/set", HTTP_GET, handleSet);       //add
  server.on("/set_all", HTTP_GET, handleSetAll);  //add 
  server.on("/neopixel", HTTP_GET, handleNeopixel); //add
  server.on("/sensors", HTTP_GET, handleSensors);
  server.on("/settings", HTTP_GET, handleSettings);
  server.on("/connect", HTTP_GET, handleConnect);
  server.begin();
}

void startAP() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid.c_str(), password.c_str());
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  isAPMode = true;
  connecting = false;
}

void connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
  Serial.print("Connecting to: ");
  Serial.print(wifi_ssid.c_str());

  Serial.print(" Password: ");
  Serial.print(wifi_password.c_str());
}

// ========== Main task ==========
void main_server_task(void *pvParameters){
  pinMode(BOOT_PIN, INPUT_PULLUP);
  startAP();
  setupServer();

  while(1){
    server.handleClient();

    // BOOT Button to switch to AP Mode
    if (digitalRead(BOOT_PIN) == LOW) {
      vTaskDelay(100);
      if (digitalRead(BOOT_PIN) == LOW) {
        if (!isAPMode) {
          startAP();
          setupServer();
        }
      }
    }

    // STA Mode
    if (connecting) {
      if (WiFi.status() == WL_CONNECTED) {
        Serial.print("STA IP address: ");
        Serial.println(WiFi.localIP());
        isWifiConnected = true; //Internet access

        xSemaphoreGive(xBinarySemaphoreInternet);

        isAPMode = false;
        connecting = false;
         
      } else if (millis() - connect_start_ms > 10000) { // timeout 10s
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