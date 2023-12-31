#include <angle_calculations.h>

double haversine(double rocketLatitude, double rocketLongitude, double antennaLatitude, double antennaLongitude) {
  unsigned long r = 6371000;
  double d = 2*r*asin(sqrt(pow(sin((rocketLatitude-antennaLatitude)*PI/180/2),2)
    +cos(rocketLatitude*PI/180)*cos(antennaLatitude*PI/180)*pow(sin((rocketLongitude-antennaLongitude)*PI/180/2),2))); //Haversine
  return d;
}

int calculateElevAngle(double rocketLatitude, double rocketLongitude, double rocketAltitude, double antennaLatitude, double antennaLongitude, int precision){   //Izvada leņķi starp pieskari un taisno līniju uz objektu
  unsigned long circumference = 40075000;
  unsigned long r = 6371000;
  double surfaceDistance = haversine(rocketLatitude, rocketLongitude, antennaLatitude, antennaLongitude);

  float angleBAD = surfaceDistance/circumference*2*PI;
  float BD = sqrt(2*pow(r,2)*(1-cos(angleBAD)));       //Attālums starp atrašanās vietām ejo taisnā līnijā caur zemi (ignorējot augstumu)
  double angleABD = (PI-angleBAD)/2.0;                   //leņķis starp rādiusu un līniju caur zemi
  double angleCDB = PI-angleABD;                       //Leņķis starp augstuma taisni un taisni caur zemi
  double BC = sqrt(BD*BD + rocketAltitude*rocketAltitude + BD*rocketAltitude*cos(angleCDB));  //Taisnas līnijas attālums līdz raķetei
  Serial.println("CALC Straight line to ballon: " + String(BC, 6));
  double angleDBC = asin(rocketAltitude*sin(angleCDB)/BC);   //leņķis starp caur zemi esošo līniju un taisno līniju līdz objektam
  Serial.println("CALC Angle between earth line and balloon: " + String(angleDBC, 6));
  double outputAngle = (angleDBC-PI/2+angleABD);
  double outputAngleDegrees = outputAngle*180/PI;
  Serial.println("CALC Output angle: " + String(outputAngleDegrees, 6));
  int rounded = round(outputAngleDegrees/precision)*precision;
  Serial.println("CALC Output angle rounded: " + String(rounded));
  return rounded;
}

int calculateAzimuth(double rocketLatitude, double rocketLongitude, double antennaLatitude, double antennaLongitude, double precision){
  double diffLon = rocketLongitude*PI/180 - antennaLongitude*PI/180;
  double y = sin(diffLon)*cos(rocketLatitude*PI/180);
  double x = cos(antennaLatitude*PI/180)*sin(rocketLatitude*PI/180) - sin(antennaLatitude*PI/180)*cos(rocketLatitude*PI/180)*cos(diffLon);
  double outputBearing = atan2(y,x);
  double outputBearingDegrees = outputBearing*180/PI;
  int rounded = round(outputBearingDegrees/precision)*precision;
  if (rounded<0) {
    rounded+=360;
  }
  return 360-rounded;
}
