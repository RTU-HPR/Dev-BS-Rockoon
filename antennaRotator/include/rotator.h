#pragma once
#include <config.h>
#include <comms.h>
//#include <gps.h>

// Rotator object
class Rotator
{
  private:
  
  struct TRACKABLE_OBJECT_POSITION
  {
    double longitude = 24.124262;
    double latitude = 57.3204037;
    double altitude = 0;
  };

  TRACKABLE_OBJECT_POSITION ballon_position;

  // Function declaration
  void update_location(Comms &comms, TRACKABLE_OBJECT_POSITION &ballon_position);
  void update_angles(Comms &comms, TRACKABLE_OBJECT_POSITION &ballon_position, Config &config);
  
  public:
  bool state = false;
  struct manualPosition{
      double azimuth = 0;
      double elevation = 0;
  };
  manualPosition manualPosition;
  Config config;
  Comms comms;
 // GPS gps;
  // Functions declaration
  void init(Comms &comms, Config &config);
  void update(Comms &comms, Config &config);
};