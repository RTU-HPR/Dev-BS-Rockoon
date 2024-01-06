#pragma once
#include <TinyGPS++.h>

class Gps
{
public:
  /**
   * @brief GPS object
   */
  TinyGPSPlus gps;

  struct Gps_Config {
    HardwareSerial *serial;
  };
  Gps_Config gps_config;
  
  /**
   * @brief Initialise the GPS
   * @param gps_config GPS config object
   * @return Whether the GPS was initialised successfully
   */
  bool beginGps(const Gps_Config &gps_config);

  /**
   * @brief Read the GPS data
   * @param gps GPS object
   * @return Whether the GPS data was read successfully
   */
  bool readGps(TinyGPSPlus &gps);
};