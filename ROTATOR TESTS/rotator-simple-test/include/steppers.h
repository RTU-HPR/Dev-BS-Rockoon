#pragma once
#include <AccelStepper.h>

// Stepper motors
class Steppers
{
  public:
    struct Stepper_Config
    {
      int step_pin;
      int dir_pin;
      int enable_pin;
      int steps_per_revolution;
      int gear_ratio;
      int min_pulse_width;
      int max_speed;
      int max_acceleration;
    };
    
    struct Current_Angle_Position
    {
      double azimuth;
      double elevation;
    };
    Current_Angle_Position current_angle = {0, 0};

    struct Current_Stepper_Steps
    {
      double azimuth;
      double elevation;
    };
    Current_Stepper_Steps current_steps = {0, 0};

  private:
    Stepper_Config azimuth_config;
    Stepper_Config elevation_config;

    double calculateShortestAzimuth(double current_az, double target_az);
  public:
    AccelStepper *azimuth_stepper;
    AccelStepper *elevation_stepper;

    void enableAzimuthStepper(){digitalWrite(azimuth_config.enable_pin, HIGH);};
    void enableElevationStepper(){digitalWrite(elevation_config.enable_pin, HIGH);};
    void disableAzimuthStepper(){digitalWrite(azimuth_config.enable_pin, LOW);};
    void disableElevationStepper(){digitalWrite(elevation_config.enable_pin, LOW);};


    void beginSteppers(Stepper_Config azimuth_config, Stepper_Config elevation_config);
    void setRequiredPosition(double azimuth, double elevation);
    void moveSteppers();
};