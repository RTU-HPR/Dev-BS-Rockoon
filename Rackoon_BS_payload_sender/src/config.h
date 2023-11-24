#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <LittleFS.h>
#include <SDFS.h>
#include <ranging_wrapper.h>
#include <RFM_Wrapper.h>

class Config
{
public:
    //------------ OFTEN CHANGED ------------------
    // hard data rate limiter
    const int MAX_LOOP_TIME = 20; // ms

    bool WAIT_PC = true;
    
    // 433 MHz LoRa
    RFM_Wrapper com_lora;
    RFM_Wrapper::Lora_Device com_config = {.FREQUENCY = 434,
                                        .CS = 5,
                                        .DIO0 = 7,
                                        .DIO1 = 8,
                                        .RESET = 6,
                                        .SYNC_WORD = 0x12,
                                        .TXPOWER = 10,
                                        .SPREADING = 9,
                                        .CODING_RATE = 7,
                                        .SIGNAL_BW = 125,
                                        .SPI = &SPI};

    
    //----------------------------------SOMEWHAT STATIC -------------------------------------

    // logging
    const unsigned long PC_BAUDRATE = 115200;
    
    // SPI0
    const int SPI0_RX = 4;
    const int SPI0_TX = 3;
    const int SPI0_SCK = 2;
};