#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PCF8575.h>
#include <SPI.h>

/*
1. Simply include the wanted test file using #include   !! dont include more than one
2. make sure the included file has the right pins
3. run the start function
*/

#include <lora_test.h>

void setup()
{
    Serial.begin(115200); // initialize serial
    while (!Serial)
    {
        delay(100);
    }
    delay(1000);
    Serial.println("------------------- starting ------------------");
    // Add start here
    start();

    Serial.println("------------------- setup / test done ------------------");
}

void loop()
{
}