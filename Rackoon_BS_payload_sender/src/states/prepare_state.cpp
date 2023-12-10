#include "states\prepare_state.h"
#include "core\gps.h"
double latitude = 56;
double longitude = 24;
float altitude = 1000;
// MAIN FUNCTIONS
// Prepare state loop
bool prepare_state_loop(Cansat &cansat)
{
    cansat.gpsfin.read_gps(cansat.log, cansat.config);
    latitude = cansat.gpsfin.data.gps_lat;
    longitude = cansat.gpsfin.data.gps_lng;
    altitude = cansat.gpsfin.data.gps_alt;
    String msg = String(latitude,6) + "," + String(longitude, 6) + "," + String(altitude);
    Serial.println(msg);
    cansat.log.send_com_lora(msg, cansat.config);

    /*
    while (latitude < 70)
    {
        latitude += 1;
        longitude += 0.25;
        altitude += 150;
        String msg = String(latitude, 6) + "," + String(longitude, 6) + "," + String(altitude);
        Serial.println(msg);
        cansat.log.send_com_lora(msg, cansat.config);
        delay(3000);
    }
    while (latitude > 50)
    {
        latitude -= 0.1;
        longitude -= 0.25;
        if (altitude > 1000)
        {
            altitude -= 150;
        }
        String msg = String(latitude, 6) + "," + String(longitude, 6) + "," + String(altitude);
        Serial.println(msg);
        cansat.log.send_com_lora(msg, cansat.config);
        delay(3000);
    }
    */

    return false;
}

// Prepare state setup
void prepare_state(Cansat &cansat)
{

    // Run prepare loop while waiting for arming signal
    while (!prepare_state_loop(cansat))
    {
    }
}
