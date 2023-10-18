#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <RadioLib.h>

#include "angleCalculations.cpp"

#include <esp_now.h>
#include <WiFi.h>

#define LoRa_MOSI 10
#define LoRa_MISO 11
#define LoRa_SCK 9

#define LoRa_nss 8
#define LoRa_dio1 14
#define LoRa_nrst 12
#define LoRa_busy 13

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
    int elev;
    int azim;
} struct_message;

// Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

SX1262 radio = new Module(LoRa_nss, LoRa_dio1, LoRa_nrst, LoRa_busy);
void setup(){
 // lora::setup;
  Serial.begin(115200);
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
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
  SPI.begin(LoRa_SCK, LoRa_MISO, LoRa_MOSI, LoRa_nss);

  // initialize SX1262 with default settings
  Serial.print(F("[SX1262] Initializing ... "));
  int state = radio.begin();
  if (state == RADIOLIB_ERR_NONE)
  {
    Serial.println(F("success!"));
  }
  else
  {
    Serial.print(F("failed, code "));
    Serial.println(state);
    while (true);
  }
  
}
//Antenna location
double antennaLongitude = 24.15;
double antennaLatitude = 56.94;

double rocketLongitude = 56.94628;
double rocketLatitude = 24.10507;
double rocketAltitude = 100;
float RSSI = 0;

int i = 5;



double rotationPrecision = 5; //Antenas precizitāte grādos

void getLocation(String data){
  Serial.println(data);
  String delimiter = ",";
  rocketLatitude = data.substring(0, data.indexOf(delimiter)).toDouble();
  data = data.substring(data.indexOf(delimiter)+1);
  rocketLongitude = data.substring(0, data.indexOf(delimiter)).toDouble();
  rocketAltitude = data.substring(data.indexOf(delimiter)+1).toDouble();
}

void receive(){
  String received;
   int state = radio.receive(received);
   if(state == RADIOLIB_ERR_NONE){
     getLocation(received);
     RSSI = radio.getRSSI();
   }else{
     Serial.println(state);
   }
}

void loop() {

  receive();
  int elevation = calculateElevAngle(rocketLatitude, rocketLongitude, antennaLatitude, antennaLongitude, rocketAltitude, rotationPrecision);  //Radiānos
  int azimuth = calculateAzimuth(rocketLatitude, rocketLongitude, antennaLatitude, antennaLongitude, rotationPrecision);
  Serial.println("elevation: " + String(elevation) + " azimuth:" + String(azimuth) + " RSSI:" + String(RSSI));
  
  if (i==10){
    i = 0;
      // Set values to send
    myData.elev = elevation;
    myData.azim = azimuth;
    
    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
    }
  }
  i++;

  delay(1000);
}