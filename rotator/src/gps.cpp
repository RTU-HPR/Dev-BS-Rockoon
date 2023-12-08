#include "gps.h"
void GPS::initGPS(){
    Serial.begin(115200);
    _gps_initialised = true;
    Serial1.begin(gpsBaud);
    
}
void GPS::readGPS(){
    while (Serial1.available()>0){
        char result = Serial1.read();
        _gps.encode(result);
        if(_gps.location.isUpdated()){
            double newGPSLat = _gps.location.lat();
            double newGPSLon = _gps.location.lng();
            //sanity check, compares to 0 and checks if within our viscinity
            if((newGPSLat == 0 && newGPSLon == 0) || ((50 <= newGPSLat && newGPSLat <= 60) && (15 <= newGPSLon && newGPSLon <= 35))) {
                last_gps_packet_time = millis();
                data.gpsLat = newGPSLat;
                data.gpsLon = newGPSLon;
                data.gpsAlt = _gps.altitude.meters();
                data.gpsSattellites = _gps.satellites.value();
            }
            else{
                Serial.println("Location impossible");
            }
        }
    }
}