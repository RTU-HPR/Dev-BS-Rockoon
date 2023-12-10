#pragma once
#include "config.h"
#include "core\log.h"
#include "states\prepare_state.h"
#include "core\gps.h"

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Servo.h>

// Cansat object
class Cansat
{ 
    void init_all_com_bus(Config &config);

public:
    Config config;
    Log log;
    gpsClass gpsfin;
    void start_states(Cansat &cansat);
};
