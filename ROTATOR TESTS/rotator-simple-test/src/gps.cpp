#include <gps.h>

bool Gps::beginGps(const Gps_Config &gps_config)
{
  this->gps_config = gps_config;
  this->gps_config.serial->begin(9600);
  return true;
}

bool Gps::readGps(TinyGPSPlus &gps)
{
  while (this->gps_config.serial->available() > 0)
  {
    gps.encode(this->gps_config.serial->read());
  }
  return true;
}
