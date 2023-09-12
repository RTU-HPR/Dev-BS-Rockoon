#include <Arduino.h>
#include <Wire.h>
#include "LoRa.h"
void setup(){
  lora::setup;
  Serial.begin(115200);
  SPI.begin(loraSCK, loraMISO, loraMOSI, loraSS);
  Serial.println("Initalising LoRa");
  radio.begin(434, 125, 9, 7, 0x34, 10);
}
float longitude = 56.946285;
float latitude = 24.105078;
int altitude = 100;
void receive(){
  String received;
  int state = radio.receive(received);
  if(state == RADIOLIB_ERR_NONE){
    Serial.println(received);
  }else{
    Serial.println(state);
  }
}

void loop() {
   /* Serial.print(longitude);
    Serial.print(", ");
    Serial.print(latitude);
    Serial.print(", ");
    latitude=latitude+1;
    Serial.print(altitude);
    Serial.println();
    altitude = altitude + 1;
    */
  //lora::receive;
  receive();
  delay(1000);
}