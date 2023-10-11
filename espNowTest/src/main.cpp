#include <esp_now.h>
#include <WiFi.h>
#include <Arduino.h>

typedef struct struct_message {
    double lon;
    double lat;
    double alt;
} struct_message;

struct_message myData;

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData, incomingData, sizeof(myData));
  String data = String(myData.lat) + "," + String(myData.lon) + "," + String(myData.alt);
 Serial.println(data)
}
 
void setup(){
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK){
    Serial.println("Error intialising ESP NOW");
    return;
  }
 // esp_now_unregister_recv_cb(OnDataRecv);
}
 
void loop(){

}


// DC:54:75:D0:49:40
// Rotator D4:D4:DA:E5:6F:A4