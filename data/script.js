// ========== SIDEBAR & LAYOUT ==========

(function () {
    const sidebar = document.getElementById('sidebar');
    const toggle = document.getElementById('sidebarToggle');
    const overlay = document.getElementById('sidebar-overlay') || document.querySelector('.sidebar-overlay');

    // If toggle exists -> wire collapse behavior
    if (toggle && sidebar) {
        toggle.addEventListener('click', () => {
            // collapse / expand
            sidebar.classList.toggle('sidebar-collapsed');
            // rotate chevron icon accordingly
            const icon = toggle.querySelector('.material-icons-outlined');
            if (icon) icon.textContent = sidebar.classList.contains('sidebar-collapsed') ? 'chevron_right' : 'chevron_left';
        });
    }

    // Activate correct menu item based on path or data-page
    const current = window.location.pathname.split('/').pop() || 'index.html';
    document.querySelectorAll('.sidebar-list-item').forEach(li => {
        const page = li.getAttribute('data-page') || (li.querySelector('a') && li.querySelector('a').getAttribute('href'));
        if (page && (current === page || (current === '' && page === 'dashboard.html'))) {
            li.classList.add('active');
        } else li.classList.remove('active');

        // click navigation behavior (if you use layout fetchs)
        li.addEventListener('click', () => {
            // if the list items have data-page and you're using layout loader, call loadPage
            const pageTo = li.getAttribute('data-page');
            if (pageTo && typeof loadPage === 'function') {
                loadPage(pageTo);
            } else if (pageTo) {
                // fallback: navigate to page
                window.location.href = pageTo;
            }
        });
    });

    // Mobile overlay open/close (optional)
    document.querySelectorAll('.menu-icon, .header .material-icons-outlined').forEach(btn => {
        btn && btn.addEventListener('click', () => {
            if (!sidebar) return;
            sidebar.classList.toggle('sidebar-open');
            overlay && overlay.classList.toggle('active');
        });
    });
    overlay && overlay.addEventListener('click', () => {
        if (!sidebar) return;
        sidebar.classList.remove('sidebar-open');
        overlay.classList.remove('active');
    });
})();


// ========== FAKE CARD DATA (WIND / RAIN) ==========

function initFakeCardData() {
    const fakeWindSpeedKmh = 30; // km/h
    const fakeWindSpeedMs = (fakeWindSpeedKmh / 3.6).toFixed(1); // m/s
    const fakeWindDirection = "NE";
    const fakeRain = 36.4;

    const windSpeedEl = document.getElementById("windSpeedValue");
    const windDirEl   = document.getElementById("windDirValue");
    const rainValEl   = document.getElementById("rainValue");

    if (windSpeedEl) windSpeedEl.textContent = fakeWindSpeedMs + " m/s";
    if (windDirEl)   windDirEl.textContent   = fakeWindDirection;
    if (rainValEl)   rainValEl.textContent   = fakeRain + " mm";
}


// ========== CHART LOGIC (RAIN / WIND) ==========

let rainfallChart, windSpeedChart;

function initDynamicCharts() {
    const rainfallCanvas = document.getElementById("rainfallChart");
    const windSpeedCanvas = document.getElementById("windSpeedChart");

    // Nếu dashboard hiện tại không có biểu đồ thì bỏ qua
    if (!rainfallCanvas || !windSpeedCanvas || typeof Chart === "undefined") return;

    const rainfallCtx = rainfallCanvas.getContext("2d");
    const windSpeedCtx = windSpeedCanvas.getContext("2d");

    rainfallChart = new Chart(rainfallCtx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'Rainfall (mm)',
                data: [],
                borderColor: 'rgb(137, 185, 133)',
                backgroundColor: 'rgba(137, 185, 133, 0.4)',
                pointBackgroundColor: 'rgb(137, 185, 133)',
                fill: true,
                tension: 0.4
            }]
        },
        options: getChartOptions()
    });

    windSpeedChart = new Chart(windSpeedCtx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'Wind Speed (m/s)',
                data: [],
                borderColor: 'rgb(100, 149, 237)',
                backgroundColor: 'rgba(100, 149, 237, 0.3)',
                pointBackgroundColor: 'rgb(100, 149, 237)',
                fill: true,
                tension: 0.4
            }]
        },
        options: getChartOptions()
    });

    // Cập nhật mỗi 3 giây
    setInterval(updateCharts, 3000);
}

function getChartOptions() {
    return {
        responsive: true,
        animation: false,
        scales: {
            x: {
                ticks: { color: '#fff' },
                grid: { color: '#444' },
            },
            y: {
                beginAtZero: true,
                ticks: { color: '#fff' },
                grid: { color: '#444' }
            }
        },
        plugins: {
            legend: {
                labels: { color: '#fff' }
            }
        }
    };
}

let lastTimeLabel = "";

function updateCharts() {
    if (!rainfallChart || !windSpeedChart) return;

    const now = new Date();
    const timeLabel = getRoundedTimeLabel(now); // làm tròn 5 phút

    // Chỉ cập nhật nếu là thời gian mới
    if (timeLabel === lastTimeLabel) return;
    lastTimeLabel = timeLabel;

    const rainfallValue = (Math.random() * 5).toFixed(2);
    const windSpeedValue = (Math.random() * 20).toFixed(1);

    appendData(rainfallChart, timeLabel, rainfallValue);
    appendData(windSpeedChart, timeLabel, windSpeedValue);

    // Cập nhật biểu tượng
    const windValueEl = document.querySelector('.card:nth-child(1) h1');
    if (windValueEl) windValueEl.textContent = `${windSpeedValue} m/s`;

    const rainIcon = document.getElementById("rainIcon");
    if (rainIcon && typeof rainIcon.setSpeed === "function") {
        const rainSpeed = Math.min(Math.max(rainfallValue / 2, 0.5), 2);
        rainIcon.setSpeed(rainSpeed);
    }
}

// Làm tròn giờ hiện tại về mốc 5 phút gần nhất, trả về chuỗi HH:mm
function getRoundedTimeLabel(date) {
    const minutes = date.getMinutes();
    const roundedMinutes = Math.floor(minutes / 5) * 5;
    const hours = date.getHours();

    const hh = hours.toString().padStart(2, "0");
    const mm = roundedMinutes.toString().padStart(2, "0");

    return `${hh}:${mm}`;
}

function appendData(chart, label, value) {
    chart.data.labels.push(label);
    chart.data.datasets[0].data.push(value);

    if (chart.data.labels.length > 10) {
        chart.data.labels.shift();
        chart.data.datasets[0].data.shift();
    }

    chart.update();
}

function updateEnvFromSensors() {
    const tempEl = document.getElementById("temperature");
    const humEl  = document.getElementById("humidity");

    // Nếu không ở trang dashboard (không có 2 ID này) thì thôi
    if (!tempEl || !humEl) return;

    fetch("/sensors")
        .then(res => res.json())
        .then(data => {
            if (typeof data.temp === "number") {
                tempEl.textContent = data.temp.toFixed(1) + " °C";
            }
            if (typeof data.hum === "number") {
                humEl.textContent = data.hum.toFixed(1) + " %";
            }
        })
        .catch(err => {
            console.error("Error fetching /sensors:", err);
        });
}

// Khi dashboard đã load xong (index + dashboard.html đã chèn vào DOM)
window.addEventListener("load", () => {
    // gọi lần đầu
    updateEnvFromSensors();
    // cập nhật lại mỗi 3 giây
    setInterval(updateEnvFromSensors, 3000);
});

// ---- ĐIỀU KHIỂN LED & NEOPIXEL (phù hợp với /set, /set_all, /neopixel) ----

function updateLedUI(data) {
    const status1 = document.getElementById('led1Status');
    const icon1   = document.getElementById('led1Icon');
    const status2 = document.getElementById('led2Status');
    const icon2   = document.getElementById('led2Icon');

    if (data.led1 && status1 && icon1) {
        status1.innerText = data.led1;
        if (data.led1 === "ON") {
            icon1.classList.add("led-on");
            icon1.classList.remove("led-off");
        } else {
            icon1.classList.add("led-off");
            icon1.classList.remove("led-on");
        }
    }

    if (data.led2 && status2 && icon2) {
        status2.innerText = data.led2;
        if (data.led2 === "ON") {
            icon2.classList.add("led-on");
            icon2.classList.remove("led-off");
        } else {
            icon2.classList.add("led-off");
            icon2.classList.remove("led-on");
        }
    }
}

// device: 'led1' hoặc 'led2', state: 'on' | 'off'
function toggleDevice(device, state) {
    let ledId = 0;
    if (device === 'led1') ledId = 1;
    else if (device === 'led2') ledId = 2;
    else return;

    fetch(`/set?led=${ledId}&state=${state}`)
        .then(res => res.json())
        .then(data => {
            console.log("SET response:", data);
            updateLedUI(data);
        })
        .catch(err => console.error("Device control error:", err));
}

function setNeoColor(color) {
    fetch(`/neopixel?hex=${encodeURIComponent(color)}`)
        .then(res => res.text())
        .then(text => {
            console.log("NEOPIXEL response:", text);
            const neoStatus = document.getElementById("neoStatus");
            if (neoStatus) {
                neoStatus.innerText = `Color: ${color.toUpperCase()}`;
            }
        })
        .catch(err => console.error("NeoPixel Error:", err));
}

function allOn() {
    fetch('/set_all?state=on')
        .then(res => res.json())
        .then(data => {
            console.log("ALL ON response:", data);
            updateLedUI(data);
        })
        .catch(err => console.error("All ON error:", err));
}

function allOff() {
    fetch('/set_all?state=off')
        .then(res => res.json())
        .then(data => {
            console.log("ALL OFF response:", data);
            updateLedUI(data);
        })
        .catch(err => console.error("All OFF error:", err));
}


// ================== WIFI CONFIG PAGE ==================
document.addEventListener("DOMContentLoaded", () => {
  const form  = document.getElementById("wifiForm");
  if (!form) return;           // nếu không phải wifi.html thì thôi

  const btn   = document.getElementById("btnConnect");
  const msg   = document.getElementById("msg");
  const toast = document.getElementById("toast");
  const togglePass = document.getElementById("togglePass");
  const passInput  = document.getElementById("pass");

  // Hiện / ẩn mật khẩu
  if (togglePass && passInput) {
    togglePass.onclick = () => {
      passInput.type = (passInput.type === "password") ? "text" : "password";
    };
  }

  form.onsubmit = (e) => {
    e.preventDefault();

    const ssid = document.getElementById("ssid").value.trim();
    const pass = passInput.value;

    if (!ssid) {
      msg.textContent = "Vui lòng nhập SSID.";
      return;
    }
    if (pass.length < 8) {
      msg.textContent = "Mật khẩu tối thiểu 8 ký tự.";
      return;
    }

    const oldHTML = btn.innerHTML;
    btn.disabled = true;
    btn.innerHTML = '<span class="spinner"></span>Connecting...';
    msg.textContent = "";

    // Gửi yêu cầu connect tới ESP32
    fetch('/connect?ssid=' + encodeURIComponent(ssid) +
          '&pass=' + encodeURIComponent(pass))
      .then(r => r.text())
      .then(() => {
        // báo đang kết nối
        toast.className = 'toast show';
        toast.textContent = 'Đang kết nối Wi-Fi...';

        // check /wifi_status mỗi 1s
        let check = setInterval(() => {
          fetch('/wifi_status')
            .then(r => r.text())
            .then(st => {
              console.log('wifi_status =', st);

              if (st === 'connected') {
                clearInterval(check);
                toast.textContent = 'Kết nối thành công!';
                setTimeout(() => {
                  // ✅ chỉ khi connected mới chuyển trang
                  window.location.href = '/';
                }, 1200);
              } else if (st === 'failed') {
                clearInterval(check);
                toast.className = 'toast error show';
                toast.textContent = 'Kết nối thất bại. Kiểm tra SSID/mật khẩu.';

                btn.disabled = false;
                btn.innerHTML = oldHTML;
              }
              // st === 'connecting' → cứ để đó, không redirect
            })
            .catch(err => {
              console.error("wifi_status error:", err);
            });
        }, 1000);
      })
      .catch(err => {
        console.error("connect error:", err);
        toast.className = 'toast error show';
        toast.textContent = 'Không gửi được yêu cầu đến thiết bị.';

        btn.disabled = false;
        btn.innerHTML = oldHTML;
      });
  };
});
