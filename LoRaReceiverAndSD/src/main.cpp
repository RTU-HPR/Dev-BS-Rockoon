#include <Arduino.h>
#include "sdCard.h"
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <EEPROM.h>
#include <SD.h>
#include <RadioLib.h>

#define SD_CS 36
#define SD_MOSI 34
#define SD_MISO 19
#define SD_SCLK 5
#define LoRa_MOSI 10
#define LoRa_MISO 11
#define LoRa_SCK 9
#define LoRa_nss 8
#define LoRa_dio1 14
#define LoRa_nrst 12
#define LoRa_busy 13
SPIClass sd(HSPI);
SX1262 radio = new Module(LoRa_nss, LoRa_dio1, LoRa_nrst, LoRa_busy);
void setup() {
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
    sd.begin(SD_SCLK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, sd)){
    Serial.println("Card mount failed");
  }else{
    Serial.println("Card successfully mounted");
  }
    SDCard :: createDir(SD, "/BallonData");
  SDCard :: listDir(SD, "/BallonData", 0);
  SDCard :: writeFile(SD, "/data.csv", "Longtitude, Latitude, Altitude");
  SDCard :: readFile(SD, "/data.csv");
}

void loop() {
  // put your main code here, to run repeatedly:
  String received;
int state = radio.receive(received);
   if(state == RADIOLIB_ERR_NONE){
 SDCard :: appendFile(SD, "/data.csv", "\n");
 SDCard :: readFile(SD, "/data.csv");
 SDCard ::  appendFile (SD, "/data.csv", received);
   }else{
     Serial.println(state);
   }
 delay(1000);
}
