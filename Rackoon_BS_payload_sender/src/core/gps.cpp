#include "core\gps.h"
void gpsClass::read_gps(Log &log, Config &config)
{
    if (!_gps_initialized)
    {
        Serial.println("failed");
        return;
    }
    while (_gps_serial->available() > 0)
    {
        _gps.encode(_gps_serial->read());

        if (_gps.location.isUpdated())
        {
            double new_gps_lat = _gps.location.lat();
            double new_gps_lng = _gps.location.lng();


            // SANITY CHECK, BECAUSE THERE IS NOTHING ELSE TO REALLY CHECK
            // Check if location is 0 (not yet established) or somewhere in the northern eastern Europe
            if ((new_gps_lat == 0 && new_gps_lng == 0) || ((50 <= new_gps_lat && new_gps_lat <= 60) && (15 <= new_gps_lng && new_gps_lng <= 35)))
            {
                _last_gps_packet_time = millis();
                data.gps_lat = new_gps_lat;
                data.gps_lng = new_gps_lng;
                data.gps_alt = _gps.altitude.meters();
                data.gps_satellites = _gps.satellites.value();
                //data.gps_time = gps.time.value();
            }
            else
            {
                //log.log_error_msg_to_flash("GPS location is not correct: " + String(new_gps_lat, 6) + " " + String(new_gps_lng, 6));
            }
        }
    }
}
void getGps(){
}