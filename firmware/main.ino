/*
 * ============================================================
 *  Vehicle Tracking Machine - Firmware (ESP32)
 *  Author  : Hamzah Saiyed
 *  Version : 1.0.0
 *  Board   : ESP32 DevKit V1
 *  Sensors : GPS (NEO-6M), MPU6050 (IMU), HC-SR04 (Ultrasonic)
 * ============================================================
 *
 *  Features:
 *   - Real-time GPS location tracking
 *   - IMU-based direction & tilt sensing
 *   - Motor direction control via L298N driver
 *   - WiFi-based data push to backend server
 *   - Ultrasonic obstacle detection
 *   - Serial debug logging
 * ============================================================
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <TinyGPS++.h>
#include <Wire.h>
#include <MPU6050.h>
#include <HardwareSerial.h>

// ─────────────────────────────────────────────
//  WiFi Credentials
// ─────────────────────────────────────────────
const char* WIFI_SSID     = "YOUR_SSID";
const char* WIFI_PASSWORD = "YOUR_PASSWORD";

// ─────────────────────────────────────────────
//  Backend Server
// ─────────────────────────────────────────────
const char* SERVER_URL = "http://192.168.1.100:3000/api/location";

// ─────────────────────────────────────────────
//  Pin Definitions
// ─────────────────────────────────────────────

// Motor Driver L298N
#define MOTOR_IN1   25
#define MOTOR_IN2   26
#define MOTOR_IN3   27
#define MOTOR_IN4   14
#define MOTOR_ENA   32   // PWM enable A
#define MOTOR_ENB   33   // PWM enable B

// Ultrasonic HC-SR04
#define TRIG_PIN    18
#define ECHO_PIN    19

// GPS Serial (UART2)
#define GPS_RX      16
#define GPS_TX      17
#define GPS_BAUD    9600

// LED Indicators
#define LED_GPS     2
#define LED_WIFI    4
#define LED_MOVE    5

// ─────────────────────────────────────────────
//  Constants
// ─────────────────────────────────────────────
#define OBSTACLE_THRESHOLD_CM   30
#define SEND_INTERVAL_MS        2000
#define MOTOR_SPEED_DEFAULT     180   // 0-255 PWM
#define SERIAL_BAUD             115200

// ─────────────────────────────────────────────
//  Objects
// ─────────────────────────────────────────────
TinyGPSPlus   gps;
MPU6050       mpu;
HardwareSerial gpsSerial(2);
HTTPClient    http;

// ─────────────────────────────────────────────
//  Global State
// ─────────────────────────────────────────────
struct VehicleState {
  double   latitude;
  double   longitude;
  double   speed_kmph;
  double   heading;
  float    accelX, accelY, accelZ;
  float    gyroX,  gyroY,  gyroZ;
  float    temperature;
  long     distance_cm;
  String   direction;      // "FORWARD", "BACKWARD", "LEFT", "RIGHT", "STOP"
  bool     obstacleDetected;
  unsigned long lastSendTime;
};

VehicleState vehicle = {0};

// ─────────────────────────────────────────────
//  Setup
// ─────────────────────────────────────────────
void setup() {
  Serial.begin(SERIAL_BAUD);
  Serial.println("\n[BOOT] Vehicle Tracking Machine v1.0.0");
  Serial.println("[BOOT] Author: Hamzah Saiyed");

  initPins();
  initWiFi();
  initGPS();
  initMPU();

  Serial.println("[BOOT] All systems initialised. Starting main loop...");
  vehicle.direction = "STOP";
}

// ─────────────────────────────────────────────
//  Main Loop
// ─────────────────────────────────────────────
void loop() {
  readGPS();
  readIMU();
  vehicle.distance_cm = readUltrasonic();

  // Obstacle detection logic
  vehicle.obstacleDetected = (vehicle.distance_cm < OBSTACLE_THRESHOLD_CM);
  if (vehicle.obstacleDetected) {
    Serial.printf("[OBSTACLE] Object detected at %ld cm — stopping motors\n", vehicle.distance_cm);
    stopMotors();
    vehicle.direction = "STOP";
  }

  // Determine movement direction from IMU
  if (!vehicle.obstacleDetected) {
    determineDirection();
    driveMotors();
  }

  // Push data to server periodically
  if (millis() - vehicle.lastSendTime >= SEND_INTERVAL_MS) {
    sendDataToServer();
    vehicle.lastSendTime = millis();
  }

  delay(50);
}

// ─────────────────────────────────────────────
//  Initialisation Functions
// ─────────────────────────────────────────────
void initPins() {
  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(MOTOR_IN3, OUTPUT);
  pinMode(MOTOR_IN4, OUTPUT);
  pinMode(MOTOR_ENA, OUTPUT);
  pinMode(MOTOR_ENB, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(LED_GPS,  OUTPUT);
  pinMode(LED_WIFI, OUTPUT);
  pinMode(LED_MOVE, OUTPUT);

  stopMotors();
  Serial.println("[INIT] Pins configured");
}

void initWiFi() {
  Serial.printf("[WIFI] Connecting to %s", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\n[WIFI] Connected! IP: %s\n", WiFi.localIP().toString().c_str());
    digitalWrite(LED_WIFI, HIGH);
  } else {
    Serial.println("\n[WIFI] Failed to connect. Continuing without WiFi.");
  }
}

void initGPS() {
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);
  Serial.println("[GPS] NEO-6M module initialised on UART2");
}

void initMPU() {
  Wire.begin();
  mpu.initialize();
  if (mpu.testConnection()) {
    Serial.println("[MPU] MPU6050 connected successfully");
    mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
    mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_250);
  } else {
    Serial.println("[MPU] MPU6050 connection FAILED");
  }
}

// ─────────────────────────────────────────────
//  Sensor Reading Functions
// ─────────────────────────────────────────────
void readGPS() {
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      if (gps.location.isUpdated()) {
        vehicle.latitude   = gps.location.lat();
        vehicle.longitude  = gps.location.lng();
        vehicle.speed_kmph = gps.speed.kmph();
        vehicle.heading    = gps.course.deg();
        digitalWrite(LED_GPS, HIGH);
        Serial.printf("[GPS] Lat: %.6f | Lon: %.6f | Speed: %.2f km/h | Heading: %.1f°\n",
                      vehicle.latitude, vehicle.longitude,
                      vehicle.speed_kmph, vehicle.heading);
      }
    }
  }
}

void readIMU() {
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  vehicle.accelX = ax / 16384.0;
  vehicle.accelY = ay / 16384.0;
  vehicle.accelZ = az / 16384.0;
  vehicle.gyroX  = gx / 131.0;
  vehicle.gyroY  = gy / 131.0;
  vehicle.gyroZ  = gz / 131.0;
}

long readUltrasonic() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  long distance = duration * 0.034 / 2;
  return distance;
}

// ─────────────────────────────────────────────
//  Direction & Motor Control
// ─────────────────────────────────────────────
void determineDirection() {
  float tiltThreshold = 0.3;
  float gyroThreshold = 30.0;

  if (vehicle.accelY > tiltThreshold) {
    vehicle.direction = "FORWARD";
  } else if (vehicle.accelY < -tiltThreshold) {
    vehicle.direction = "BACKWARD";
  } else if (vehicle.gyroZ > gyroThreshold) {
    vehicle.direction = "LEFT";
  } else if (vehicle.gyroZ < -gyroThreshold) {
    vehicle.direction = "RIGHT";
  } else {
    vehicle.direction = "STOP";
  }
}

void driveMotors() {
  analogWrite(MOTOR_ENA, MOTOR_SPEED_DEFAULT);
  analogWrite(MOTOR_ENB, MOTOR_SPEED_DEFAULT);
  digitalWrite(LED_MOVE, HIGH);

  if (vehicle.direction == "FORWARD") {
    digitalWrite(MOTOR_IN1, HIGH); digitalWrite(MOTOR_IN2, LOW);
    digitalWrite(MOTOR_IN3, HIGH); digitalWrite(MOTOR_IN4, LOW);

  } else if (vehicle.direction == "BACKWARD") {
    digitalWrite(MOTOR_IN1, LOW);  digitalWrite(MOTOR_IN2, HIGH);
    digitalWrite(MOTOR_IN3, LOW);  digitalWrite(MOTOR_IN4, HIGH);

  } else if (vehicle.direction == "LEFT") {
    analogWrite(MOTOR_ENA, 80);
    digitalWrite(MOTOR_IN1, LOW);  digitalWrite(MOTOR_IN2, HIGH);
    digitalWrite(MOTOR_IN3, HIGH); digitalWrite(MOTOR_IN4, LOW);

  } else if (vehicle.direction == "RIGHT") {
    analogWrite(MOTOR_ENB, 80);
    digitalWrite(MOTOR_IN1, HIGH); digitalWrite(MOTOR_IN2, LOW);
    digitalWrite(MOTOR_IN3, LOW);  digitalWrite(MOTOR_IN4, HIGH);

  } else {
    stopMotors();
  }
}

void stopMotors() {
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  digitalWrite(MOTOR_IN3, LOW);
  digitalWrite(MOTOR_IN4, LOW);
  analogWrite(MOTOR_ENA, 0);
  analogWrite(MOTOR_ENB, 0);
  digitalWrite(LED_MOVE, LOW);
}

// ─────────────────────────────────────────────
//  Data Transmission
// ─────────────────────────────────────────────
void sendDataToServer() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[HTTP] WiFi not connected — skipping send");
    return;
  }

  StaticJsonDocument<512> doc;
  doc["latitude"]          = vehicle.latitude;
  doc["longitude"]         = vehicle.longitude;
  doc["speed_kmph"]        = vehicle.speed_kmph;
  doc["heading"]           = vehicle.heading;
  doc["direction"]         = vehicle.direction;
  doc["accel_x"]           = vehicle.accelX;
  doc["accel_y"]           = vehicle.accelY;
  doc["accel_z"]           = vehicle.accelZ;
  doc["gyro_x"]            = vehicle.gyroX;
  doc["gyro_y"]            = vehicle.gyroY;
  doc["gyro_z"]            = vehicle.gyroZ;
  doc["distance_cm"]       = vehicle.distance_cm;
  doc["obstacle_detected"] = vehicle.obstacleDetected;
  doc["timestamp"]         = millis();

  String payload;
  serializeJson(doc, payload);

  http.begin(SERVER_URL);
  http.addHeader("Content-Type", "application/json");

  int httpCode = http.POST(payload);
  if (httpCode == 200) {
    Serial.printf("[HTTP] Data sent OK | Direction: %s\n", vehicle.direction.c_str());
  } else {
    Serial.printf("[HTTP] Send failed — HTTP code: %d\n", httpCode);
  }
  http.end();
}