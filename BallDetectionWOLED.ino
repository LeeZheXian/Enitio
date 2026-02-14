#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define LIGHT_SENSOR_PIN 34
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


int baseline = 0;
int sensitivity = 150; 
int point = 0;

bool hole1 = false;
bool hole2 = false;
bool hole3 = false;

void setup() {
  Serial.begin(115200);
  analogSetAttenuation(ADC_11db);

  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 10);
  display.println("Calibrating");
  display.display();
  Serial.println("Calibrating...");
  delay(1000);
  
  // Take an average for the baseline
  for(int i=0; i<10; i++) {
    baseline += analogRead(LIGHT_SENSOR_PIN);
    delay(10);
  }
  baseline /= 10;
  Serial.print("Baseline set to: ");
  Serial.println(baseline);

  updateDisplay();
}

void loop() {

  hole1 = ballDetection();
  // hole2 = ballDetection();
  // hole3 =ballDetection(); 

  if(hole1 == true){
    point += 1;
  }

  else if(hole2 == true){
    point += 1;
  }

  else if(hole3 == true){
    point += 1;
  }

  else{
    Serial.println("");
  }

  updateDisplay();
  
}

bool ballDetection(void) {

  int currentVal = analogRead(LIGHT_SENSOR_PIN);

  // If the value drops just a little bit below the baseline...
  if (currentVal < (baseline - sensitivity)) {
    delay(50); 

    return true;
  }

  else {
    return false;
  }

}

void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Status: RUNNING");
  
  display.setTextSize(3);
  display.setCursor(20, 25);
  display.print("PT: ");
  display.print(point);
  
  display.display();
}
