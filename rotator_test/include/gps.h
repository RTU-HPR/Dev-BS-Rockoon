#pragma once
#include <TinyGPS++.h>
#include <Arduino.h>
#include <Wire.h>
#include <string.h>
#include <SPI.h>
//#include <SoftwareSerial.h>
#include <HardwareSerial.h>
class GPS {
    private:
        TinyGPSPlus _gps;
        unsigned long last_gps_packet_time = 0;
        uint8_t gpsRX = 1;
        uint8_t gpsTX = 0;
        static const uint32_t gpsBaud = 9600;   
    public:
        bool _gps_initialised = false;
        struct data
        {
            float gpsLat = 0;
            float gpsLon = 0;
            float gpsAlt = 0;
            int gpsSattellites = 0;
        };
        data data;
        void readGPS();
        void initGPS();
};