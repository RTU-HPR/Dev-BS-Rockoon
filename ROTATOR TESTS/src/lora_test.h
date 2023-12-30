#include <Arduino.h>
#include <SPI.h>
#include "RadioLib_wrapper.h"

// Pins for the SPI bus, that the radio module uses
const int SPI_RX = 11; // MISO
const int SPI_TX = 10; // MOSI
const int SPI_SCK = 9;

// Used radio module series
// Our commonly used LoRa module and its series:
//  * RFM96 - Module is SX1272 and family is SX127X
//  * E22-400 - Module is SX1268 and family is SX126X
#define radio_module SX1262
#define radio_module_family Sx126x

// RX/TX switching used by the radio module
//  * Dio2 - If RX/TX enable is controlled by radio chip using DIO2
//  * Gpio - If RX/TX enable controlled by micro controller GPIO pins (if this is set define RX_enable TX_enable gpio pins)
//  * Disabled  - If not using either, set to this
// DIO2 and GPIO currently only set up for SX126X LoRa
#define radio_module_rf_switching Disabled

// Radio module config
RadioLib_Wrapper<radio_module>::Radio_Config radio_config{
    .frequency = 434, // Frequency
    .cs = 8,          // Chip select
    .dio0 = 13,       // Busy
    .dio1 = 14,       // Interrupt action
    .family = RadioLib_Wrapper<radio_module>::Radio_Config::Chip_Family::radio_module_family,
    .rf_switching = RadioLib_Wrapper<radio_module>::Radio_Config::Rf_Switching::radio_module_rf_switching,
    // If using GPIO pins for RX/TX switching, define pins used for RX and TX control
    .rx_enable = -1,
    .tx_enable = -1,
    .reset = 12,
    .sync_word = 0xF4,
    .tx_power = 14,
    .spreading = 10,
    .coding_rate = 7,
    .signal_bw = 125,
    .frequency_correction = false,
    .spi_bus = &SPI // SPI bus used by radio
};

// Ping pong
bool should_transmit = true;
int message_index = 1;
int WAIT_FOR_RECEIVE = 5000;
unsigned long last_transmit_time = 0;

void start()
{
    // Configure and begin SPI bus
    radio_config.spi_bus->begin(SPI_SCK, SPI_RX, SPI_TX);

    RadioLib_Wrapper<radio_module> *radio = new RadioLib_Wrapper<radio_module>(nullptr);

    // Configure radio module
    if (!radio->begin(radio_config))
    {
        while (true)
        {
            Serial.println("Configuring LoRa failed");
            delay(5000);
        }
    }

    Serial.println("Lora initialized");
    // If required a test message can be transmitted
    // radio->test_transmit();

    // ping pong
    while (true)
    {
        // Variables to store the received data
        String msg = "";
        float rssi = 0;
        float snr = 0;

        // Check if anything has been received
        double frquency = 0;
        if (radio->receive(msg, rssi, snr, frquency))
        {
            // Check if checksum matches
            if (radio->check_checksum(msg))
            {
                Serial.println("LoRa received: " + msg + " | RSSI: " + rssi + "| SNR: " + snr);
            }
            else
            {
                Serial.println("LoRa received with checksum fail: " + msg + " | RSSI: " + rssi + "| SNR: " + snr);
            }
            should_transmit = true;
        }

        // If nothing has been received in the defined time period, transmit a message
        if (should_transmit || millis() - last_transmit_time > WAIT_FOR_RECEIVE)
        {
            String tx_message = "Ping pong message " + String(message_index);
            radio->add_checksum(tx_message);

            while (!radio->transmit(tx_message))
            {
                delay(5);
            }
            Serial.println("msg sent");

            message_index++;
            // should_transmit = false;
            last_transmit_time = millis();
        }
    }
}
