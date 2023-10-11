#pragma once

#include "Arduino.h"

//Visām funkcijām vajag kopā 5 mainīgos, abas antenas koordinātes, abas raķetes koordinātes un raķetes augstumu

double haversine(double rocketLatitude, double rocketLongitude, double antennaLatitude, double antennaLongitude) {
  int r = 6371000;
  double d = 2*r*asin(sqrt(pow(sin((rocketLongitude-antennaLongitude)*PI/180/2),2)
    +cos(rocketLongitude*PI/180)*cos(antennaLongitude*PI/180)*pow(sin((rocketLatitude-antennaLatitude)*PI/180/2),2))); //Haversine
  return d;
}

double calculateElevAngle(double rocketLatitude, double rocketLongitude, double antennaLatitude, double antennaLongitude, double rocketAltitude){   //Izvada leņķi starp pieskari un taisno līniju uz objektu
  int circumference = 40075000;
  int r = 6371000;
  double surfaceDistance = haversine(rocketLatitude, rocketLongitude, antennaLatitude, antennaLongitude);
  float angleBAD = surfaceDistance/circumference*2*PI;
  float BD = sqrt(2*pow(r,2)*(1-cos(angleBAD)));       //Attālums starp atrašanās vietām ejo taisnā līnijā caur zemi (ignorējot augstumu)
  double angleABD = (PI-angleBAD)/2;                   //leņķis starp rādiusu un līniju caur zemi
  double angleCDB = PI-angleABD;                       //Leņķis starp augstuma taisni un taisni caur zemi
  double angleDBC = asin(rocketAltitude*sin(angleCDB)/BD);   //leņķis starp caur zemi esošo līniju un taisno līniju līdz objektam
  //return (angleDBC-PI/2+angleABD)*180/PI;   //Grādi              
  return angleDBC-PI/2+angleABD;              //Radiāni
}

double calculateAzimuth(double rocketLatitude, double rocketLongitude, double antennaLatitude, double antennaLongitude){
  double diffLon = rocketLongitude*PI/180 - antennaLongitude*PI/180;
  double y = sin(diffLon)*cos(rocketLatitude*PI/180);
  double x = cos(antennaLatitude*PI/180)*sin(rocketLatitude*PI/180) - sin(antennaLatitude*PI/180)*cos(rocketLatitude*PI/180)*cos(diffLon);
  double bearing = atan2(y,x)*(180/PI);
  if (bearing<0) {
    bearing+=360;
  }
  return bearing;
}