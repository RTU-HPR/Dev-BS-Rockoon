#include "core\log.h"
#include <SDFS.h>

// Call flash, eeprom and LoRa init
void Log::init(Config &config)
{
    // Init LoRa
    String status = _com_lora.init(true, config.com_config);
}

// Sends the provided message using LoRa
bool Log::send_com_lora(String msg, Config &config)
{
    // Check if LoRa is initialized
    if (!_com_lora.get_init_status())
    {
        return false;
    }

    // Wait while transmission is happening
    while (_com_lora.send(msg) == false)
    {
        delay(1);
    }

    return true;
}

// Checks if LoRa has received any messages. Sets the message to the received one, or to empty string otherwise
void Log::receive_com_lora(String &msg, float &rssi, float &snr, Config &config)
{
    // Check if LoRa is initialized
    if (!_com_lora.get_init_status())
    {
        return;
    }

    // Get data from LoRa
    bool message_received = _com_lora.receive(msg, rssi, snr);

    // Finish receive if all good
    if (message_received)
    {
        return;
    }

    // Default message
    msg = "";
}