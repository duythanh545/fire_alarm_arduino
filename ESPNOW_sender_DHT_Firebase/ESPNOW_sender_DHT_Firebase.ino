#include <ESP8266WiFi.h>
#include <espnow.h>
#include "DHTesp.h"

// REPLACE WITH RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x2C, 0xF4, 0x32, 0x2E, 0x67, 0x3A};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  float temperature;
  float humidity;
  float heatIndex;
  int smoke;
} struct_message;

// Create a struct_message called myData
struct_message myData;

unsigned long lastTime = 0;  
unsigned long timerDelay = 500;  // send readings timer

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}
 
DHTesp dht;

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);

  /*Register a callback function that is triggered upon sending data. 
  When a message is sent, a function is called – 
  this function returns whether the delivery was successful or not.
  */  
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  //esp_now_add_peer(uint8 mac_addr, uint8 role, uint8 channel, 
  //                  uint8 key, uint8 key_len)
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

  dht.setup(5, DHTesp::DHT11);
}
 
void loop() {
  if ((millis() - lastTime) > timerDelay) {
    // Set values to send
    myData.temperature = dht.getTemperature();
    myData.humidity = dht.getHumidity();
    myData.heatIndex = dht.computeHeatIndex(myData.temperature, myData.humidity);
    myData.smoke = analogRead(A0);

    // Send message via ESP-NOW
    //esp_now_send(uint8 mac_address, uint8 data, int len)
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

    lastTime = millis();
  }
}