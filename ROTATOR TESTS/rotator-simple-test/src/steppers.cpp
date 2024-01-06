#include "steppers.h"

void Steppers::beginSteppers(Stepper_Config azimuth_config, Stepper_Config elevation_config)
{
  // Copy the config to the class
  this->azimuth_config = azimuth_config;
  this->elevation_config = elevation_config;

  // Set the pins
  pinMode(azimuth_config.enable_pin, OUTPUT);
  pinMode(elevation_config.enable_pin, OUTPUT);
  digitalWrite(azimuth_config.enable_pin, LOW);
  digitalWrite(elevation_config.enable_pin, LOW);

  // Create the stepper objects
  azimuth_stepper = new AccelStepper(AccelStepper::DRIVER, azimuth_config.step_pin, azimuth_config.dir_pin);
  elevation_stepper = new AccelStepper(AccelStepper::DRIVER, elevation_config.step_pin, elevation_config.dir_pin);

  // Set the stepper properties
  azimuth_stepper->setMaxSpeed(azimuth_config.max_speed);
  azimuth_stepper->setAcceleration(azimuth_config.max_acceleration);
  //azimuth_stepper->setPinsInverted(true, true, true);
  azimuth_stepper->setMinPulseWidth(azimuth_config.min_pulse_width);

  elevation_stepper->setMaxSpeed(elevation_config.max_speed);
  elevation_stepper->setAcceleration(elevation_config.max_acceleration);
  elevation_stepper->setPinsInverted(true, true, true);
  elevation_stepper->setMinPulseWidth(elevation_config.min_pulse_width);

  // Set the starting position to 0
  azimuth_stepper->setCurrentPosition(0);
  elevation_stepper->setCurrentPosition(0);
} 

double Steppers::calculateShortestAzimuth(double current_az, double target_az)
{
  // double diff1 = abs(current_az - target_az);
  // double diff2 = abs(current_az - (target_az + 360));
  // double diff3 = abs(current_az - (target_az - 360));

  // if ((diff1 <= diff2) && (diff1 < diff3))
  //   return target_az;
  // if ((diff2 < diff1) && (diff2 < diff3))
  //   return target_az + 360;
  // return target_az - 360;

  // Normalize angles to be within the range [0, 360)
  current_az = fmod(current_az, 360.0);
  target_az = fmod(target_az, 360.0);

  // Calculate the difference between the angles
  double angleDifference = target_az - current_az;

  // Adjust for the shortest azimuth
  if (angleDifference > 180.0) {
      angleDifference -= 360.0;
  } else if (angleDifference < -180.0) {
      angleDifference += 360.0;
  }
  // Normalize the result to be within the range [0, 360)
  double result = fmod(current_az + angleDifference, 360.0);

  return result;
}

void Steppers::setRequiredPosition(double azimuth, double elevation)
{
  Serial.println("Passed in azimuth: " + String(current_angle.azimuth) + "|" +  String(azimuth));
  // Calculate the shortest path to the target
  double shortest_azimuth = calculateShortestAzimuth(current_angle.azimuth, azimuth);
  Serial.println("Calculate shortest azimuth: " + String(shortest_azimuth));

  // Calculate the stepper steps
  current_steps.azimuth = (shortest_azimuth * azimuth_config.steps_per_revolution * azimuth_config.gear_ratio) / 360;
  current_steps.elevation = (elevation * elevation_config.steps_per_revolution * elevation_config.gear_ratio) / 360;

  // Update the current position
  current_angle.azimuth = shortest_azimuth;
  current_angle.elevation = elevation;
}

void Steppers::moveSteppers()
{
    azimuth_stepper->moveTo(current_steps.azimuth);
    elevation_stepper->moveTo(current_steps.elevation);

    azimuth_stepper->run();
    elevation_stepper->run();
}