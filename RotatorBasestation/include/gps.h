#pragma once
#include <config.h>

class Gps
{
public:
  /**
   * @brief GPS object
   */
  TinyGPSPlus gps;
  
  /**
   * @brief GPS config object. This config is copied from the config object in beginGps()
   */
  Config::Gps_Config gps_config;

  /**
   * @brief Initialise the GPS
   * @param gps_config GPS config object
   * @return Whether the GPS was initialised successfully
   */
  bool beginGps(const Config::Gps_Config &gps_config);

  /**
   * @brief Read the GPS data
   * @param gps GPS object
   * @return Whether the GPS data was read successfully
   */
  bool readGps(TinyGPSPlus &gps);
};