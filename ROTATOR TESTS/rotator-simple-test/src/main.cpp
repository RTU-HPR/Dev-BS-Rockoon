#include <Arduino.h>

#include <config.h>

Config config;
Steppers steppers;
Gps gps;

void setup()
{
  Serial.begin(config.PC_BAUDRATE);
  while (!Serial)
  {
    delay(100); // wait for serial port to connect.
  }

  // Start the GPS serial
  Serial1.begin(config.SERIAL1_BAUDRATE);
  Serial1.setPins(config.SERIAL1_RX, config.SERIAL1_TX);

  // Start the GPS
  gps.beginGps(config.gps_config);

  // Start the steppers
  steppers.beginSteppers(config.azimuth_config, config.elevation_config);

  Serial.println("Setup Complete");
}

void loop()
{
  while (Serial.available() > 0)
  {
    String input = Serial.readString();
    input.trim();

    if (input == "n")
    {
      Serial.println("North");
      steppers.setRequiredPosition(0, 0);
    }
    else if (input == "w")
    {
      Serial.println("West");
      steppers.setRequiredPosition(270, 0);
    }
    else if (input == "e")
    {
      Serial.println("East");
      steppers.setRequiredPosition(90, 0);
    }
    else if (input == "s")
    {
      Serial.println("South");
      steppers.setRequiredPosition(180, 0);
    }
    else if (input == "nw")
    {
      Serial.println("North-West");
      steppers.setRequiredPosition(315, 0);
    }
    else if (input == "ne")
    {
      Serial.println("North-East");
      steppers.setRequiredPosition(45, 0);
    }
    else if (input == "sw")
    {
      Serial.println("South-West");
      steppers.setRequiredPosition(225, 0);
    }
    else if (input == "se")
    {
      Serial.println("South-East");
      steppers.setRequiredPosition(135, 0);
    }
    else
    {
      Serial.println("Elev: " + input);
      steppers.setRequiredPosition(0, input.toInt());
    }
    Serial.println("Azimuth: " + String(steppers.current_angle.azimuth) + "|" + String(steppers.current_steps.azimuth) + " Elevation: " + String(steppers.current_angle.elevation) + "|" + String(steppers.current_steps.elevation));
  }

  // If steppers have reached their position, disable them
  if (steppers.current_steps.azimuth == steppers.azimuth_stepper->currentPosition() && steppers.current_steps.elevation == steppers.elevation_stepper->currentPosition())
  {
    steppers.disableAzimuthStepper();
    steppers.disableElevationStepper();
  }
  else
  {
    steppers.enableAzimuthStepper();
    steppers.enableElevationStepper();
  }

  // Move the steppers
  steppers.moveSteppers();
}