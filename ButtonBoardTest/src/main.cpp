#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PCF8575.h>

// I2C
// CHANGE TO USED PINS AND I2C BUS
const int SDA = 0;
const int SCL = 1;
#define WIRE Wire // Wire or Wire1

// Port extender object
Adafruit_PCF8575 pcf;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Init I2C
  WIRE.setPins(SDA, SCL);
  WIRE.begin(); 

  // Begin Port extender
  if (!pcf.begin(0x20, &WIRE)) {
    while (true)
    {
      Serial.println("Couldn't find PCF8575");
      delay(1000);
    }
  }

  // Set all pins to input
  for (uint8_t p=0; p<16; p++)
  {
    pcf.pinMode(p, INPUT_PULLUP);
  }
}

void loop() {
  // Check if any pin is pressed
  for (uint8_t p=0; p<16; p++) 
  {
    if (!pcf.digitalRead(p)) {
      Serial.print("Button on GPIO #");
      Serial.print(p);
      Serial.println(" pressed!");
    }
  }
  delay(1);
}