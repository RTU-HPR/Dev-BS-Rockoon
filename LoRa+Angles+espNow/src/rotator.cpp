#include <rotator.h>
#include <angle_calculations.h>

// PRIVATE FUNCTIONS
void Rotator::update_location(String msg, TRACKABLE_OBJECT_POSITION &ballon_position)
{
  // Make a char array from the message
  int msg_len = msg.length() + 1;
  char char_array[msg_len];
  msg.toCharArray(char_array, msg_len);

  int result = sscanf(char_array,                 // The char array to read from
                      "%d,%d",                    // The form the variables are separated in string
                      &ballon_position.latitude,  // The values to update that correspond to the variable in string
                      &ballon_position.longitude,
                      &ballon_position.altitude);
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
}

// PUBLIC FUNCTIONS
void Rotator::init(Comms &comms, Config &config)
{
  // Init serial/radio communications
  comms.init(config);

  Serial.println("Rotator setup completed successfully!");
  Serial.println();
}

void Rotator::update(Comms &comms, Config &config)
{
  // Flag to know if new angles have been calculated
  bool new_data = false;

  // Check if a new position has been received from LoRa
  if (comms.lora_receive(comms.received_data))
  {
    // Update angles from the new position
    update_angles(comms, ballon_position, config);
    new_data = true;
  }
  // Check if a new position has been entered manually 
  else if(Serial.available() > 0)
  {
    String a = "";
    String b = "";
    int separator_pos = 0;

    a = Serial.readString();
    b = a;

    separator_pos = a.indexOf(',');
    String azimuth = a.substring(0,separator_pos);
    String elevation = b.substring(separator_pos+1, b.length());
    comms.data_to_rotator.azimuth =  azimuth.toInt();
    comms.data_to_rotator.elevation= elevation.toInt();
    new_data = true;
  }

  // If a new angle has been calculated, send it to the rotator
  if (new_data)
  {
    Serial.println("Time: " + String(millis()/1000, 0) + " | Elevation: " + String(comms.data_to_rotator.elevation) + " | Azimuth: " + String(comms.data_to_rotator.azimuth));
    
    // Send new angle data to rotator
    if (comms.send_data_to_rotator(comms))
    {
      Serial.println("New angles successfully sent to rotator!");
    }
    else
    {
      Serial.println("New angle sending to rotator failed!");
    }

    Serial.println();
  }
}