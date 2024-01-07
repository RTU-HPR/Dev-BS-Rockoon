#pragma once
#include <config.h>

// Stepper motors
class Steppers
{
  private:
    /**
     * @brief Calculates the shortest azimuth to the target
     * @param current_az Current azimuth in degrees
     * @param target_az Target azimuth in degrees
     * @return The shortest azimuth to the target. The result can be positive or negative.
    */
    double calculateShortestAzimuth(double current_az, double target_az);

    void enableAzimuthStepper(){digitalWrite(azimuth_config.enable_pin, HIGH);};
    void enableElevationStepper(){digitalWrite(elevation_config.enable_pin, HIGH);};
    void disableAzimuthStepper(){digitalWrite(azimuth_config.enable_pin, LOW);};
    void disableElevationStepper(){digitalWrite(elevation_config.enable_pin, LOW);};

  public:
    // STRUCT DECLARATIONS
    struct Current_Angle_Position
    {
      double azimuth;
      double elevation;
    };

    struct Current_Stepper_Steps
    {
      double azimuth;
      double elevation;
    };
    
    // DEFINITIONS
    /**
     * @brief Struct to hold the angle to which the steppers are currently pointing to
    */
    Current_Angle_Position current_angle = {0, 0};
    
    /**
     * @brief Struct to hold the calculated stepper steps to move to the required angles
    */
    Current_Stepper_Steps current_steps = {0, 0};

    // COPIES OF CONFIG OBJECTS TO INCREASE PERFORMANCE/READABILITY
    // In beginSteppers() the config objects are copied to these variables
    Config::Stepper_Config azimuth_config;
    Config::Stepper_Config elevation_config;

    /**
     * @brief Azimuth stepper object
    */
    AccelStepper *azimuth_stepper;

    /**
     * @brief Elevation stepper object
    */
    AccelStepper *elevation_stepper;

    // FUNCTION DECLARATIONS
    /**
     * @brief Initialise the stepper motors
     * @param azimuth_config Azimuth stepper config
     * @param elevation_config Elevation stepper config
    */
    void beginSteppers(Config::Stepper_Config azimuth_config, Config::Stepper_Config elevation_config);
    
    /**
     * @brief Sets the required position for the steppers to move to
     * @param azimuth Azimuth angle in degrees
     * @param elevation Elevation angle in degrees
    */
    void setRequiredPosition(double azimuth, double elevation);

    /**
     * @brief Moves the steppers to the required position. Call this function in the main loop as often as possible
    */
    void moveSteppers();
};