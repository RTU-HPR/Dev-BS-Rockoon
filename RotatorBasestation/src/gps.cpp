#include <gps.h>

bool Gps::beginGps(const Config::Gps_Config &gps_config)
{
  this->gps_config = gps_config;
  return true;
}

bool Gps::readGps()
{
  while (gps_config.serial->available() > 0)
  {
    gps.encode(this->gps_config.serial->read());
  }
  return true;
}
