#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <RadioLib.h>

//#include "LoRa.h"
#define LoRa_MOSI 10
#define LoRa_MISO 11
#define LoRa_SCK 9

#define LoRa_nss 8
#define LoRa_dio1 14
#define LoRa_nrst 12
#define LoRa_busy 13
//dont use pin 27
SX1262 radio = new Module(LoRa_nss, LoRa_dio1, LoRa_nrst, LoRa_busy);
void setup(){
 // lora::setup;
  Serial.begin(115200);
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