function updateEnvFromSensors() {
    const tempEl = document.getElementById("temperature");
    const humEl = document.getElementById("humidity");

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


function updateDeviceUI(data) {
    // === LED 1 & LED 2 ===
    const devices = ['led1', 'led2', 'pump', 'fan'];

    devices.forEach(device => {
        const statusElement = document.getElementById(device + 'Status');
        const iconWrapper = document.getElementById(device + 'Icon');
        const dataKey = device; // Tên key trong JSON API: data.led1, data.pump, data.fan

        if (data[dataKey] && statusElement && iconWrapper) {
            const status = data[dataKey];
            statusElement.innerText = status;

            if (status === "ON") {
                iconWrapper.classList.add("led-on");
                iconWrapper.classList.remove("led-off");
            } else {
                iconWrapper.classList.add("led-off");
                iconWrapper.classList.remove("led-on");
            }
        }
    });
}

// device: 'led1', 'led2', 'pump', hoặc 'fan', state: 'on' | 'off'
function toggleDevice(device, state) {
    let ledId = 0;

    // Nếu là LED, dùng ID số (1 hoặc 2) như API cũ của bạn
    if (device === 'led1') ledId = 1;
    else if (device === 'led2') ledId = 2;
    // Nếu là Fan/Pump, dùng tên thiết bị (string)
    else if (device === 'pump') ledId = 'pump';
    else if (device === 'fan') ledId = 'fan';
    else return; // Thiết bị không xác định

    fetch(`/set?led=${ledId}&state=${state}`)
        .then(res => res.json())
        .then(data => {
            console.log("SET response:", data);
            // Sử dụng hàm cập nhật UI mới
            updateDeviceUI(data);
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
            // Hàm updateDeviceUI sẽ cập nhật tất cả (LED, Pump, Fan) nếu API /set_all trả về status của tất cả.
            updateDeviceUI(data);
        })
        .catch(err => console.error("All ON error:", err));
}

function allOff() {
    fetch('/set_all?state=off')
        .then(res => res.json())
        .then(data => {
            console.log("ALL OFF response:", data);
            // Hàm updateDeviceUI sẽ cập nhật tất cả (LED, Pump, Fan)
            updateDeviceUI(data);
        })
        .catch(err => console.error("All OFF error:", err));
}


// ================== WIFI CONFIG PAGE ==================
document.addEventListener("DOMContentLoaded", () => {
    const form = document.getElementById("wifiForm");
    if (!form) return;           // nếu không phải wifi.html thì thôi

    const btn = document.getElementById("btnConnect");
    const msg = document.getElementById("msg");
    const toast = document.getElementById("toast");
    const togglePass = document.getElementById("togglePass");
    const passInput = document.getElementById("pass");

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
// =======================================================
// ========== LOGIC POLLING TRẠNG THÁI VÀ KHỞI ĐỘNG ==========
// =======================================================

/**
 * Hàm Polling: Lấy trạng thái hiện tại từ ESP32 qua API /state.
 * Hàm này tận dụng updateDeviceUI đã có sẵn.
 */
function pollDeviceState() {
    fetch('/state')
        .then(res => {
            if (!res.ok) {
                console.error("Failed to fetch /state:", res.status);
                throw new Error('Network response was not ok');
            }
            return res.json();
        })
        .then(data => {
            // Sử dụng hàm đã có để cập nhật giao diện
            updateDeviceUI(data);
        })
        .catch(error => console.error('Polling state error:', error));
}


// Thiết lập Khởi động và Lặp lại (Chạy sau khi DOM load xong)
document.addEventListener("DOMContentLoaded", () => {
    // ... (logic WIFI CONFIG PAGE hiện có của bạn) ...

    const form = document.getElementById("wifiForm");

    // Thêm logic khởi động cho trang điều khiển
    if (window.location.pathname.includes('control.html') ||
        (window.location.pathname === '/' && !form)) {
        // 1. Lấy trạng thái ban đầu khi tải trang
        pollDeviceState();

        // 2. Thiết lập Polling lặp lại mỗi 1 giây (1000ms)
        setInterval(pollDeviceState, 1000);
    }
});

function openWifiSettings() { // wifi.html mới dùng UI “Wi-Fi Settings” bạn vừa dán window.location.href = '/wifi.html'; // hoặc 'wifi.html' tuỳ server }
    window.location.href = '/wifi.html'; // hoặc 'wifi.html' tuỳ server
}


function updateWifiDashboard(status, ssid) {
    applyWifiStatusUI(status, ssid);

}

function updateWifiInfo() {
    fetch("/wifi_info")
        .then(res => res.json())
        .then(info => {
            const statusEl = document.getElementById("wifiStatusDashboard");
            const ssidEl = document.getElementById("wifiSsidDashboard");
            const btn = document.querySelector(".wifi-config-btn");

            if (!statusEl || !ssidEl) return;

            statusEl.textContent = info.status || "--";
            ssidEl.textContent = info.ssid ? `SSID: ${info.ssid}` : "SSID: --";

            if (btn) {
                btn.style.display = (info.status === "connected")
                    ? "none"
                    : "inline-flex";
            }
        })
        .catch(err => console.error("wifi_info error:", err));
}

function applyWifiStatusUI(rawStatus, ssid) {
    const statusBtn = document.getElementById("wifiStatusDashboard");
    const ssidEl = document.getElementById("wifiSsidDashboard");
    const cfgBtn = document.querySelector(".wifi-config-btn");

    if (!statusBtn || !ssidEl) return;

    const s = (rawStatus || "").toLowerCase();
    let label = "Unknown";
    let className = "status-unknown";

    switch (s) {
        case "connected":
            label = "Connected";
            className = "status-connected";
            break;
        case "connecting":
            label = "Connecting…";
            className = "status-connecting";
            break;
        case "failed":
        case "disconnected":
            label = "Disconnected";
            className = "status-failed";
            break;
        default:
            label = "Unknown";
            className = "status-unknown";
            break;
    }

    // reset class
    statusBtn.classList.remove(
        "status-connected",
        "status-connecting",
        "status-failed",
        "status-unknown"
    );
    statusBtn.classList.add(className);
    statusBtn.textContent = label;

    ssidEl.textContent = ssid ? `SSID: ${ssid}` : "SSID: --";

    if (cfgBtn) {
        cfgBtn.style.display = (s === "connected") ? "none" : "inline-flex";
    }
}

// Hàm gọi từ dashboard khi polling
function updateWifiInfo() {
    fetch("/wifi_info")
        .then(res => res.json())
        .then(info => {
            applyWifiStatusUI(info.status, info.ssid);
        })
        .catch(err => console.error("wifi_info error:", err));
}


function updateTinyMLInfo() {
    const labelEl = document.getElementById("tinymlCurrentLabel");
    const summaryEl = document.getElementById("tinymlSummary");

    if (!labelEl || !summaryEl) return; // không phải dashboard.html

    fetch("/tinyml_info")
        .then(res => res.json())
        .then(info => {
            const cls = typeof info.class === "number" ? info.class : -1;
            const prob = typeof info.prob === "number" ? info.prob : NaN;
            const label = info.label || "--";

            // dòng chữ lớn
            labelEl.textContent = label;

            // summary nhỏ phía dưới
            let desc = "";
            switch (cls) {
                case 0:
                    desc = " Cool";
                    break;
                case 1:
                    desc = "Neutral";
                    break;
                case 2:
                    desc = "Warm";
                    break;
                default:
                    desc = "No recent predictions.";
                    break;
            }


            summaryEl.textContent = desc;
        })
        .catch(err => {
            console.error("tinyml_info error:", err);
        });
}
