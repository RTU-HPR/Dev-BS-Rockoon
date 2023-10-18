#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <iostream>
#include <LoRa.h>
#include <EEPROM.h>
#include <RadioLib.h>
#include "LoRa.cpp"
//SX1272 radios = new Module(loraSS, loraDIO0, loraRST, loraDIO1);
#define LoRa_MOSI 10
#define LoRa_MISO 11
#define LoRa_SCK 9
#define LoRa_nss 8
#define LoRa_dio1 14
#define LoRa_nrst 12
#define LoRa_busy 13
String outgoing = "string";
SX1262 radio = new Module(LoRa_nss, LoRa_dio1, LoRa_nrst, LoRa_busy);
void setup(){
  //lora::setup;
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
void transmitter (String outgoing){
Serial.print(F("[SX1262] Transmitting packet ... "));

  int state = radio.transmit(outgoing);

  if (state == RADIOLIB_ERR_NONE)
  {
    // the packet was successfully transmitted
    Serial.println(F("success!"));
  }
  else if (state == RADIOLIB_ERR_PACKET_TOO_LONG)
  {
    // the supplied packet was longer than 256 bytes
    Serial.println(F("too long!"));
  }
  else if (state == RADIOLIB_ERR_TX_TIMEOUT)
  {
    // timeout occured while transmitting packet
    Serial.println(F("timeout!"));
  }
  else
  {
    // some other error occurred
    Serial.print(F("failed, code "));
    Serial.println(state);
  }
}
double longtitude = 56.9527230;
double latitude = 24.0809493;
int altitude = 100;
// void send(String message){
//         Serial.println("Transmitting:");
//         int state = radios.transmit(message);
//         if(state == RADIOLIB_ERR_NONE){
//             Serial.println("success");
//         }else if(state == RADIOLIB_ERR_PACKET_TOO_LONG){
//             Serial.println("failed, packet too long");
//         }else{
//             Serial.println(state);
//         }
// }
//rewrite using sandeep mistry's lib
void loop() {
  
  String data = String(longtitude) + "," + String(latitude) + "," + String(altitude);
  Serial.println(data);
  //lora::send(data);
  //send(data);
  transmitter(data);
  altitude = altitude +1;
  delay(1000);
  }