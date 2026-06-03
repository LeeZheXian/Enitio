#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- PIN DEFINITIONS ---
#define VRX_PIN    33 
#define VRY_PIN    26 
#define servoX_PIN 13
#define servoY_PIN 12
#define HOLE1_PIN  15
#define HOLE2_PIN  2 
#define HOLE3_PIN  4
#define HOLE4_PIN  16

// --- OLED SETUP ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- CALIBRATION VARIABLES (Values will be set during Setup) ---
int xRawIdle = 0; 
int yRawIdle = 0; 
int xServoCenter = 92;
int yServoCenter = 103;

// --- GAME & MOVEMENT SETTINGS ---
int rawDeadzone = 80; 
float maxSpeed = 0.8; 
int sensitivity = 150; // Using your successful detection threshold

// --- GLOBAL VARIABLES ---
Servo servo_x, servo_y;
float currentX = xServoCenter;
float currentY = yServoCenter;
int baseline1, baseline2, baseline3, baseline4;
int score = 0;
int targetHole = 0;

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    for(;;);
  }

  // --- START AUTO-CALIBRATION ---
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("CALIBRATING...");
  display.println("RELEASE JOYSTICK!");
  display.display();

  // 1. Calculate Joystick Idle (Average 50 samples)
  long xSum = 0, ySum = 0;
  for(int i = 0; i < 50; i++) {
    xSum += analogRead(VRX_PIN);
    ySum += analogRead(VRY_PIN);
    delay(20);
  }
  xRawIdle = xSum / 50;
  yRawIdle = ySum / 50;

  Serial.print("Calibrated X Idle: "); Serial.println(xRawIdle);
  Serial.print("Calibrated Y Idle: "); Serial.println(yRawIdle);

  // 2. Attach Servos & Reset to Center
  servo_x.attach(servoX_PIN);
  servo_y.attach(servoY_PIN);
  servo_x.write(xServoCenter);
  servo_y.write(yServoCenter);

  // 3. Calibrate LDR Baselines for the 4 holes
  baseline1 = calibrateHole(HOLE1_PIN);
  baseline2 = calibrateHole(HOLE2_PIN);
  baseline3 = calibrateHole(HOLE3_PIN);
  baseline4 = calibrateHole(HOLE4_PIN);

  // Pick first random target
  targetHole = random(1, 5); 
  updateDisplay();
}

void loop() {
  // --- 1. PRIORITY SENSOR CHECK ---
  // We poll the LDR multiple times to catch the fast steel ball
  bool hitTarget = false;
  for(int i=0; i<10; i++) {
    if (targetHole == 1) hitTarget = (analogRead(HOLE1_PIN) < (baseline1 - sensitivity));
    else if (targetHole == 2) hitTarget = (analogRead(HOLE2_PIN) < (baseline2 - sensitivity));
    else if (targetHole == 3) hitTarget = (analogRead(HOLE3_PIN) < (baseline3 - sensitivity));
    else if (targetHole == 4) hitTarget = (analogRead(HOLE4_PIN) < (baseline4 - sensitivity));
    
    if(hitTarget) break;
    delay(1); 
  }

  if (hitTarget) {
    score++;
    displaySuccess();
    
    int nextHole;
    do { nextHole = random(1, 5); } while (nextHole == targetHole);
    targetHole = nextHole;
    
    updateDisplay();
  }

  // --- 2. JOYSTICK & SERVO LOGIC ---
  int xRaw = analogRead(VRX_PIN);
  int yRaw = analogRead(VRY_PIN);
  int targetX, targetY;

  // X Mapping +/- 30 degrees from Auto-Calibrated center
  if (abs(xRaw - xRawIdle) <= rawDeadzone) targetX = xServoCenter;
  else if (xRaw < xRawIdle) targetX = map(xRaw, 0, xRawIdle, xServoCenter - 30, xServoCenter);
  else targetX = map(xRaw, xRawIdle, 4095, xServoCenter, xServoCenter + 30);

  // Y Mapping +/- 30 degrees from Auto-Calibrated center
  if (abs(yRaw - yRawIdle) <= rawDeadzone) targetY = yServoCenter;
  else if (yRaw < yRawIdle) targetY = map(yRaw, 0, yRawIdle, yServoCenter - 30, yServoCenter);
  else targetY = map(yRaw, yRawIdle, 4095, yServoCenter, yServoCenter + 30);

  // Unified Smooth Movement (Speed Limited)
  float diffX = targetX - currentX;
  float diffY = targetY - currentY;

  if (abs(diffX) > maxSpeed) currentX += (diffX > 0) ? maxSpeed : -maxSpeed;
  else currentX = targetX;

  if (abs(diffY) > maxSpeed) currentY += (diffY > 0) ? maxSpeed : -maxSpeed;
  else currentY = targetY;

  servo_x.write((int)currentX);
  servo_y.write((int)currentY);

  delay(5); // Fast loop to keep sensors sharp
}

// --- HELPER FUNCTIONS ---

int calibrateHole(int pin) {
  long sum = 0;
  for(int i=0; i<15; i++) {
    sum += analogRead(pin);
    delay(10);
  }
  return sum / 15;
}

void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("SCORE: "); display.print(score);
  display.setTextSize(2);
  display.setCursor(0, 25);
  display.print("TARGET: ");
  display.setTextSize(4);
  display.print(targetHole);
  display.display();
}

void displaySuccess() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(15, 25);
  display.println("GOAL!!");
  display.display();
  delay(800); 
}