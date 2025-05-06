#include <Servo.h>
#include <SoftwareSerial.h>

SoftwareSerial espSerial(0, 1); // RX = 0, TX = 1 (match ESP TX & RX)

#define enA 6
#define in1 7
#define in2 8
#define in3 9
#define in4 10
#define enB 5

#define L_S 12
#define R_S 11
#define echo A2
#define trigger A3
#define servoPin A5

#define SERVO_FRONT 70
#define SERVO_LEFT 0
#define SERVO_RIGHT 140

Servo myServo;
int mode = 2;  // Default to Web Control Mode
int motorSpeed = 255;
int Set = 15;
bool obstacleDetected = false;
int distance_L, distance_F, distance_R;
unsigned long lastCommandTime = 0;

void setup() {
  espSerial.begin(9600);

  pinMode(R_S, INPUT);
  pinMode(L_S, INPUT);
  pinMode(echo, INPUT);
  pinMode(trigger, OUTPUT);

  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(enB, OUTPUT);

  myServo.attach(servoPin);
  setMotorSpeed(motorSpeed);
  scanServo();
  distance_F = Ultrasonic_read();
  delay(500);
}

void loop() {
  if (espSerial.available()) {
    char command = espSerial.read();
    lastCommandTime = millis();

    if (command == '1') mode = 1;
    else if (command == '2') mode = 2;
    else if (command == 'V') {
      while (!espSerial.available());
      motorSpeed = espSerial.read();
      setMotorSpeed(motorSpeed);
    } else if (mode == 2) {
      controlFromWeb(command);
    }
  }

  if (mode == 2) {
    distance_F = Ultrasonic_read();
    obstacleDetected = (distance_F <= Set);

    // 🚀 Only stop if moving forward, NOT when moving backward!
    if (obstacleDetected && !isMovingBackward()) {
      stopMotors();
    }
  }

  if (mode == 1) autonomousMode();
  else if (mode == 2 && millis() - lastCommandTime > 7000) {
    stopMotors();
  }
}

// ✅ **Always allow B (backward)**
void controlFromWeb(char cmd) {
  if (cmd == 'B') {
    moveBackward(); // ✅ Always allow moving backward
    return;
  }

  // 🚀 Stop only F when an obstacle is detected
  if (cmd == 'F' && obstacleDetected) {
    stopMotors();
    return;
  }

  switch (cmd) {
    case 'F': moveForward(); break;
    case 'L': turnLeft(); break;
    case 'R': turnRight(); break;
    case 'S': stopMotors(); break;
  }
}




void autonomousMode() {
  distance_F = Ultrasonic_read();
  bool rightSensor = digitalRead(R_S);
  bool leftSensor = digitalRead(L_S);

  if (rightSensor && leftSensor) {   // Both sensors HIGH
    stopMotors();
  } else if (!rightSensor && !leftSensor) {  // Both LOW
    if (distance_F > Set) {
      moveForward();
    } else {
      checkSides();
    }
  } else if (rightSensor) {  // Only right sensor HIGH
    turnRight();
  } else if (leftSensor) {   // Only left sensor HIGH
    turnLeft();
  }
  delay(10);
}

void scanServo() {
  for (int angle = SERVO_LEFT; angle <= SERVO_RIGHT; angle += 10) {
    myServo.write(angle);
    delay(30);
  }
  for (int angle = SERVO_RIGHT; angle >= SERVO_FRONT; angle -= 10) {
    myServo.write(angle);
    delay(30);
  }
}

long Ultrasonic_read() {
  digitalWrite(trigger, LOW); delayMicroseconds(2);
  digitalWrite(trigger, HIGH); delayMicroseconds(10);
  digitalWrite(trigger, LOW);
  long duration = pulseIn(echo, HIGH, 30000);
  if (duration == 0) return 50;
  long distance = duration / 29 / 2;
  return (distance > 200) ? 200 : distance; // Cap max distance to 200cm
}

void checkSides() {
  stopMotors();
  delay(100);

  // Check right side
  myServo.write(SERVO_RIGHT);
  delay(700);
  distance_R = Ultrasonic_read();
  delay(100);

  // Check left side
  myServo.write(SERVO_LEFT);
  delay(700);
  distance_L = Ultrasonic_read();
  delay(100);

   // Reset servo to front
  myServo.write(SERVO_FRONT);
  delay(300);
  // Check front again
  distance_F = Ultrasonic_read();
  if (distance_F > Set) {
    moveForward();
    return;
  }

 

  // If both sides are blocked, stop
  else if (distance_R < 35 && distance_L < 35) {
    stopMotors();
  } 
  // Choose the better side
  else {
    compareDistance();
    
  } 
}

void compareDistance() {
  setMotorSpeed(motorSpeed);
  if (distance_R > distance_L) {
    turnLeft(); delay(300);
    moveForward(); delay(400);
    turnRight(); delay(300);
    moveForward(); delay(400);
    turnRight(); delay(200);
  } else {
    turnRight(); delay(300);
    moveForward(); delay(400);
    turnLeft(); delay(300);
    moveForward(); delay(400);
    turnLeft(); delay(200);
  }
}

void setMotorSpeed(int speed) {
  analogWrite(enA, speed);
  analogWrite(enB, speed);
}

bool isMovingBackward() {
  return digitalRead(in1) == HIGH && digitalRead(in2) == LOW &&
         digitalRead(in3) == LOW && digitalRead(in4) == HIGH;
}


void moveForward() { digitalWrite(in1, LOW); digitalWrite(in2, HIGH); digitalWrite(in3, HIGH); digitalWrite(in4, LOW); }
void moveBackward() { digitalWrite(in1, HIGH); digitalWrite(in2, LOW); digitalWrite(in3, LOW); digitalWrite(in4, HIGH); }
void turnRight() { digitalWrite(in1, LOW); digitalWrite(in2, HIGH); digitalWrite(in3, LOW); digitalWrite(in4, HIGH); }
void turnLeft() { digitalWrite(in1, HIGH); digitalWrite(in2, LOW); digitalWrite(in3, HIGH); digitalWrite(in4, LOW); }
void stopMotors() { digitalWrite(in1, LOW); digitalWrite(in2, LOW); digitalWrite(in3, LOW); digitalWrite(in4, LOW); }