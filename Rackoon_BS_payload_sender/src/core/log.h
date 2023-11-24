#pragma once
#include <SPI.h>
#include <RadioLib.h>
#include <Arduino.h>
#include "config.h"

class Log
{
private:
    RFM_Wrapper _com_lora;
    
public:
    void init(Config &config);
    bool send_com_lora(String msg, Config &config);
    void receive_com_lora(String &msg, float &rssi, float &snr, Config &config);
};
