#include <rotator.h>
#include <angle_calculations.h>

// PRIVATE FUNCTIONS
void Rotator::update_location(Comms &comms, TRACKABLE_OBJECT_POSITION &ballon_position)
{
  // Make a char array from the message
  String msg = comms.received_data.msg;
  msg.trim();
  int msg_len = msg.length() + 1;
  char char_array[msg_len];
  msg.toCharArray(char_array, msg_len);

  char *token = strtok(char_array, ",");
  ballon_position.latitude = atof(token);
  token = strtok(NULL, ",");
  ballon_position.longitude = atof(token);
  token = strtok(NULL, ",");
  ballon_position.altitude = atof(token);
}

void Rotator::update_angles(Comms &comms, TRACKABLE_OBJECT_POSITION &ballon_position, Config &config)
{
  comms.data_to_rotator.elevation = calculateElevAngle(ballon_position.latitude, 
                                      ballon_position.longitude,
                                      config.rotator_location.LATITUDE,
                                      config.rotator_location.LONGITUDE, 
                                      ballon_position.altitude, 
                                      config.ROTATION_PRECISION);
  comms.data_to_rotator.azimuth = calculateAzimuth(ballon_position.latitude, 
                                    ballon_position.longitude,
                                    config.rotator_location.LATITUDE,
                                    config.rotator_location.LONGITUDE,  
                                    config.ROTATION_PRECISION);
  manualPosition.azimuth = calculateAzimuth(ballon_position.latitude, 
                                    ballon_position.longitude,
                                    config.rotator_location.LATITUDE,
                                    config.rotator_location.LONGITUDE,  
                                    config.ROTATION_PRECISION);
  manualPosition.azimuth =calculateElevAngle(ballon_position.latitude, 
                                      ballon_position.longitude,
                                      config.rotator_location.LATITUDE,
                                      config.rotator_location.LONGITUDE, 
                                      ballon_position.altitude, 
                                      config.ROTATION_PRECISION);
}

// PUBLIC FUNCTIONS
void Rotator::init(Comms &comms, Config &config)
{
  // Init serial/radio communications
  comms.init(config);
  //gps.initGPS();
  //gps.readGPS();
  //config.rotator_location.LATITUDE = gps.data.gpsLat;
//config.rotator_location.LONGITUDE = gps.data.gpsLon;
 // Serial.println("Rotator location set to lat: " + String(config.rotator_location.LATITUDE) + " Lon: " + String(config.rotator_location.LONGITUDE));
  Serial.println("Rotator setup completed successfully!");
  Serial.println();
}
bool Rotator::newData(Comms &comms, Config &config){
   // Check if a new position has been received from LoRa
  if (comms.lora_receive(comms.received_data))
  {
    // Update ballon location from received message
    Serial.println("Message stored in received data: " + comms.received_data.msg);
    update_location(comms, ballon_position);
    Serial.println("Ballon latitude: " + String(ballon_position.latitude));
    Serial.println("Ballon longitude: " + String(ballon_position.longitude));
    Serial.println("Ballon altitude: " + String(ballon_position.altitude));

    // Update angles from the new position
    update_angles(comms, ballon_position, config);
    return true;
  }else{
    return false;
  }
}
bool Rotator::manualInput(Comms &comms, Config &config){
  if(Serial.available() > 0)
  {
    String msg = Serial.readString();
    Serial.println("Message is: " + msg);
    int msg_len = msg.length() + 1;
    char char_array[msg_len];
    msg.toCharArray(char_array, msg_len);

    char *token = strtok(char_array, ",");
    ballon_position.latitude = atof(token);
    token = strtok(NULL, ",");
    ballon_position.longitude = atof(token);
    token = strtok(NULL, ",");
    ballon_position.altitude = atof(token);
    
    update_angles(comms, ballon_position, config);
    return true;
  }
return false;
}
//void Rotator::update(Comms &comms, Config &config)
//{
  // // Flag to know if new angles have been calculated
  // bool new_data = false;

  // // Check if a new position has been received from LoRa
  // if (comms.lora_receive(comms.received_data))
  // {
  //   // Update ballon location from received message
  //   Serial.println("Message stored in received data: " + comms.received_data.msg);
  //   update_location(comms, ballon_position);
  //   Serial.println("Ballon latitude: " + String(ballon_position.latitude));
  //   Serial.println("Ballon longitude: " + String(ballon_position.longitude));
  //   Serial.println("Ballon altitude: " + String(ballon_position.altitude));

  //   // Update angles from the new position
  //   update_angles(comms, ballon_position, config);
  //   new_data = true;
  // }
  // // Check if a new position has been entered manually 
  // else if(Serial.available() > 0)
  // {
  //   String msg = Serial.readString();
  //   Serial.println("Message is: " + msg);
  //   int msg_len = msg.length() + 1;
  //   char char_array[msg_len];
  //   msg.toCharArray(char_array, msg_len);

  //   char *token = strtok(char_array, ",");
  //   ballon_position.latitude = atof(token);
  //   token = strtok(NULL, ",");
  //   ballon_position.longitude = atof(token);
  //   token = strtok(NULL, ",");
  //   ballon_position.altitude = atof(token);
    
  //   update_angles(comms, ballon_position, config);
  //   new_data = true;
  // }

  // // If a new angle has been calculated, send it to the rotator
  //  if (new_data)
  // {
  // //   Serial.println("Elevation: " + String(comms.data_to_rotator.elevation) + " | Azimuth: " + String(comms.data_to_rotator.azimuth));
    
  // //   // Send new angle data to rotator
  // //   if (comms.getAngles(comms))
  // //   {
  // //     Serial.println("New angles gotten");
  // //   }
  // //   else
  // //   {
  // //     Serial.println("New angle sending to rotator failed!");
  // //   }
    
  // //   Serial.println();
  // //   new_data = false;
  //   state = true;
  //  }else{
  //   state = false;
  //  }
//}