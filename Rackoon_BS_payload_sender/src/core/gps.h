#pragma once
#include <TinyGPS++.h>
#include "core\log.h"
#include "config.h"
class gpsClass {
    private:
        TinyGPSPlus _gps; 
        SerialUART *_gps_serial;
        unsigned long _last_gps_packet_time = 0;
    public:
          bool _gps_initialized = false;
          struct Gdata
          {
            float gps_lat = 0;      // deg
            float gps_lng = 0;      // deg
            float gps_alt = 0;   // m
            int gps_satellites = 0; // count
          };
          Gdata data;
          void read_gps(Log &log, Config &config);
};