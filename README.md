# 🚗 Vehicle Tracking Machine — IoT Project

<div align="center">

![ESP32](https://img.shields.io/badge/ESP32-DevKit_V1-blue?logo=espressif&logoColor=white)
![GPS](https://img.shields.io/badge/GPS-NEO--6M-green)
![IMU](https://img.shields.io/badge/IMU-MPU6050-orange)
![Node.js](https://img.shields.io/badge/Backend-Node.js_+_Express-339933?logo=nodedotjs)
![MongoDB](https://img.shields.io/badge/Database-MongoDB-47A248?logo=mongodb)
![Socket.IO](https://img.shields.io/badge/Realtime-Socket.IO-010101?logo=socketdotio)
![License](https://img.shields.io/badge/License-MIT-yellow)

**A fully functional IoT vehicle tracking system built with ESP32, GPS, IMU, and a real-time web dashboard.**

</div>

---

## 📋 Table of Contents

- [Overview](#overview)
- [Hardware Components](#hardware-components)
- [Circuit Wiring](#circuit-wiring)
- [Project Structure](#project-structure)
- [Features](#features)
- [USB Connection Manual](#usb-connection-manual)
- [Setup & Installation](#setup--installation)
- [How It Works](#how-it-works)
- [API Reference](#api-reference)
- [Dashboard](#dashboard)
- [Tech Stack](#tech-stack)
- [Author](#author)

---

## Overview

This IoT project tracks a physical vehicle (or robot car) in real time using:

- An **ESP32** microcontroller as the brain
- A **NEO-6M GPS module** for location tracking
- An **MPU6050 IMU** (accelerometer + gyroscope) for direction sensing
- An **HC-SR04 ultrasonic sensor** for obstacle detection
- An **L298N motor driver** to control 2 DC motors (wheels)
- A **Node.js + Socket.IO backend** that receives data and pushes it live
- A **Leaflet.js web dashboard** to visualise the vehicle on a real map

When powered on, the ESP32 reads sensor data every 50ms, drives the motors based on tilt/direction, avoids obstacles automatically, and sends a JSON packet to the backend every 2 seconds over WiFi.

---

## Hardware Components

| Component | Model | Purpose |
|---|---|---|
| Microcontroller | ESP32 DevKit V1 | Main processor + WiFi |
| GPS Module | NEO-6M | Real-time location |
| IMU | MPU6050 | Accelerometer + Gyroscope |
| Ultrasonic | HC-SR04 | Obstacle detection |
| Motor Driver | L298N | Controls 2 DC motors |
| DC Motors | TT Gear Motors (x2) | Drive wheels |
| Power | 7.4V LiPo Battery | Powers motors + ESP32 |
| Chassis | 2WD Robot Car Frame | Physical structure |

---

## Circuit Wiring

```
ESP32 GPIO    →    Component Pin
─────────────────────────────────────────────────────────
GPIO 25       →    L298N IN1
GPIO 26       →    L298N IN2
GPIO 27       →    L298N IN3
GPIO 14       →    L298N IN4
GPIO 32       →    L298N ENA  (PWM)
GPIO 33       →    L298N ENB  (PWM)

GPIO 16 (RX2) →    NEO-6M TX
GPIO 17 (TX2) →    NEO-6M RX
GND           →    NEO-6M GND
3.3V          →    NEO-6M VCC

GPIO 21 (SDA) →    MPU6050 SDA
GPIO 22 (SCL) →    MPU6050 SCL
GND           →    MPU6050 GND
3.3V          →    MPU6050 VCC

GPIO 18       →    HC-SR04 TRIG
GPIO 19       →    HC-SR04 ECHO (via voltage divider!)
GND           →    HC-SR04 GND
5V            →    HC-SR04 VCC

GPIO 2        →    LED (GPS lock indicator)
GPIO 4        →    LED (WiFi indicator)
GPIO 5        →    LED (Motor activity indicator)
─────────────────────────────────────────────────────────
⚠️  HC-SR04 ECHO outputs 5V — use a 1kΩ/2kΩ voltage divider
    to bring it down to 3.3V before connecting to ESP32!
```

---

## Project Structure

```
vehicle-tracking/
│
├── 📁 firmware/                 # ESP32 Arduino code
│   ├── main.ino                 # Main program loop
│   ├── motor_control.h          # L298N motor driver library
│   └── gps_utils.h              # GPS parsing & utilities
│
├── 📁 backend/                  # Node.js server
│   ├── server.js                # Express + Socket.IO + MongoDB
│   ├── package.json             # Dependencies
│   └── .env.example             # Environment variables template
│
├── 📁 frontend/
│   └── public/
│       └── index.html           # Live tracking dashboard
│
├── 📁 docs/
│   └── wiring_diagram.md        # Circuit reference
│
└── README.md
```

---

## Features

### Firmware (ESP32)
- ✅ Real-time GPS location + speed + heading
- ✅ IMU-based direction detection (FORWARD / BACKWARD / LEFT / RIGHT)
- ✅ Obstacle detection with automatic motor stop
- ✅ PWM motor speed control + smooth ramp
- ✅ WiFi data transmission every 2 seconds
- ✅ LED status indicators (GPS lock, WiFi, motor activity)
- ✅ Serial debug logging at 115200 baud

### Backend (Node.js)
- ✅ REST API for receiving and querying location data
- ✅ Real-time Socket.IO broadcast to dashboard
- ✅ MongoDB storage with geospatial indexing
- ✅ Rate limiting (120 req/min per device)
- ✅ Historical track query with date filters
- ✅ Statistics endpoint (avg speed, max speed, obstacle count)

### Dashboard (Web)
- ✅ Live Leaflet.js map with vehicle marker + track trail
- ✅ Direction indicator with animated arrow
- ✅ GPS coordinates, speed, heading display
- ✅ IMU accelerometer bar gauges
- ✅ Obstacle warning alert (pulsing animation)
- ✅ Connection status indicator
- ✅ Packet counter

---

## 🔌 USB Connection Manual

This section explains exactly how to connect your ESP32 to your PC via USB, install drivers, and upload firmware step by step.

---

### Step 1 — What You Need

| Item | Details |
|---|---|
| USB Cable | **Micro-USB** (ESP32 DevKit V1 uses Micro-USB — NOT USB-C) |
| PC | Windows 10/11, macOS, or Linux |
| Arduino IDE | Version 2.x recommended — [download here](https://www.arduino.cc/en/software) |

> ⚠️ Make sure your cable is a **data cable**, not just a charging cable. Many cheap cables only charge and cannot transfer data. If your PC doesn't detect the ESP32, try a different cable first.

---

### Step 2 — Install the USB Driver (CP2102 / CH340)

ESP32 DevKit V1 boards use one of two USB-to-Serial chips. You must install the correct driver or your PC will not see the COM port.

**Check which chip your board has** — look at the small IC near the USB port:

| Chip Marking | Driver to Install |
|---|---|
| `CP2102` or `CP2104` | [Silicon Labs CP210x Driver](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers) |
| `CH340` or `CH341` | [CH340 Driver](https://www.wch-ic.com/downloads/CH341SER_EXE.html) |

**Installation steps (Windows):**
1. Download the driver from the link above
2. Run the installer as Administrator
3. Click **Install** and wait for "Install OK"
4. Plug in your ESP32 via USB
5. Open **Device Manager** → expand **Ports (COM & LPT)**
6. You should see something like `Silicon Labs CP210x USB to UART Bridge (COM3)` or `USB-SERIAL CH340 (COM4)`
7. Note your COM port number — you will need it in Arduino IDE

**macOS:**
- CP2102: Download and install the `.dmg` from Silicon Labs
- CH340: Download and install the `.pkg` from the link above
- After install, the port appears as `/dev/cu.SLAB_USBtoUART` or `/dev/cu.usbserial-XXXX`

**Linux (Ubuntu/Debian):**
- CH340 and CP2102 drivers are usually built into the kernel — no install needed
- Plug in the ESP32 and check: `ls /dev/ttyUSB*` — you should see `/dev/ttyUSB0`
- If permission denied: `sudo usermod -aG dialout $USER` then log out and back in

---

### Step 3 — Add ESP32 Board to Arduino IDE

Arduino IDE does not support ESP32 by default. You must add it manually.

1. Open Arduino IDE
2. Go to **File → Preferences**
3. In the **"Additional boards manager URLs"** field, paste:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Click **OK**
5. Go to **Tools → Board → Boards Manager**
6. Search for `esp32`
7. Find **"esp32 by Espressif Systems"** and click **Install**
8. Wait for download to complete (may take a few minutes)

---

### Step 4 — Physical USB Connection

```
  Your Laptop / PC
       │
       │  USB-A port
       │
  [USB Cable — Micro-USB]
       │
       │  Micro-USB port (bottom of ESP32 board)
       │
  ┌────▼────────────────────────┐
  │     ESP32 DevKit V1         │
  │  ┌──────────────────────┐   │
  │  │  Micro-USB Port      │◄──│── Plug cable here
  │  └──────────────────────┘   │
  │                             │
  │  [PWR LED should light up]  │
  └─────────────────────────────┘
```

When connected correctly:
- The **red power LED** on the ESP32 board lights up immediately
- Windows plays the "device connected" sound
- A new COM port appears in Device Manager

---

### Step 5 — Configure Arduino IDE for ESP32

1. Go to **Tools → Board → ESP32 Arduino**
2. Select **"ESP32 Dev Module"**
3. Configure the port settings exactly as below:

| Setting | Value |
|---|---|
| Board | ESP32 Dev Module |
| Upload Speed | **115200** |
| CPU Frequency | 240MHz (WiFi/BT) |
| Flash Frequency | 80MHz |
| Flash Mode | QIO |
| Flash Size | 4MB (32Mb) |
| Partition Scheme | Default 4MB with spiffs |
| Core Debug Level | None |
| PSRAM | Disabled |
| Port | COM3 (or your port from Step 2) |

4. Go to **Tools → Port** and select your COM port (e.g. `COM3` on Windows, `/dev/ttyUSB0` on Linux)

---

### Step 6 — Install Required Libraries

Go to **Tools → Manage Libraries** and search for + install each of these:

| Library | Author | Version |
|---|---|---|
| `TinyGPS++` | Mikal Hart | Latest |
| `MPU6050` | Electronic Cats | Latest |
| `ArduinoJson` | Benoit Blanchon | Latest (v7.x) |

---

### Step 7 — Upload the Firmware

1. Open `firmware/main.ino` in Arduino IDE
2. Update your credentials at the top of the file:
   ```cpp
   const char* WIFI_SSID     = "YOUR_WIFI_NAME";
   const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
   const char* SERVER_URL    = "http://192.168.1.YOUR_PC_IP:3000/api/location";
   ```
   > To find your PC's IP: run `ipconfig` (Windows) or `ifconfig` (Linux/Mac) and look for the IPv4 address on your WiFi adapter.
3. Click the **Upload button** (→ arrow) or press `Ctrl + U`
4. You will see in the console:

   ```
   Connecting........_____....
   Chip is ESP32-D0WDQ6 (revision 1)
   Uploading stub...
   Running stub...
   Stub running...
   Changing baud rate to 921600
   Changed.
   Configuring flash size...
   Compressed 847936 bytes to 512345...
   Writing at 0x00010000... (100 %)
   Leaving...
   Hard resetting via RTS pin...
   ```

5. After upload completes, the ESP32 **automatically resets and runs the code**

---

### Step 8 — Monitor Serial Output

To verify everything is working:

1. Go to **Tools → Serial Monitor** (or press `Ctrl + Shift + M`)
2. Set baud rate to **115200** (bottom-right of Serial Monitor window)
3. Press the **EN (Reset) button** on the ESP32

You should see output like:
```
[BOOT] Vehicle Tracking Machine v1.0.0
[BOOT] Author: Hamzah Saiyed
[INIT] Pins configured
[WIFI] Connecting to MyWiFi........
[WIFI] Connected! IP: 192.168.1.105
[GPS] NEO-6M module initialised on UART2
[MPU] MPU6050 connected successfully
[BOOT] All systems initialised. Starting main loop...
[GPS] Lat: 23.022505 | Lon: 72.571365 | Speed: 0.00 km/h | Heading: 0.0°
[HTTP] Data sent OK | Direction: STOP
```

---

### Troubleshooting USB Issues

| Problem | Solution |
|---|---|
| No COM port appears | Try a different USB cable (must be data cable, not charge-only) |
| "Permission denied" on Linux | Run `sudo usermod -aG dialout $USER` and reboot |
| Upload fails with "connecting..." | Hold the **BOOT button** on ESP32 while clicking Upload, release after "Connecting..." appears |
| Board not found in IDE | Re-install ESP32 board package via Boards Manager |
| Garbled text in Serial Monitor | Make sure baud rate is set to **115200** |
| WiFi not connecting | Double-check SSID/password — ESP32 only supports 2.4GHz WiFi, not 5GHz |
| HTTP send failed | Ensure PC and ESP32 are on the same WiFi network, and backend server is running |

---

## Setup & Installation

### 1. Clone the Repository

```bash
git clone https://github.com/Hamzah1507/vehicle-tracking-machine.git
cd vehicle-tracking-machine
```

### 2. Backend Setup

```bash
cd backend
npm install

# Copy environment file and fill in your values
cp .env.example .env
nano .env

# Start server (development)
npm run dev

# Start server (production)
npm start
```

> Make sure MongoDB is running: `mongod --dbpath /data/db`

### 3. Firmware Setup

1. Open `firmware/main.ino` in **Arduino IDE**
2. Install required libraries via Library Manager:
   - `TinyGPS++` by Mikal Hart
   - `MPU6050` by Electronic Cats
   - `ArduinoJson` by Benoit Blanchon
3. Update WiFi credentials in `main.ino`:
   ```cpp
   const char* WIFI_SSID     = "YOUR_SSID";
   const char* WIFI_PASSWORD = "YOUR_PASSWORD";
   const char* SERVER_URL    = "http://192.168.1.YOUR_IP:3000/api/location";
   ```
4. Select board: **ESP32 Dev Module**
5. Upload to ESP32

### 4. Open Dashboard

Navigate to `http://localhost:3000` in your browser.

---

## How It Works

```
ESP32 Loop (every 50ms)
    │
    ├── Read GPS (UART2)     → latitude, longitude, speed, heading
    ├── Read IMU (I2C)       → accelXYZ, gyroXYZ
    ├── Read Ultrasonic      → distance_cm
    │
    ├── If obstacle < 30cm   → Stop motors ⚠️
    ├── Else                 → Determine direction from IMU
    │                          → Drive motors accordingly
    │
    └── Every 2 seconds      → POST JSON to backend
                                     │
                                 MongoDB save
                                     │
                              Socket.IO broadcast
                                     │
                            Dashboard updates live 🗺️
```

---

## API Reference

### `POST /api/location`
Receive a data packet from ESP32.

**Body (JSON):**
```json
{
  "latitude": 23.022505,
  "longitude": 72.571365,
  "speed_kmph": 5.2,
  "heading": 180.0,
  "direction": "FORWARD",
  "accel_x": 0.12,
  "accel_y": 0.95,
  "accel_z": 0.04,
  "gyro_x": 1.2,
  "gyro_y": 0.5,
  "gyro_z": -0.3,
  "distance_cm": 85,
  "obstacle_detected": false
}
```

### `GET /api/location/latest`
Returns the most recent vehicle state.

### `GET /api/location/history?limit=50&from=ISO&to=ISO`
Returns historical track data.

### `GET /api/stats`
Returns summary statistics.

### `GET /health`
Server health check.

---

## Dashboard

Open `http://localhost:3000` to see:

- 🗺️ **Live map** — vehicle moves in real time
- 🧭 **Direction** — arrow updates with each packet
- 📍 **GPS data** — coordinates, speed, heading
- 📐 **IMU gauges** — accelerometer X/Y/Z bar graphs
- 🔊 **Ultrasonic** — distance + obstacle alert
- 🕐 **Live packet counter**

---

## Tech Stack

| Layer | Technology |
|---|---|
| Microcontroller | ESP32 (Arduino framework) |
| GPS Library | TinyGPS++ |
| IMU Library | MPU6050 (Electronic Cats) |
| JSON | ArduinoJson |
| Backend | Node.js, Express.js |
| Real-time | Socket.IO |
| Database | MongoDB + Mongoose |
| Security | Helmet, CORS, Rate Limiting |
| Frontend | HTML5, CSS3, JavaScript |
| Map | Leaflet.js + OpenStreetMap |

---

## Author

**Hamzah Saiyed**
- 📧 hamzah.2004saiyed@gmail.com
- 🐙 [github.com/Hamzah1507](https://github.com/Hamzah1507)
---

## License

MIT License — free to use, modify, and distribute.

---

> ⭐ If this project helped you, please consider giving it a star on GitHub!
