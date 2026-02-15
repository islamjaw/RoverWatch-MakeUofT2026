// ==========================================
//   ARDUINO ROVER - AGGRESSIVE STABILIZATION
// ==========================================
#include <Wire.h>
#include <MPU6050_light.h>

// --- 1. PIN DEFINITIONS ---
const int motorLeft_A = A2;
const int motorLeft_B = A3;
const int motorRight_A = 8; 
const int motorRight_B = 9;
const int trigPin = 6;
const int echoPin = 7;

// --- 2. TUNING VARIABLES ---
int speedPulse = 35;
float motorLeftTrim = 1.0;   
float motorRightTrim = 1.3;  // Adjust this based on your 't' test
float Kp = 6.0;              // DOUBLED for much stronger correction
float Ki = 0.3;              // NEW: Integral term to fix persistent drift
float cmPerPulse = 0.8;
int stopDistance = 40;
int warningDistance = 60;

// --- 3. MISSION TARGET ---
float missionX = 400.0;
float missionY = 100.0;

// --- 4. STATE VARIABLES ---
MPU6050 mpu(Wire);
char currentState = 'x'; 
float targetAngle = 0;
float posX = 0, posY = 0;
unsigned long lastObstacleCheck = 0;
bool isMovingForward = false;
float errorIntegral = 0;  // NEW: Accumulates error over time

void setup() {
  Serial.begin(9600);
  Wire.begin();
  
  pinMode(motorLeft_A, OUTPUT); pinMode(motorLeft_B, OUTPUT);
  pinMode(motorRight_A, OUTPUT); pinMode(motorRight_B, OUTPUT);
  pinMode(trigPin, OUTPUT); pinMode(echoPin, INPUT);
  
  stopMotors();
  Serial.println("--- CALIBRATING GYRO ---");
  mpu.begin();
  delay(1000);
  mpu.calcOffsets(); 
  Serial.println("--- READY ---");
  Serial.println("'g'=Mission | 'x'=Stop | 't'=Test | 'p'=Position");
}

void loop() {
  mpu.update();

  // --- MISSION LOGIC ---
  if (currentState == 'g') {
    float dx = missionX - posX;
    float dy = missionY - posY;
    float distRemaining = sqrt(sq(dx) + sq(dy));

    if (distRemaining < 10.0) {
      Serial.println("--- TARGET REACHED! ---");
      stopMotors();
      currentState = 'x';
      isMovingForward = false;
      errorIntegral = 0;
    } 
    else {
      targetAngle = atan2(dy, dx) * (180.0 / PI);
      
      // OBSTACLE CHECK
      if (millis() - lastObstacleCheck > 150) {
        lastObstacleCheck = millis();
        long dist = getDistance();
        
        if (dist > 2 && dist < stopDistance) {
          Serial.println("OBSTACLE! Scanning...");
          stopMotors();
          isMovingForward = false;
          errorIntegral = 0;  // Reset integral on obstacle
          performEnhancedDodge();
        } 
        else if (dist > 2 && dist < warningDistance) {
          speedPulse = 25;
        }
        else {
          speedPulse = 35;
        }
      }
      
      // DRIVE WITH STABILIZATION
      driveStabilizedPID(targetAngle);
    }
  } 
  else if (currentState == 't') {
    // TEST MODE
    driveStabilizedPID(targetAngle);
  }
  else {
    isMovingForward = false;
    errorIntegral = 0;
  }

  // --- KEYBOARD INPUT ---
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    if (cmd == 'g') { 
      currentState = 'g'; 
      posX = 0; posY = 0;
      errorIntegral = 0;
      targetAngle = mpu.getAngleZ();
      Serial.println("Mission Active!"); 
    }
    else if (cmd == 'x') { 
      currentState = 'x'; 
      stopMotors(); 
      isMovingForward = false;
      errorIntegral = 0;
      speedPulse = 35; 
    }
    else if (cmd == 't') {
      currentState = 't';
      targetAngle = mpu.getAngleZ();
      errorIntegral = 0;
      Serial.print("TEST: Target heading = ");
      Serial.println(targetAngle);
      Serial.println("Driving for 5 sec...");
      unsigned long testStart = millis();
      while (millis() - testStart < 5000) {
        mpu.update();
        driveStabilizedPID(targetAngle);
      }
      currentState = 'x';
      stopMotors();
      Serial.print("Final angle: "); 
      Serial.print(mpu.getAngleZ());
      Serial.print(" (drift: ");
      Serial.print(mpu.getAngleZ() - targetAngle);
      Serial.println("°)");
    }
    else if (cmd == 'p') {
      Serial.print("Pos: ["); Serial.print(posX, 1); 
      Serial.print(", "); Serial.print(posY, 1); 
      Serial.print("] Angle: "); Serial.println(mpu.getAngleZ(), 1);
    }
    else if (cmd == '+') {
      motorRightTrim += 0.05;
      Serial.print("Right trim: "); Serial.println(motorRightTrim, 2);
    }
    else if (cmd == '-') {
      motorRightTrim -= 0.05;
      Serial.print("Right trim: "); Serial.println(motorRightTrim, 2);
    }
  }
}

// ==========================================
//      ENHANCED OBSTACLE AVOIDANCE
// ==========================================
void performEnhancedDodge() {
  Serial.println("=== SCANNING ===");
  
  float originalAngle = mpu.getAngleZ();
  
  // LOOK LEFT
  turnToAngle(originalAngle + 45, 25);
  delay(150);
  long distLeft = getDistance();
  Serial.print("Left: "); Serial.print(distLeft); Serial.println("cm");
  delay(50);
  
  // LOOK RIGHT
  turnToAngle(originalAngle - 45, 25);
  delay(150);
  long distRight = getDistance();
  Serial.print("Right: "); Serial.print(distRight); Serial.println("cm");
  delay(50);
  
  // RETURN TO ORIGINAL
  turnToAngle(originalAngle, 25);
  delay(150);
  
  // DECIDE PATH
  if (distLeft > distRight && distLeft > stopDistance) {
    Serial.println("-> LEFT");
    driveRawTracked(false, 300);
    turnToAngle(mpu.getAngleZ() + 60, 30);
  } 
  else if (distRight > distLeft && distRight > stopDistance) {
    Serial.println("-> RIGHT");
    driveRawTracked(false, 300);
    turnToAngle(mpu.getAngleZ() - 60, 30);
  }
  else if (distLeft > 20 || distRight > 20) {
    Serial.println("-> PARTIAL");
    driveRawTracked(false, 400);
    if (distLeft > distRight) {
      turnToAngle(mpu.getAngleZ() + 70, 30);
    } else {
      turnToAngle(mpu.getAngleZ() - 70, 30);
    }
  }
  else {
    Serial.println("-> 180 TURN");
    driveRawTracked(false, 500);
    turnToAngle(mpu.getAngleZ() + 180, 40);
  }
  
  // Clear obstacle
  for(int i = 0; i < 40; i++) {
    driveRawTracked(true, 50);
    if (i % 10 == 0) {
      if (getDistance() > stopDistance * 1.5) break;
    }
  }
  
  stopMotors();
  errorIntegral = 0;  // Reset after dodge
  delay(100);
}

// ==========================================
//      PRECISE ANGLE TURNING
// ==========================================
void turnToAngle(float targetA, int turnSpeed) {
  mpu.update();
  float current = mpu.getAngleZ();
  float error = targetA - current;
  
  while (error > 180) error -= 360;
  while (error < -180) error += 360;
  
  int maxIterations = 150;
  int iterations = 0;
  
  while (abs(error) > 2.0 && iterations < maxIterations) {
    mpu.update();
    current = mpu.getAngleZ();
    error = targetA - current;
    
    while (error > 180) error -= 360;
    while (error < -180) error += 360;
    
    // Variable turn speed based on error
    int currentTurnSpeed = turnSpeed;
    if (abs(error) < 10) currentTurnSpeed = turnSpeed / 2;  // Slow down near target
    
    if (error > 0) {
      digitalWrite(motorLeft_A, LOW); digitalWrite(motorLeft_B, HIGH);
      digitalWrite(motorRight_A, HIGH); digitalWrite(motorRight_B, LOW);
    } else {
      digitalWrite(motorLeft_A, HIGH); digitalWrite(motorLeft_B, LOW);
      digitalWrite(motorRight_A, LOW); digitalWrite(motorRight_B, HIGH);
    }
    
    delay(currentTurnSpeed);
    stopMotors();
    delay(10);
    iterations++;
  }
  
  stopMotors();
}

// ==========================================
//   PID STABILIZED DRIVE (More Aggressive)
// ==========================================
void driveStabilizedPID(float goal) {
  float currentAngle = mpu.getAngleZ();
  float error = goal - currentAngle;
  
  // Normalize error
  while (error > 180) error -= 360;
  while (error < -180) error += 360;
  
  // Accumulate error for integral term
  errorIntegral += error;
  errorIntegral = constrain(errorIntegral, -50, 50);  // Prevent windup
  
  // PID correction: Proportional + Integral
  float correction = (Kp * error) + (Ki * errorIntegral);
  
  // Base speeds with motor trim
  float leftP = speedPulse * motorLeftTrim;
  float rightP = speedPulse * motorRightTrim;

  // Apply correction (positive error = need to turn left)
  if (correction > 0) {
    leftP -= abs(correction);
  } else {
    rightP -= abs(correction);
  }
  
  // Constrain to safe ranges
  leftP = constrain(leftP, 10, (speedPulse + 25) * motorLeftTrim);
  rightP = constrain(rightP, 10, (speedPulse + 25) * motorRightTrim);

  // ACTIVATE MOTORS
  digitalWrite(motorLeft_A, HIGH); digitalWrite(motorLeft_B, LOW);
  digitalWrite(motorRight_A, HIGH); digitalWrite(motorRight_B, LOW);
  isMovingForward = true;
  
  int activeDuration = min(leftP, rightP);
  delay(activeDuration);
  
  // UPDATE POSITION
  updatePositionFromMovement(activeDuration);
  
  // Differential timing
  if (leftP < rightP) {
    digitalWrite(motorLeft_A, LOW); digitalWrite(motorLeft_B, LOW);
    delay(rightP - leftP);
    digitalWrite(motorRight_A, LOW); digitalWrite(motorRight_B, LOW);
  } else {
    digitalWrite(motorRight_A, LOW); digitalWrite(motorRight_B, LOW);
    delay(leftP - rightP);
    digitalWrite(motorLeft_A, LOW); digitalWrite(motorLeft_B, LOW);
  }
  
  isMovingForward = false;
  delay(5);  // Reduced delay for faster response
}

// ==========================================
//   POSITION UPDATE
// ==========================================
// Inside RoverMapper.ino - updatePositionFromMovement function [cite: 171, 172, 175]
void updatePositionFromMovement(int pulseDuration) {
  if (!isMovingForward) return;
  float angleRad = mpu.getAngleZ() * (PI / 180.0);
  float distMoved = cmPerPulse * (pulseDuration / 10.0);
  posX += distMoved * cos(angleRad);
  posY += distMoved * sin(angleRad);
  
  // ADD THIS LINE: Send data to laptop via Serial 
  Serial.print("COORD:"); Serial.print(posX); Serial.print(","); Serial.println(posY);
}

// ==========================================
//   TRACKED MOVEMENT
// ==========================================
void driveRawTracked(bool fwd, int t) {
  float leftDuration = t * motorLeftTrim;
  float rightDuration = t * motorRightTrim;
  
  if(fwd) { 
    digitalWrite(motorLeft_A, HIGH); digitalWrite(motorLeft_B, LOW); 
    digitalWrite(motorRight_A, HIGH); digitalWrite(motorRight_B, LOW);
    
    delay(min(leftDuration, rightDuration));
    
    if (leftDuration < rightDuration) {
      digitalWrite(motorLeft_A, LOW); digitalWrite(motorLeft_B, LOW);
      delay(rightDuration - leftDuration);
      digitalWrite(motorRight_A, LOW); digitalWrite(motorRight_B, LOW);
    } else {
      digitalWrite(motorRight_A, LOW); digitalWrite(motorRight_B, LOW);
      delay(leftDuration - rightDuration);
      digitalWrite(motorLeft_A, LOW); digitalWrite(motorLeft_B, LOW);
    }
    
    float angleRad = mpu.getAngleZ() * (PI / 180.0);
    float distMoved = cmPerPulse * (t / 10.0);
    posX += distMoved * cos(angleRad);
    posY += distMoved * sin(angleRad);
  }
  else { 
    digitalWrite(motorLeft_A, LOW); digitalWrite(motorLeft_B, HIGH); 
    digitalWrite(motorRight_A, LOW); digitalWrite(motorRight_B, HIGH);
    
    delay(min(leftDuration, rightDuration));
    
    if (leftDuration < rightDuration) {
      digitalWrite(motorLeft_A, LOW); digitalWrite(motorLeft_B, LOW);
      delay(rightDuration - leftDuration);
      digitalWrite(motorRight_A, LOW); digitalWrite(motorRight_B, LOW);
    } else {
      digitalWrite(motorRight_A, LOW); digitalWrite(motorRight_B, LOW);
      delay(leftDuration - rightDuration);
      digitalWrite(motorLeft_A, LOW); digitalWrite(motorLeft_B, LOW);
    }
    
    float angleRad = mpu.getAngleZ() * (PI / 180.0);
    float distMoved = cmPerPulse * (t / 10.0);
    posX -= distMoved * cos(angleRad);
    posY -= distMoved * sin(angleRad);
  }
  
  mpu.update();
}

// ==========================================
//          BASIC HELPERS
// ==========================================
long getDistance() {
  digitalWrite(trigPin, LOW); 
  delayMicroseconds(2); 
  digitalWrite(trigPin, HIGH); 
  delayMicroseconds(10); 
  digitalWrite(trigPin, LOW);
  long d = pulseIn(echoPin, HIGH, 15000); 
  if (d == 0) return 999; 
  return d * 0.034 / 2;
}

void stopMotors() { 
  digitalWrite(motorLeft_A, LOW); digitalWrite(motorLeft_B, LOW); 
  digitalWrite(motorRight_A, LOW); digitalWrite(motorRight_B, LOW);
  isMovingForward = false;
}