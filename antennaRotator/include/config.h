#pragma once
#include <Arduino.h>

class Config
{
  public:
  unsigned int PC_BAUDRATE = 115200;
  bool WAIT_FOR_PC = true;

  struct ROTATOR_LOCATION
  {
    double LATITUDE = 56.931245;
    double LONGITUDE = 24.124262;
  };

  ROTATOR_LOCATION rotator_location;

  double ROTATION_PRECISION = 1;
};