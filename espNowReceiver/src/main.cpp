#include <esp_now.h>
#include <WiFi.h>
#include <Arduino.h>

typedef struct struct_message {
    int elev;
    int azim;
} struct_message;

struct_message myData;

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  String data = String (myData.azim) + " " + String(myData.elev);
 Serial.println(data);
}
 
void setup(){
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
  Serial.println("Error initializing ESP-NOW");
  return;
  }else{
    Serial.println("Initialised ESP now");
  }
esp_now_register_recv_cb(OnDataRecv);

}
 
void loop(){

}

// // DC:54:75:D0:49:40