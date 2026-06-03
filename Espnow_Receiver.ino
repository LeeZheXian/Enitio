#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <XPT2046_Touchscreen.h>

// Pins
#define TFT_CS    10
#define TFT_DC    6
#define TFT_RST   7
#define TOUCH_CS  8

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
XPT2046_Touchscreen ts(TOUCH_CS);

typedef struct struct_message {
    int id;
    int points;
} struct_message;

struct_message myData;
int nodePoints[5] = {0, 0, 0, 0, 0}; 

void updateDisplay() {
  int totalSum = 0;
  for(int i = 1; i <= 4; i++) totalSum += nodePoints[i];

  tft.fillScreen(ILI9341_BLACK);
  
  // Reset Button
  tft.fillRect(10, 180, 150, 50, ILI9341_RED);
  tft.setCursor(20, 195);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.println("RESET");

  // Sum Display
  tft.setCursor(10, 50);
  tft.setTextSize(5);
  tft.print("Point: ");
  tft.println(totalSum);
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  if (myData.id >= 1 && myData.id <= 4) {
    nodePoints[myData.id] = myData.points;
    updateDisplay();
  }
}

void setup() {
  tft.begin();
  tft.setRotation(1);
  ts.begin();
  updateDisplay();
  
  WiFi.mode(WIFI_MODE_STA);
  esp_now_init();
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  // Simple check for touch in the reset button area
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    // Update these coordinates based on your calibration
    if (p.x > 2000 && p.x < 3500 && p.y > 1000 && p.y < 2000) {
      resetPoints(); 
      delay(500); 
    }
  }
}

void resetPoints() {
  // Loop through all indices used (1 to 4) and set to 0
  for(int i = 1; i <= 4; i++) {
    nodePoints[i] = 0;
  }
  
  // Refresh the display to show the new '0' total
  updateDisplay();
}