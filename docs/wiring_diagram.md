# 🔌 Wiring Diagram — Vehicle Tracking Machine

## ESP32 Pin Map

```
                      ┌─────────────────────────────────┐
                      │         ESP32 DevKit V1          │
                      │                                  │
     GPS NEO-6M TX ───│ GPIO 16 (RX2)    GPIO 25 ───────│─── L298N IN1
     GPS NEO-6M RX ───│ GPIO 17 (TX2)    GPIO 26 ───────│─── L298N IN2
                      │                  GPIO 27 ───────│─── L298N IN3
  MPU6050 SDA ────────│ GPIO 21 (SDA)    GPIO 14 ───────│─── L298N IN4
  MPU6050 SCL ────────│ GPIO 22 (SCL)    GPIO 32 ───────│─── L298N ENA (PWM)
                      │                  GPIO 33 ───────│─── L298N ENB (PWM)
  HC-SR04 TRIG ───────│ GPIO 18                         │
  HC-SR04 ECHO* ──────│ GPIO 19          GPIO 2 ────────│─── LED (GPS)
                      │                  GPIO 4 ────────│─── LED (WiFi)
                      │                  GPIO 5 ────────│─── LED (Motor)
                      │                                  │
              GND ────│ GND              3.3V ───────────│─── GPS VCC
              GND ────│ GND              3.3V ───────────│─── MPU6050 VCC
              GND ────│ GND              5V  ────────────│─── HC-SR04 VCC
                      └─────────────────────────────────┘

* HC-SR04 ECHO is 5V — use voltage divider:
  ECHO → 1kΩ → GPIO19
              ↓
             2kΩ
              ↓
             GND
```

## L298N Motor Driver

```
  L298N Driver
  ┌──────────────────────────┐
  │  IN1 ←── ESP32 GPIO 25  │
  │  IN2 ←── ESP32 GPIO 26  │──── Motor A (Left Wheel)
  │  ENA ←── ESP32 GPIO 32  │
  │                          │
  │  IN3 ←── ESP32 GPIO 27  │
  │  IN4 ←── ESP32 GPIO 14  │──── Motor B (Right Wheel)
  │  ENB ←── ESP32 GPIO 33  │
  │                          │
  │  VCC ←── Battery 7.4V   │
  │  GND ←── Common GND     │
  │  5V  ──► ESP32 VIN       │  (L298N has onboard 5V reg)
  └──────────────────────────┘
```

## Power Distribution

```
  LiPo Battery 7.4V
       │
       ├──── L298N VCC (motor power)
       │         └── L298N 5V out → ESP32 VIN
       │
       └──── Common GND (ESP32 + L298N + all sensors)

  ⚠️  Do NOT power ESP32 from USB while motors are running —
     motor noise can cause brownouts. Use the battery via L298N.
```

## Component Checklist

Before powering on, verify:

- [ ] All GNDs connected to a common ground
- [ ] Voltage divider on HC-SR04 ECHO line
- [ ] GPS module has clear sky view (indoor may not get fix)
- [ ] MPU6050 I2C address = 0x68 (AD0 pin to GND)
- [ ] Motor polarity correct (swap wires if spinning wrong way)
- [ ] WiFi credentials set in firmware
- [ ] Server IP address set in firmware
- [ ] Backend server running before powering ESP32