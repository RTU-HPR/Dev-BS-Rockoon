#include <Arduino.h>

double haversine(double rocketLatitude, double rocketLongitude, double antennaLatitude, double antennaLongitude);
int calculateElevAngle(double rocketLatitude, double rocketLongitude, double rocketAltitude, double antennaLatitude, double antennaLongitude, double antennaAltitude, int precision);
int calculateAzimuth(double rocketLatitude, double rocketLongitude, double antennaLatitude, double antennaLongitude, double precision);