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
            sidebar.classList.toggle('sidebar-open');
            overlay && overlay.classList.toggle('active');
        });
    });
    overlay && overlay.addEventListener('click', () => {
        sidebar.classList.remove('sidebar-open');
        overlay.classList.remove('active');
    });
})();
function initFakeCardData() {
    const fakeWindSpeedKmh = 30; // km/h
    const fakeWindSpeedMs = (fakeWindSpeedKmh / 3.6).toFixed(1); // m/s
    const fakeWindDirection = "NE";
    const fakeRain = 36.4;

    document.getElementById("windSpeedValue").textContent = fakeWindSpeedMs + " m/s";
    document.getElementById("windDirValue").textContent = fakeWindDirection;
    document.getElementById("rainValue").textContent = fakeRain + " mm";
}



// ===== CHART LOGIC =====
let rainfallChart, windSpeedChart;

function initDynamicCharts() {
    const rainfallCtx = document.getElementById("rainfallChart").getContext("2d");
    const windSpeedCtx = document.getElementById("windSpeedChart").getContext("2d");

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

    setInterval(updateCharts, 3000); // Cập nhật mỗi 3 giây
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

window.onload = function () {
    initFakeCardData(); // Hiển thị dữ liệu ảo
    initDynamicCharts(); // Load biểu đồ
};



function toggleDevice(device, state) {
    // Gửi request tới ESP32 (ví dụ: /?device=led1&state=on)
    fetch(`/?device=${device}&state=${state}`)
        .then(res => res.json())
        .then(data => {
            const statusEl = document.getElementById(`${device}Status`);
            const iconEl = document.getElementById(`${device}Icon`);

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

// Khi trang load, đảm bảo các icon ở trạng thái tắt
window.addEventListener("load", () => {
    ["led1Icon", "led2Icon"].forEach(id => {
        document.getElementById(id).classList.add("led-off");
    });
});

