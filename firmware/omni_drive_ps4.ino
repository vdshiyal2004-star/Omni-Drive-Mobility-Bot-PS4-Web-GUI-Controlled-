/*
 * Omni-Directional 4-Wheel Robot — PS4 Controller Firmware
 * Board: ESP32
 * Controller: PS4 DualShock 4 (via PS4Controller library, Bluetooth)
 *
 * Left joystick drives the omni-wheel velocity equations (holonomic movement).
 * L2/R2 triggers give analog forward/reverse throttle.
 * D-pad gives fixed-speed strafe/rotate presets.
 * Circle = all motors forward, Cross = stop.
 */

#include <PS4Controller.h>

// ---- Motor pin mapping ----
// Motor 1
#define M1_SPEED 15
#define M1_DIR   2
// Motor 2
#define M2_SPEED 5
#define M2_DIR   18
// Motor 3
#define M3_SPEED 19
#define M3_DIR   21
// Motor 4
#define M4_SPEED 22
#define M4_DIR   23

// Joystick values below this are treated as noise/dead zone
const int JOYSTICK_DEADZONE = 10;

void setup() {
  Serial.begin(9600);
  PS4.begin();
  Serial.println("Waiting for PS4 controller...");

  pinMode(M1_SPEED, OUTPUT);
  pinMode(M1_DIR, OUTPUT);
  pinMode(M2_SPEED, OUTPUT);
  pinMode(M2_DIR, OUTPUT);
  pinMode(M3_SPEED, OUTPUT);
  pinMode(M3_DIR, OUTPUT);
  pinMode(M4_SPEED, OUTPUT);
  pinMode(M4_DIR, OUTPUT);
}

void loop() {
  if (!PS4.isConnected()) {
    return;
  }

  int x = PS4.LStickX();
  int y = PS4.LStickY();

  if (abs(x) > JOYSTICK_DEADZONE || abs(y) > JOYSTICK_DEADZONE) {
    driveOmni(x, y);
  } else {
    stopAllMotors();
    handleTriggers();
    handleDpad();
    handleFaceButtons();
  }
}

// Holonomic drive: convert joystick x/y into 4 wheel speeds
void driveOmni(int x, int y) {
  float motorSpeed1 = 0.3536 * (x - y);
  float motorSpeed2 = 0.3536 * (x + y);
  float motorSpeed3 = 0.3536 * (-x + y);
  float motorSpeed4 = 0.3536 * (-x - y);

  setMotor(M1_DIR, M1_SPEED, motorSpeed1 >= 0 ? HIGH : LOW, convertSpeed(motorSpeed1));
  setMotor(M2_DIR, M2_SPEED, motorSpeed2 >= 0 ? HIGH : LOW, convertSpeed(motorSpeed2));
  setMotor(M3_DIR, M3_SPEED, motorSpeed3 >= 0 ? HIGH : LOW, convertSpeed(motorSpeed3));
  setMotor(M4_DIR, M4_SPEED, motorSpeed4 >= 0 ? HIGH : LOW, convertSpeed(motorSpeed4));
}

void handleTriggers() {
  int r2 = PS4.R2Value();
  if (r2 > 10) {
    setMotor(M1_DIR, M1_SPEED, HIGH, r2);
    setMotor(M2_DIR, M2_SPEED, HIGH, r2);
    setMotor(M3_DIR, M3_SPEED, HIGH, r2);
    setMotor(M4_DIR, M4_SPEED, HIGH, r2);
  }

  int l2 = PS4.L2Value();
  if (l2 > 10) {
    setMotor(M1_DIR, M1_SPEED, LOW, l2);
    setMotor(M2_DIR, M2_SPEED, LOW, l2);
    setMotor(M3_DIR, M3_SPEED, LOW, l2);
    setMotor(M4_DIR, M4_SPEED, LOW, l2);
  }
}

void handleDpad() {
  const int presetSpeed = 100;

  if (PS4.Right()) {
    setMotor(M1_DIR, M1_SPEED, HIGH, presetSpeed);
    setMotor(M4_DIR, M4_SPEED, LOW, presetSpeed);
    setMotor(M3_DIR, M3_SPEED, LOW, presetSpeed);
    setMotor(M2_DIR, M2_SPEED, HIGH, presetSpeed);
  }
  if (PS4.Up()) {
    setMotor(M1_DIR, M1_SPEED, LOW, presetSpeed);
    setMotor(M4_DIR, M4_SPEED, LOW, presetSpeed);
    setMotor(M3_DIR, M3_SPEED, HIGH, presetSpeed);
    setMotor(M2_DIR, M2_SPEED, HIGH, presetSpeed);
  }
  if (PS4.Down()) {
    setMotor(M1_DIR, M1_SPEED, HIGH, presetSpeed);
    setMotor(M4_DIR, M4_SPEED, HIGH, presetSpeed);
    setMotor(M3_DIR, M3_SPEED, LOW, presetSpeed);
    setMotor(M2_DIR, M2_SPEED, LOW, presetSpeed);
  }
  if (PS4.Left()) {
    setMotor(M1_DIR, M1_SPEED, LOW, presetSpeed);
    setMotor(M4_DIR, M4_SPEED, HIGH, presetSpeed);
    setMotor(M3_DIR, M3_SPEED, HIGH, presetSpeed);
    setMotor(M2_DIR, M2_SPEED, LOW, presetSpeed);
  }
}

void handleFaceButtons() {
  if (PS4.Cross()) {
    stopAllMotors();
  }
  if (PS4.Circle()) {
    setMotor(M1_DIR, M1_SPEED, HIGH, 100);
    setMotor(M2_DIR, M2_SPEED, HIGH, 100);
    setMotor(M3_DIR, M3_SPEED, HIGH, 100);
    setMotor(M4_DIR, M4_SPEED, HIGH, 100);
  }
}

void stopAllMotors() {
  analogWrite(M1_SPEED, 0);
  analogWrite(M2_SPEED, 0);
  analogWrite(M3_SPEED, 0);
  analogWrite(M4_SPEED, 0);
}

// Scales a raw velocity-equation output (max ~63.5) into a 0-255 PWM range
int convertSpeed(float speed) {
  return (int)(abs(speed) * 2.5);
}

void setMotor(int dirPin, int speedPin, int direction, int speed) {
  digitalWrite(dirPin, direction);
  analogWrite(speedPin, speed);
}
