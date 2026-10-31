#include <Servo.h>

// Recovered development baseline from May 2026; see README for provenance.

Servo servoLeft;
Servo servoRight;

// =============================================================================
// ESSENTIAL — pins, timing, motion, IR read, corridor centering (lab baseline)
// =============================================================================

const int irLedPinLeft = 10;
const int irLedPinMid = 6;
const int irLedPinRight = 2;

const int irReceiverPinLeft = 11;
const int irReceiverPinMid = 7;
const int irReceiverPinRight = 3;

const int redLedPinLeft = A2;
const int redLedPinMid = A1;
const int redLedPinRight = A0;

const int STOP = 1500;
const int MED_CCW = 1600;
const int MED_CW = 1410;
const int SLOW_CCW = 1550;
const int SLOW_CW = 1450;

const int TURN_TIME = 350;
const int PAUSE_TIME = 100;
const int CENTER_DEADBAND = 1;
const int STEER_TRIM_US = 50;

const int UTURN_TIME = 2000;
const unsigned long LONG_END_NO_SIDE_OPEN_MS = 4000;

bool finishReached = false;
unsigned long lastTimeLeftOrRightOpenMs = 0;

void moveForward() {
  servoLeft.writeMicroseconds(MED_CCW);
  servoRight.writeMicroseconds(MED_CW);
}

void stopMotors() {
  servoLeft.writeMicroseconds(STOP);
  servoRight.writeMicroseconds(STOP);
}

void turnRight() {
  servoLeft.writeMicroseconds(SLOW_CCW);
  servoRight.writeMicroseconds(SLOW_CCW);
  delay(TURN_TIME);
  stopMotors();
  delay(PAUSE_TIME);
}

void turnLeft() {
  servoLeft.writeMicroseconds(SLOW_CW);
  servoRight.writeMicroseconds(SLOW_CW);
  delay(TURN_TIME);
  stopMotors();
  delay(PAUSE_TIME);
}

void steerLeft() {
  servoLeft.writeMicroseconds(MED_CCW - STEER_TRIM_US);
  servoRight.writeMicroseconds(MED_CW - STEER_TRIM_US);
}

void steerRight() {
  servoLeft.writeMicroseconds(MED_CCW + STEER_TRIM_US);
  servoRight.writeMicroseconds(MED_CW + STEER_TRIM_US);
}

int irDetect(int irLedPin, int irReceiverPin, long frequency) {
  tone(irLedPin, frequency);
  delayMicroseconds(900);
  int ir = digitalRead(irReceiverPin);
  noTone(irLedPin);
  delayMicroseconds(900);
  return ir;
}

int irDistance(int irLedPin, int irReceiverPin) {
  int distance = 0;
  for (long f = 38000; f <= 42000; f += 500) {
    distance += irDetect(irLedPin, irReceiverPin, f);
  }
  return distance;
}

void centreRobot() {
  int rightDist = irDistance(irLedPinRight, irReceiverPinRight);
  int error = rightDist - 5;

  if (abs(error) <= CENTER_DEADBAND) {
    moveForward();
  } else if (error > 0) {
    steerLeft();
  } else {
    steerRight();
  }
}

void deadEndUTurn() {
  stopMotors();
  delay(PAUSE_TIME);
  servoLeft.writeMicroseconds(SLOW_CCW);
  servoRight.writeMicroseconds(SLOW_CCW);
  delay(UTURN_TIME);
  stopMotors();
  delay(PAUSE_TIME);
}

void updateLongEndSideOpenTimer(bool deadEnd, int leftVal, int rightVal) {
  if (!deadEnd && (leftVal == 1 || rightVal == 1)) {
    lastTimeLeftOrRightOpenMs = millis();
  }
}

bool isLongDeadEndByCorridorTime() {
  return (millis() - lastTimeLeftOrRightOpenMs) >= LONG_END_NO_SIDE_OPEN_MS;
}

void setup() {
  delay(5000);

  servoLeft.attach(13);
  servoRight.attach(12);

  pinMode(irReceiverPinLeft, INPUT);
  pinMode(irReceiverPinMid, INPUT);
  pinMode(irReceiverPinRight, INPUT);

  pinMode(irLedPinLeft, OUTPUT);
  pinMode(irLedPinMid, OUTPUT);
  pinMode(irLedPinRight, OUTPUT);

  pinMode(redLedPinLeft, OUTPUT);
  pinMode(redLedPinMid, OUTPUT);
  pinMode(redLedPinRight, OUTPUT);

  lastTimeLeftOrRightOpenMs = millis();
  moveForward();
}

void loop() {
  if (finishReached) {
    stopMotors();
    return;
  }

  int leftVal = irDetect(irLedPinLeft, irReceiverPinLeft, 38000);
  int midVal = irDetect(irLedPinMid, irReceiverPinMid, 38000);
  int rightVal = irDetect(irLedPinRight, irReceiverPinRight, 38000);

  digitalWrite(redLedPinLeft, leftVal == 0 ? HIGH : LOW);
  digitalWrite(redLedPinMid, midVal == 0 ? HIGH : LOW);
  digitalWrite(redLedPinRight, rightVal == 0 ? HIGH : LOW);

  const bool deadEnd = (leftVal == 0 && midVal == 0 && rightVal == 0);
  updateLongEndSideOpenTimer(deadEnd, leftVal, rightVal);

  // Priority: dead end → right open (even if mid open) → front blocked → straight centering
  if (deadEnd) {
    if (isLongDeadEndByCorridorTime()) {
      finishReached = true;
      stopMotors();
    } else {
      deadEndUTurn();
    }
  } else if (rightVal == 1) {
    stopMotors();
    delay(PAUSE_TIME);
    turnRight();
    moveForward();
  } else if (midVal == 0) {
    stopMotors();
    delay(PAUSE_TIME);
    if (leftVal == 1) {
      turnLeft();
      moveForward();
    } else {
      stopMotors();
    }
  } else {
    centreRobot();
  }

  delay(10);
}
