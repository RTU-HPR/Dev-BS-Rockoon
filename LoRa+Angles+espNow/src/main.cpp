#include <Arduino.h>
#include <RadioLib.h>
#include "class.h"
#include "wrapper.h"

rotatorAngles rotAngle = rotatorAngles(broadcastAddress, LoRa_SCK, LoRa_MISO, LoRa_MOSI, LoRa_nss);
SX1262 radio = new Module(LoRa_nss, LoRa_dio1, LoRa_nrst, LoRa_busy);

void setup(){
  rotAngle.setup(radio);
}

void loop() {
  rotAngle.loop(radio);
}