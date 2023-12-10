#include <rotator.h>
#include <Arduino.h>

Rotator rotator;

void setup()
{
  rotator.init(rotator.comms, rotator.config);
}

void loop()
{
  rotator.update(rotator.comms, rotator.config);
}