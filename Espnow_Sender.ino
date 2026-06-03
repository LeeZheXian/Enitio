#include <esp_now.h>
#include <WiFi.h>

// MAC Address of your Receiver Hub
uint8_t broadcastAddress[] = {0xB8, 0xD6, 0x1A, 0xAA, 0x50, 0xC0};

typedef struct struct_message {
    int id;
    int points;
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_MODE_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  // ==========================================
  // FUTURE SENSOR LOGIC SECTION
  // Add your sensor reading code here later
  // E.g., myData.points = analogRead(34);
  // ==========================================
  
  // Hardcoded value for testing right now:
  myData.id = 1; // CHANGE THIS FOR EACH MINI (1, 2, 3, or 4)
  myData.points = 10; 

  esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
  
  delay(2000 + random(0, 500));
}