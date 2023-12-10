#include <core/cansat.h>

// PRIVATE FUNCTIONS
// Initialize SPI/I2C/PC Serial/Hardware Serial communication
void Cansat::init_all_com_bus(Config &config)
{
    SPI.setRX(config.SPI0_RX);
    SPI.setTX(config.SPI0_TX);
    SPI.setSCK(config.SPI0_SCK);
    SPI.begin();
}

// Main function that checks and runs appropriate state
void Cansat::start_states(Cansat &cansat)
{
    // FOR DEBUGGING PURPOSES
    // IT SHOULDN'T BE HERE, IT SHOULD ONLY BE IN INIT ALL COM BUS
    Serial.begin(config.PC_BAUDRATE);
    if (config.WAIT_PC)
    {
        while (!Serial)
        {
            delay(500);
        }
    }

    // Initialize communications
    cansat.init_all_com_bus(cansat.config);

    // Initialize SD card/communication LoRa
    cansat.log.init(cansat.config);

    // If ascent/descent state is not set, start in prepare state
    prepare_state(cansat);
}
