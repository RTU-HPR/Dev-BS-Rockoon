#pragma once
#include <config.h>
#include <comms.h>

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
  void update_location(String msg, TRACKABLE_OBJECT_POSITION &ballon_position);
  void update_angles(Comms &comms, TRACKABLE_OBJECT_POSITION &ballon_position, Config &config);
  
  public:
  Config config;
  Comms comms;

  // Functions declaration
  void init(Comms &comms, Config &config);
  void update(Comms &comms, Config &config);
};