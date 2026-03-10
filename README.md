# 🚗 Vehicle Tracking Machine

![ESP32](https://img.shields.io/badge/ESP32-DevKit_V1-blue?logo=espressif&logoColor=white)
![GPS](https://img.shields.io/badge/GPS-NEO--6M-green)
![Node.js](https://img.shields.io/badge/Backend-Node.js-339933?logo=nodedotjs)
![License](https://img.shields.io/badge/License-MIT-yellow)

Real-time IoT vehicle tracking system using ESP32, GPS, IMU sensors and a live web dashboard.

---

## 🔧 Hardware

| Component | Model |
|---|---|
| Microcontroller | ESP32 DevKit V1 |
| GPS | NEO-6M |
| IMU | MPU6050 |
| Ultrasonic | HC-SR04 |
| Motor Driver | L298N |
| Motors | TT Gear Motors x2 |
| Power | 7.4V LiPo Battery |

---

## 📁 Project Structure

```
vehicle-tracking-machine/
├── firmware/        # ESP32 Arduino code
├── backend/         # Node.js + Express + MongoDB
├── frontend/public/ # Live Leaflet.js dashboard
└── docs/            # Wiring diagrams
```

---

## 🚀 Quick Start

**1. Clone**
```bash
git clone https://github.com/Hamzah1507/vehicle-tracking-machine.git
```

**2. Backend**
```bash
cd backend
npm install
cp .env.example .env
npm run dev
```

**3. Firmware**
- Open `firmware/main.ino` in Arduino IDE
- Update WiFi credentials and server IP
- Select board: **ESP32 Dev Module**
- Upload via USB (115200 baud)

**4. Dashboard** → `http://localhost:3000`

---

## 🔌 USB Upload (ESP32)

1. Install driver: **CP2102** or **CH340** (check chip near USB port)
2. Arduino IDE → Tools → Board → **ESP32 Dev Module**
3. Tools → Port → select your **COM port**
4. Press Upload (`Ctrl+U`)
> If upload fails → hold **BOOT button** while clicking Upload

---

## ⚙️ Tech Stack

`ESP32` `Arduino` `TinyGPS++` `MPU6050` `Node.js` `Express` `MongoDB` `Socket.IO` `Leaflet.js`

---

## 👤 Author

**Hamzah Saiyed** — [github.com/Hamzah1507](https://github.com/Hamzah1507) · hamzah.2004saiyed@gmail.com

> ⭐ Star this repo if it helped you!
