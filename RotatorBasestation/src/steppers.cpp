#include <steppers.h>

void Steppers::beginSteppers(Config::Stepper_Config azimuth_config, Config::Stepper_Config elevation_config)
{
  // Copy the config objects
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
  // azimuth_stepper->setPinsInverted(true, true, true);
  azimuth_stepper->setMinPulseWidth(azimuth_config.min_pulse_width);

  elevation_stepper->setMaxSpeed(elevation_config.max_speed);
  elevation_stepper->setAcceleration(elevation_config.max_acceleration);
  elevation_stepper->setPinsInverted(true, true, true); // Elevator stepper is inverted
  elevation_stepper->setMinPulseWidth(elevation_config.min_pulse_width);

  // Set the starting position to 0
  azimuth_stepper->setCurrentPosition(0);
  elevation_stepper->setCurrentPosition(0);
} 

double Steppers::calculateShortestAzimuth(double current_az, double target_az)
{
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
  // Calculate the shortest path to the target
  double shortest_azimuth = calculateShortestAzimuth(current_angle.azimuth, azimuth);

  // Calculate the stepper steps
  current_steps.azimuth = (shortest_azimuth * azimuth_config.steps_per_revolution * azimuth_config.gear_ratio) / 360;
  current_steps.elevation = (elevation * elevation_config.steps_per_revolution * elevation_config.gear_ratio) / 360;

  // Update the current position
  current_angle.azimuth = shortest_azimuth;
  current_angle.elevation = elevation;
}

void Steppers::moveSteppers()
{
  // If steppers have reached their position turn them off
  if (current_steps.azimuth == azimuth_stepper->currentPosition() && current_steps.elevation == elevation_stepper->currentPosition())
  {
    disableAzimuthStepper();
    disableElevationStepper();
    return;
  }
  else
  {
    enableAzimuthStepper();
    enableElevationStepper();
  }

  // Update the position to move to
  azimuth_stepper->moveTo(current_steps.azimuth);
  elevation_stepper->moveTo(current_steps.elevation);

  // Run the steppers
  azimuth_stepper->run();
  elevation_stepper->run();
}