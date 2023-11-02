#include <Arduino.h>

#include <esp_now.h>
#include <WiFi.h>

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {212, 212, 218, 229, 111, 164};
//D4:D4:DA:E5:6F:A4
// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
    int ELEV;
    int AZIM;
} struct_message;

// Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  } else{
    Serial.println( "ESP NOW Initialised");
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }  
}
 
void loop() {
  // Set values to send
 int sk = 0;
  String a = "";
  String b = "";
  // Send message via ESP-NOW
  
   
  
  if(Serial.available()>0){
  a = Serial.readString();
  b=a;
    sk = a.indexOf(',');
    String az = a.substring(0,sk);
    String el = b.substring(sk+1, b.length());
    myData.AZIM =  az.toInt();
    myData.ELEV= el.toInt();
    //Serial.println(myData.AZIM + ", " + myData.ELEV);
    Serial.println();
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    if (result == ESP_OK) {
    Serial.println("Sent with success - " + String(myData.AZIM) + "," + String(myData.ELEV));
    delay(5000); 
  }
  else {
    Serial.println("Error sending the data");
  }
  }
  
}