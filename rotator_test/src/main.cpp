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

int ENA = 40; // define Enable Pin
int DIR = 39; // define Direction pin
int PUL = 45; // define Pulse pin

int ENA2 = 33; // define Enable Pin
int DIR2 = 47; // define Direction pin
int PUL2 = 48; // define Pulse pin

AccelStepper ELstepper(1, PUL, DIR);
AccelStepper AZstepper(1, PUL2, DIR2);

void setup()
{
    pinMode(PUL, OUTPUT);
    pinMode(DIR, OUTPUT);
    pinMode(ENA, OUTPUT);
    pinMode(PUL2, OUTPUT);
    pinMode(DIR2, OUTPUT);
    pinMode(ENA2, OUTPUT);
    Serial.begin(115200);

    AZstepper.setPinsInverted(true, true, true);
    AZstepper.setMaxSpeed(800);
    AZstepper.setAcceleration(600);
    AZstepper.setMinPulseWidth(50);

    ELstepper.setPinsInverted(true, true, true);
    ELstepper.setMaxSpeed(800);
    ELstepper.setAcceleration(600);
    ELstepper.setMinPulseWidth(50);

    digitalWrite(ENA, HIGH);
    digitalWrite(ENA2, HIGH);

    AZstepper.setCurrentPosition(0);
    ELstepper.setCurrentPosition(0);
}

void loop()
{
    Serial.println("BOTH STEPPERS TO 10000");
    long pos = 10000;

    AZstepper.moveTo(pos);
    ELstepper.moveTo(pos);

    unsigned long runTime = millis();
    while(millis() - runTime < 10000)
    {
        AZstepper.run();
        ELstepper.run();
    }

    Serial.println("BOTH STEPPERS TO 0");
    pos = 0;

    AZstepper.moveTo(pos);
    ELstepper.moveTo(pos);

    runTime = millis();
    while(millis() - runTime < 10000)
    {
        AZstepper.run();
        ELstepper.run();
    }

    Serial.println("AZ STEPPER TO 10000");
    pos = 10000;

    AZstepper.moveTo(pos);

    runTime = millis();
    while(millis() - runTime < 10000)
    {
        AZstepper.run();
    }

    Serial.println("AZ STEPPER TO 0");
    pos = 0;

    AZstepper.moveTo(pos);

    runTime = millis();
    while(millis() - runTime < 10000)
    {
        AZstepper.run();
    }

    Serial.println("EL STEPPER TO 10000");
    pos = 10000;

    ELstepper.moveTo(pos);

    runTime = millis();
    while(millis() - runTime < 10000)
    {
        ELstepper.run();
    }
    Serial.println("EL STEPPER TO 0");

    pos = 0;

    ELstepper.moveTo(pos);

    runTime = millis();
    while(millis() - runTime < 10000)
    {
        ELstepper.run();
    }

    // for (int i = 0; i < 20000; i++) // Forward 5000 steps
    // {
    //     digitalWrite(DIR, LOW);
    //     digitalWrite(ENA, LOW);
    //     digitalWrite(PUL, HIGH);

    //     digitalWrite(DIR2, LOW);
    //     digitalWrite(ENA2, LOW);
    //     digitalWrite(PUL2, HIGH);
    //     delayMicroseconds(2000);
    //     digitalWrite(PUL, LOW);
    //     digitalWrite(PUL2, LOW);
    //     delayMicroseconds(2000);
    // }
    // for (int i = 0; i < 20000; i++) // Backward 5000 steps
    // {
    //     digitalWrite(DIR, HIGH);
    //     digitalWrite(ENA, LOW);
    //     digitalWrite(PUL, HIGH);
    //     digitalWrite(DIR2, HIGH);
    //     digitalWrite(ENA2, LOW);
    //     digitalWrite(PUL2, HIGH);
    //     delayMicroseconds(2000);
    //     digitalWrite(PUL, LOW);
    //     digitalWrite(PUL2, LOW);
    //     delayMicroseconds(2000);
    // }
    // Serial.println("hei");
    // delay(1000);
}
