#include <Arduino.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <AccelStepper.h>

/*

@file  DRI0043.ino
@brief TB6600 arduino Stepper Motor Driver is an easy-to-use professional stepper motor driver, which could control a two-phase stepping motor.
@copyright  Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com/)
@license  The MIT License (MIT)
@author  DFRobot
@version  V1.0
@date  2023-08-03
*/

int PUL = 45; // define Pulse pin
int DIR = 46; // define Direction pin
int ENA = 42; // define Enable Pin

int PUL2 = 48; // define Pulse pin
int DIR2 = 47; // define Direction pin
int ENA2 = 33; // define Enable Pin

void setup()
{
    pinMode(PUL, OUTPUT);
    pinMode(DIR, OUTPUT);
    pinMode(ENA, OUTPUT);
    pinMode(PUL2, OUTPUT);
    pinMode(DIR2, OUTPUT);
    pinMode(ENA2, OUTPUT);
    Serial.begin(115200);
}

void loop()
{

    for (int i = 0; i < 20000; i++) // Forward 5000 steps
    {
        digitalWrite(DIR, LOW);
        digitalWrite(ENA, LOW);
        digitalWrite(PUL, HIGH);

        digitalWrite(DIR2, LOW);
        digitalWrite(ENA2, LOW);
        digitalWrite(PUL2, HIGH);
        delayMicroseconds(2000);
        digitalWrite(PUL, LOW);
        digitalWrite(PUL2, LOW);
        delayMicroseconds(2000);
    }
    for (int i = 0; i < 20000; i++) // Backward 5000 steps
    {
        digitalWrite(DIR, HIGH);
        digitalWrite(ENA, LOW);
        digitalWrite(PUL, HIGH);
        digitalWrite(DIR2, HIGH);
        digitalWrite(ENA2, LOW);
        digitalWrite(PUL2, HIGH);
        delayMicroseconds(2000);
        digitalWrite(PUL, LOW);
        digitalWrite(PUL2, LOW);
        delayMicroseconds(2000);
    }
    Serial.println("hei");
    delay(1000);
}
