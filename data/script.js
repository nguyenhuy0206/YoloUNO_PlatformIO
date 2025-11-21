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


// ========== JS CŨ: LED / SENSOR / NEOPIXEL ==========

// Điều khiển LED bằng API /set và /set_all
function setLED(id, state) {
    fetch('/set?led=' + id + '&state=' + state)
        .then(r => r.json())
        .then(j => {
            const s1 = document.getElementById('s1');
            const s2 = document.getElementById('s2');
            if (j.led1 && s1) s1.innerText = j.led1;
            if (j.led2 && s2) s2.innerText = j.led2;
        })
        .catch(() => {
            const s = (state === 'on') ? 'ON' : 'OFF';
            const s1 = document.getElementById('s1');
            const s2 = document.getElementById('s2');
            if (id === 1 && s1) s1.innerText = s;
            if (id === 2 && s2) s2.innerText = s;
        });
}

function setAll(state) {
    fetch('/set_all?state=' + state)
        .then(r => r.json())
        .then(j => {
            const s1 = document.getElementById('s1');
            const s2 = document.getElementById('s2');
            if (j.led1 && s1) s1.innerText = j.led1;
            if (j.led2 && s2) s2.innerText = j.led2;
        })
        .catch(() => {
            const s = (state === 'on') ? 'ON' : 'OFF';
            const s1 = document.getElementById('s1');
            const s2 = document.getElementById('s2');
            if (s1) s1.innerText = s;
            if (s2) s2.innerText = s;
        });
}

// Polling /sensors để cập nhật temp/hum
setInterval(() => {
    fetch('/sensors')
        .then(res => res.json())
        .then(d => {
            if (d.temp !== undefined) {
                const tempEl = document.getElementById('temp');
                if (tempEl) tempEl.innerText = d.temp;
            }
            if (d.hum !== undefined) {
                const humEl = document.getElementById('hum');
                if (humEl) humEl.innerText = d.hum;
            }
        })
        .catch(() => { });
}, 3000);

// NeoPixel color palette
const COLORS = [
    "#000000","#303030","#606060","#909090","#c0c0c0","#e0e0e0","#ffffff","#ff00ff",
    "#ff0000","#ff4000","#ff8000","#ffbf00","#ffff00","#bfff00","#80ff00","#40ff00",
    "#00ff00","#00ff40","#00ff80","#00ffbf","#00ffff","#00bfff","#0080ff","#0040ff",
    "#0000ff","#4000ff","#8000ff","#bf00ff","#ff00bf","#ff0080","#ff0040","#ff8080",
    "#cc0000","#cc3300","#cc6600","#cc9900","#cccc00","#99cc00","#66cc00","#33cc00",
    "#00cc00","#00cc33","#00cc66","#00cc99","#00cccc","#0099cc","#0066cc","#0033cc",
    "#0000cc","#3300cc","#6600cc","#9900cc","#cc00cc","#cc0099","#cc0066","#cc0033",
    "#ff6666","#ff9966","#ffcc66","#ffff66","#ccff66","#99ff66","#66ff66","#66ffff"
];

let legacyControlInitialized = false;

function initLegacyControlPage() {
    const grid = document.getElementById('grid');
    const palette = document.getElementById('palette');
    const neo = document.getElementById('neo');

    // Nếu không có các phần tử này thì không phải trang control cũ → bỏ qua
    if (!grid || !palette || !neo) return;

    if (legacyControlInitialized) return;
    legacyControlInitialized = true;

    // Tạo palette màu
    grid.innerHTML = "";
    COLORS.forEach(hex => {
        const d = document.createElement('div');
        d.className = 'sw';
        d.style.background = hex;
        d.title = hex;
        d.onclick = () => pickColor(hex);
        grid.appendChild(d);
    });

    // togglePalette được gọi từ onclick trong HTML
    window.togglePalette = function () {
        palette.classList.toggle('show');
    };

    function pickColor(hex) {
        const swatch = document.getElementById('swatch');
        if (swatch) swatch.style.background = hex;
        fetch('/neopixel?hex=' + encodeURIComponent(hex)).catch(() => { });
        palette.classList.remove('show');
    }

    // Đóng palette nếu click ra ngoài
    document.addEventListener('click', (e) => {
        if (!neo.contains(e.target)) palette.classList.remove('show');
    });
}


// ========== JS CŨ: WIFI SETTINGS PAGE ==========

function initWifiPage() {
    const form = document.getElementById('wifiForm');
    const btn = document.getElementById('btnConnect');
    const msg = document.getElementById('msg');
    const toast = document.getElementById('toast');
    const toggle = document.getElementById('togglePass');

    if (!form || !btn || !msg || !toast || !toggle) return; // không phải trang wifi

    // Toggle hiện/ẩn mật khẩu
    toggle.onclick = function () {
        const p = document.getElementById('pass');
        if (!p) return;
        p.type = (p.type === 'password') ? 'text' : 'password';
    };

    form.onsubmit = function (e) {
        e.preventDefault();
        const ssid = document.getElementById('ssid').value.trim();
        const pass = document.getElementById('pass').value;

        // kiểm tra nhanh
        if (!ssid) { msg.textContent = "Vui lòng nhập SSID."; return; }
        if (pass.length < 8) { msg.textContent = "Mật khẩu tối thiểu 8 ký tự."; return; }

        // trạng thái loading
        const oldHTML = btn.innerHTML;
        btn.disabled = true;
        btn.innerHTML = '<span class="spinner"></span>Connecting...';
        msg.textContent = "";

        fetch('/connect?ssid=' + encodeURIComponent(ssid) + '&pass=' + encodeURIComponent(pass))
            .then(r => r.text())
            .then(text => {
                // Hiển thị “đang kết nối…”
                toast.className = 'toast show';
                toast.textContent = 'Đang kết nối Wi-Fi...';

                // bắt đầu kiểm tra trạng thái mỗi 1 giây
                let check = setInterval(() => {
                    fetch('/wifi_status')
                        .then(r => r.text())
                        .then(st => {
                            if (st === 'connected') {
                                clearInterval(check);
                                toast.textContent = 'Kết nối thành công!';
                                setTimeout(() => { window.location = '/'; }, 1200);
                            }
                            if (st === 'failed') {
                                clearInterval(check);
                                toast.className = 'toast error show';
                                toast.textContent = 'Kết nối thất bại. Kiểm tra SSID/mật khẩu.';
                                btn.disabled = false;
                                btn.innerHTML = oldHTML;
                            }
                        });
                }, 1000);
            })
            .catch(err => {
                toast.className = 'toast error show';
                toast.textContent = 'Không gửi được yêu cầu đến thiết bị.';
                btn.disabled = false;
                btn.innerHTML = oldHTML;
            });
    };
}


// ========== DEVICE CONTROL MỚI (LED ICON) ==========

function toggleDevice(device, state) {
    // Gửi request tới ESP32 (ví dụ: /?device=led1&state=on)
    // LƯU Ý: backend của bạn hiện tại dùng /set, /set_all.
    // Hàm này bạn có thể chỉnh sau cho khớp API, tạm để nguyên như bạn đang dùng.
    fetch(`/?device=${device}&state=${state}`)
        .then(res => res.json())
        .then(data => {
            const statusEl = document.getElementById(`${device}Status`);
            const iconEl = document.getElementById(`${device}Icon`);

            if (!statusEl || !iconEl) return;

            // Cập nhật trạng thái UI
            if (state === "on") {
                statusEl.innerText = "ON";
                iconEl.classList.add("led-on");
                iconEl.classList.remove("led-off");
            } else {
                statusEl.innerText = "OFF";
                iconEl.classList.add("led-off");
                iconEl.classList.remove("led-on");
            }
        })
        .catch(err => console.error("Device control error:", err));
}


// ========== GLOBAL LOAD INIT ==========

window.addEventListener("load", () => {
    // Dashboard (wind/rain cards + charts)
    initFakeCardData();
    initDynamicCharts();

    // Legacy control page (LED + NeoPixel palette)
    initLegacyControlPage();

    // Wi-Fi settings page
    initWifiPage();

    // Khi trang load, đảm bảo các icon ở trạng thái tắt
    ["led1Icon", "led2Icon"].forEach(id => {
        const el = document.getElementById(id);
        if (el) el.classList.add("led-off");
    });
});
