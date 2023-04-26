#include <ESP8266WiFi.h>
#include <espnow.h>
#include <FirebaseArduino.h>

#define FIREBASE_HOST "led-control-926eb-default-rtdb.asia-southeast1.firebasedatabase.app" 
#define FIREBASE_AUTH "rvBycBAzVByojzWQcFid6eIPLtIJoRsQNGWTaF8X"   
#define WIFI_SSID "HAHA"   
#define WIFI_PASSWORD "4119627022"

int countDHT = 0;
int countSmoke = 0;
int countAlert = 0;
int ledAlert = 16;
int ledDHT = 5;
int ledSmoke = 4;

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  float temperature;
  float humidity;
  float heatIndex;
  int smoke;
} struct_message;

// Create a struct_message called myData
struct_message myData;

// Callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&myData, incomingData, sizeof(myData));
}

void connectedWifiAndFirebase(){
  delay(1000);                
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);                                      //try to connect with wifi
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP Address is : ");
  Serial.println(WiFi.localIP());                                                      //print local IP address
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);                                       // connect to firebase
  delay(1000);
}

//check 10 times if sensor broken
void checkSensor(){
  if (countDHT >= 10) {
    Firebase.setBool("Error/DHT", true);
    digitalWrite(ledDHT, 1);    
  }else{
    Firebase.setBool("Error/DHT", false);    
    digitalWrite(ledDHT, 0);
  }

  if (countSmoke >= 10) {
    Firebase.setBool("Error/Smoke", true);
    digitalWrite(ledSmoke, 1);
  }else{
    Firebase.setBool("Error/Smoke", false); 
    digitalWrite(ledSmoke, 0);   
  }
}

//if fire is exist, check 3 times
void fireAlert(){
  if(myData.heatIndex > 80 || myData.temperature > 60 || myData.smoke > 800){
    countAlert++;
    if (countAlert > 3) {
      digitalWrite(ledAlert, 1);
      Firebase.setBool("State/Flammable", true);
    }      
  }else{
    countAlert = 0;
    digitalWrite(ledAlert, 0); 
    Firebase.setBool("State/Flammable", false);  
  }
}
 
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  connectedWifiAndFirebase();
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);

  /*
  Register a callback function that is triggered upon receiving data. 
  When data is received via ESP-NOW, a function is called.
  */
  esp_now_register_recv_cb(OnDataRecv);
 
  pinMode(ledAlert, OUTPUT);
  pinMode(ledDHT, OUTPUT);
  pinMode(ledSmoke, OUTPUT);
  digitalWrite(ledAlert, 0);
  digitalWrite(ledSmoke, 0);
  digitalWrite(ledDHT, 0);
}

void loop() {
  delay(1000);
  //DHT11
  if((isnan(myData.temperature) && isnan(myData.humidity))){
    countDHT++;
    Serial.println("Failed to read DHT11");
  }else{
    countDHT = 0;
    Firebase.setInt("Value/Temperature", myData.temperature); 
    Firebase.setInt("Value/Humidity", myData.humidity);
    Firebase.setInt("Value/Heat_Index", myData.heatIndex);

    Serial.println("Send to Firebase DHT11");
  }
  //Smoke
  if(myData.smoke < 100){
    countSmoke++;
    Serial.println("Failed to read Smoke");    
  }else{
    countSmoke = 0;
    Firebase.setInt("Value/Smoke_Index", myData.smoke);
    Serial.println("Send to Firebase Smoke");
  }

  checkSensor();  
  fireAlert();

  Serial.println();  
    
  
}

