📘 Thermal Comfort Model – How It Works
1. 📥 Model Input – Nhận dữ liệu gì?

Mô hình nhận 2 giá trị đầu vào (đã chuẩn hóa):

Input	Ý nghĩa
input[0]	Temperature (normalized)
input[1]	Humidity (normalized)

Các giá trị này là bản đã chuẩn hóa từ dữ liệu sensor bằng MinMaxScaler giống hệt khi training.

✔ Công thức chuẩn hóa

Dùng thông số từ norm_stats_minmax.json:

x_norm = x * scale[i] + shift[i]


Trong đó:

scale[i] = JSON "scale"

shift[i] = JSON "min_"

2. 🔄 Quy trình xử lý trước khi đưa vào model

Thuật toán trên thiết bị IoT chạy như sau:

Step 1 — Đọc sensor
float ta = glob_temperature;   // °C
float rh = glob_humidity;      // %

Step 2 — Giới hạn giá trị

Tránh sensor lỗi hoặc vượt range training:

Temp trong [20.1 → 47]
Humidity trong [18.5 → 93]

Step 3 — Chuẩn hóa MinMaxScaler

Ví dụ JSON:

"scale": [0.03717472, 0.0134228188],
"min_": [-0.74721187, -0.24832214]


MCU tính:

ta_norm = ta * 0.03717472   + (-0.74721187)
rh_norm = rh * 0.0134228188 + (-0.24832214)

Step 4 — Ghi vào input tensor
input->data.f[0] = ta_norm;
input->data.f[1] = rh_norm;

3. 🤖 Model Output – Trả ra gì?

Mô hình trả về 3 xác suất softmax:

Output index	Ý nghĩa
out[0]	Cool
out[1]	Neutral
out[2]	Warm

Ví dụ:

out = [0.12, 0.73, 0.15]

4. 🧠 Cách chọn kết quả tốt nhất

Ta chọn class có xác suất cao nhất:

best_prob  = max(out0, out1, out2)
best_class = vị trí của giá trị lớn nhất


Ví dụ:

Neutral = 0.73 → class = 1

5. 💬 Map Class → Comfort Text
Class	Comfort
0	Cool
1	Neutral
2	Warm
6. 📤 Kết quả cuối cùng trả ra

Thiết bị sẽ in ra dạng:

Temp=29.3°C, Humidity=62% → Comfort: Neutral (prob=0.73)
Raw probs: [0.12, 0.73, 0.15]


Hoặc gửi MQTT:

{
  "temperature": 29.3,
  "humidity": 62,
  "comfort_class": 1,
  "comfort_text": "Neutral",
  "probability": 0.73
}

🎯 Tóm tắt (TL;DR)

Input model = 2 giá trị chuẩn hóa: temperature_norm + humidity_norm

Normalize bằng công thức MinMaxScaler dùng scale + min_

Output model = 3 xác suất

Lấy cái lớn nhất → quyết định comfort

Map class:

0 → Cool

1 → Neutral

2 → Warm