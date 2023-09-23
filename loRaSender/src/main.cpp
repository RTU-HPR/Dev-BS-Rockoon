#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <iostream>
#include <LoRa.h>
#include "LoRa.cpp"
//SX1272 radios = new Module(loraSS, loraDIO0, loraRST, loraDIO1);
#define SS 18
#define RST 14
#define DI0 26
#define SCK 5
#define MISO 19
#define MOSI 27
String outgoing = "string";
void setup(){
  //lora::setup;
  Serial.begin(115200);
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DI0);
  //radios.begin(434, 125, 9, 7, 0x34, 10);
  Serial.println("Initalising LoRa");
  if(!LoRa.begin(433E6)){
    Serial.println("Starting LoRa failed");
  }else {
    Serial.println("Success");
  }
}
void transmitter (String outgoing){
  LoRa.beginPacket();
  LoRa.print(outgoing);
  LoRa.endPacket();
  Serial.println("Package sent: " + outgoing);
}
double longtitude = 56.946285;
double latitude = 24.105078;
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
  
  String data = String(latitude) + "," + String(longtitude) + "," + String(altitude);
  //lora::send(data);
  //send(data);
  transmitter(data);
  altitude = altitude +1;
  delay(1000);
  }