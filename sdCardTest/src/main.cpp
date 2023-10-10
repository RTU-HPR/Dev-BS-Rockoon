#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "sdCard.h"
#define SD_CS 36
#define SD_MOSI 34
#define SD_MISO 19
#define SD_SCLK 5

SPIClass sd(HSPI);
void setup(){
  Serial.begin(115200);
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
void loop(){
 double longtitude = 56.95;
 double latitude = 24.11;
 int altitude = 100;
 altitude = altitude +1; 
 SDCard :: appendFile(SD, "/data.csv", "\n");
 SDCard :: readFile(SD, "/data.csv");

 String data = String(longtitude) +"," + String(latitude) + "," + String(altitude); 

 SDCard ::  appendFile (SD, "/data.csv", data);
 
 delay(1000);
}