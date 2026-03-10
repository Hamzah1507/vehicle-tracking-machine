/*
 * ============================================================
 *  motor_control.h  —  L298N Motor Driver Helper
 *  Vehicle Tracking Machine
 *  Author: Hamzah Saiyed
 * ============================================================
 */

#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>

// ─────────────────────────────────────────────
//  Motor Driver Configuration
// ─────────────────────────────────────────────
struct MotorConfig {
  uint8_t in1, in2;   // Motor A direction pins
  uint8_t in3, in4;   // Motor B direction pins
  uint8_t ena;         // Motor A PWM enable
  uint8_t enb;         // Motor B PWM enable
  uint8_t defaultSpeed;
};

class MotorController {
  private:
    MotorConfig cfg;
    uint8_t currentSpeedA;
    uint8_t currentSpeedB;
    String  currentDirection;
    bool    isRunning;

  public:
    // ── Constructor ──────────────────────────
    MotorController(MotorConfig config) : cfg(config),
      currentSpeedA(0), currentSpeedB(0),
      currentDirection("STOP"), isRunning(false) {}

    // ── Initialise pins ──────────────────────
    void begin() {
      pinMode(cfg.in1, OUTPUT); pinMode(cfg.in2, OUTPUT);
      pinMode(cfg.in3, OUTPUT); pinMode(cfg.in4, OUTPUT);
      pinMode(cfg.ena, OUTPUT); pinMode(cfg.enb, OUTPUT);
      stop();
      Serial.println("[MOTOR] Controller initialised");
    }

    // ── Move Forward ─────────────────────────
    void forward(uint8_t speed = 0) {
      if (speed == 0) speed = cfg.defaultSpeed;
      setSpeedA(speed);
      setSpeedB(speed);
      digitalWrite(cfg.in1, HIGH); digitalWrite(cfg.in2, LOW);
      digitalWrite(cfg.in3, HIGH); digitalWrite(cfg.in4, LOW);
      currentDirection = "FORWARD";
      isRunning = true;
      Serial.printf("[MOTOR] FORWARD  | Speed: %d\n", speed);
    }

    // ── Move Backward ────────────────────────
    void backward(uint8_t speed = 0) {
      if (speed == 0) speed = cfg.defaultSpeed;
      setSpeedA(speed);
      setSpeedB(speed);
      digitalWrite(cfg.in1, LOW);  digitalWrite(cfg.in2, HIGH);
      digitalWrite(cfg.in3, LOW);  digitalWrite(cfg.in4, HIGH);
      currentDirection = "BACKWARD";
      isRunning = true;
      Serial.printf("[MOTOR] BACKWARD | Speed: %d\n", speed);
    }

    // ── Turn Left (pivot) ────────────────────
    void turnLeft(uint8_t speed = 0) {
      if (speed == 0) speed = cfg.defaultSpeed;
      setSpeedA(speed / 2);   // slow left motor
      setSpeedB(speed);
      digitalWrite(cfg.in1, LOW);  digitalWrite(cfg.in2, HIGH);
      digitalWrite(cfg.in3, HIGH); digitalWrite(cfg.in4, LOW);
      currentDirection = "LEFT";
      isRunning = true;
      Serial.printf("[MOTOR] TURN LEFT | Speed: %d\n", speed);
    }

    // ── Turn Right (pivot) ───────────────────
    void turnRight(uint8_t speed = 0) {
      if (speed == 0) speed = cfg.defaultSpeed;
      setSpeedA(speed);
      setSpeedB(speed / 2);   // slow right motor
      digitalWrite(cfg.in1, HIGH); digitalWrite(cfg.in2, LOW);
      digitalWrite(cfg.in3, LOW);  digitalWrite(cfg.in4, HIGH);
      currentDirection = "RIGHT";
      isRunning = true;
      Serial.printf("[MOTOR] TURN RIGHT | Speed: %d\n", speed);
    }

    // ── Hard Stop ────────────────────────────
    void stop() {
      setSpeedA(0);
      setSpeedB(0);
      digitalWrite(cfg.in1, LOW); digitalWrite(cfg.in2, LOW);
      digitalWrite(cfg.in3, LOW); digitalWrite(cfg.in4, LOW);
      currentDirection = "STOP";
      isRunning = false;
    }

    // ── Brake (active stop) ──────────────────
    void brake() {
      digitalWrite(cfg.in1, HIGH); digitalWrite(cfg.in2, HIGH);
      digitalWrite(cfg.in3, HIGH); digitalWrite(cfg.in4, HIGH);
      delay(100);
      stop();
      Serial.println("[MOTOR] BRAKE applied");
    }

    // ── Getters ──────────────────────────────
    String getDirection() { return currentDirection; }
    bool   running()      { return isRunning; }
    uint8_t getSpeedA()   { return currentSpeedA; }
    uint8_t getSpeedB()   { return currentSpeedB; }

    // ── Set individual motor speeds ──────────
    void setSpeedA(uint8_t speed) {
      currentSpeedA = speed;
      analogWrite(cfg.ena, speed);
    }

    void setSpeedB(uint8_t speed) {
      currentSpeedB = speed;
      analogWrite(cfg.enb, speed);
    }

    // ── Smooth acceleration ramp ─────────────
    void rampToSpeed(uint8_t targetSpeed, uint16_t durationMs = 500) {
      uint8_t steps   = 20;
      uint8_t current = min(currentSpeedA, currentSpeedB);
      int16_t delta   = (targetSpeed - current) / steps;
      uint16_t delayMs = durationMs / steps;

      for (uint8_t i = 0; i < steps; i++) {
        current += delta;
        setSpeedA(current);
        setSpeedB(current);
        delay(delayMs);
      }
      setSpeedA(targetSpeed);
      setSpeedB(targetSpeed);
    }

    // ── Status dump to serial ────────────────
    void printStatus() {
      Serial.printf("[MOTOR STATUS] Dir: %-10s | SpeedA: %3d | SpeedB: %3d | Running: %s\n",
                    currentDirection.c_str(), currentSpeedA, currentSpeedB,
                    isRunning ? "YES" : "NO");
    }
};

#endif // MOTOR_CONTROL_H