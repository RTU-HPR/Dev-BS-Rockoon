#include <Arduino.h>

#include <config.h>
#include <steppers.h>
#include <gps.h>
#include <communication.h>
#include <lcd.h>

Config config;
Steppers steppers;
Gps gps;
Communication communication;
LCD lcd;

void setup()
{
  // Start the pc Serial
  Serial.begin(config.PC_BAUDRATE);

  // Start the LCD
  lcd.begin(config.LCD_ADDRESS, config.LCD_COLUMNS, config.LCD_ROWS, config.LCD_UPDATE_INTERVAL);
  Serial.println("LCD setup complete");

  // Start the GPS
  Serial1.begin(config.SERIAL1_BAUDRATE);
  Serial1.setPins(config.SERIAL1_RX, config.SERIAL1_TX);
  gps.beginGps(config.gps_config);
  Serial.println("GPS setup complete");

  // Start the steppers
  steppers.beginSteppers(config.azimuth_config, config.elevation_config);
  Serial.println("Stepper setup complete");

  // Start the radio
  config.radio_config.spi_bus->begin(config.SPI0_SCK, config.SPI0_RX, config.SPI0_TX);
  if (!communication.beginRadio(config.radio_config))
  {
    while (true)
    {
      Serial.println("Configuring LoRa failed");
      delay(5000);
    }
  }
  Serial.println("LoRa setup complete");

  // Start the WiFi
  communication.beginWiFi(config.wifi_config);
  Serial.println("WiFi setup complete");

  Serial.println("Setup Complete");
}

void loop()
{
  // Move the steppers
  steppers.moveSteppers();

  // Update the GPS
  gps.readGps();

  // Update the LCD
  lcd.updateLcdScreen(communication.last_radio_packet_millis, communication.connecetedToWiFi);

  // If gps data is valid and required time has passed, send the gps data over UDP
  if (gps.gps.location.isValid() && gps.gps.time.isValid() && millis() - communication.lastRotatorPositionSendMillis > config.GPS_SEND_INTERVAL)
  {
    uint16_t ccsds_packet_length = 0;

    // Create the message
    String data_msg = String(gps.gps.location.lat(), 6) + "," + String(gps.gps.location.lng(), 6) + "," + String(gps.gps.altitude.meters(), 2);

    // Create epoch time from GPS year, month, day, hour, minute, second
    uint32_t epoch_time = gps.gps.date.year() * 31536000 + gps.gps.date.month() * 2592000 + gps.gps.date.day() * 86400 + gps.gps.time.hour() * 3600 + gps.gps.time.minute() * 60 + gps.gps.time.second();
    uint16_t subseconds = map(gps.gps.time.centisecond(), 0, 99, 0, 65535);

    // Create the CCSDS packet
    byte *ccsds = create_ccsds_telemetry_packet(config.ROTATOR_TELEMETRY_APID, gps.rotator_gps_sequence_count, epoch_time, subseconds, data_msg, ccsds_packet_length);

    // Send the message over UDP
    if (communication.connecetedToWiFi && communication.remoteIpKnown)
    {
      communication.tmUdp.beginPacket(communication.tcUdp.remoteIP(), config.wifi_config.tmPort);
      for (size_t i = 0; i < ccsds_packet_length; i++)
      {
        communication.tmUdp.write(ccsds[i]);
      }
      communication.tmUdp.endPacket();

      gps.rotator_gps_sequence_count++;
    }

    // Print the message
    Serial.println("Rotator GPS position sent");

    // Update the last send time
    communication.lastRotatorPositionSendMillis = millis();

    // Free memory
    delete[] ccsds;
  }

  // Variables for the message
  byte *msg = new byte[256];
  uint16_t msg_length = 0;
  float flight_computer_latitude = 0;
  float flight_computer_longitude = 0;
  float flight_computer_altitude = 0;
  float rssi = 0;
  float snr = 0;
  double frequency = 0;
  bool checksum_good = false;

  if (communication.receiveRadio(msg, msg_length, rssi, snr, frequency))
  {
    // Check if checksum matches
    if (check_crc_16_cciit_of_ccsds_packet(msg, msg_length))
    {
      checksum_good = true;
    }

    if (checksum_good)
    {
      // Update the last radio packet time
      communication.last_radio_packet_millis = millis();

      // Print the received message
      Serial.print("TELEMETRY PACKET | HEX: ");
      // Print as hex
      for (size_t i = 0; i < msg_length; i++)
      {
        Serial.print(msg[i], HEX);
        Serial.print(" ");
      }
      Serial.println();

      // Append RSSI and SNR to message
      Converter rssi_converter;
      rssi_converter.f = rssi;
      msg[msg_length - 2] = rssi_converter.b[3];
      msg[msg_length - 1] = rssi_converter.b[2];
      msg[msg_length + 0] = rssi_converter.b[1];
      msg[msg_length + 1] = rssi_converter.b[0];

      Converter snr_converter;
      snr_converter.f = snr;
      msg[msg_length + 2] = snr_converter.b[3];
      msg[msg_length + 3] = snr_converter.b[2];
      msg[msg_length + 4] = snr_converter.b[1];
      msg[msg_length + 5] = snr_converter.b[0];

      msg_length += 6 + 2; // Added 6 bytes for RSSI and SNR, 2 bytes for new checksum

      // Add the new checksum to the message
      add_crc_16_cciit_to_ccsds_packet(msg, msg_length);

      // Parse the message
      uint16_t apid = 0;
      uint16_t sequence_count = 0;
      uint32_t gps_epoch_time = 0;
      uint16_t subseconds = 0;
      byte *packet_data = new byte[msg_length];
      uint16_t packet_data_length = 0;
      parse_ccsds_telemetry(msg, apid, sequence_count, gps_epoch_time, subseconds, packet_data, packet_data_length);

      // Print the message
      if (apid == 100 || apid == 200)
      {
        Converter data_values[6];
        extract_ccsds_data_values(packet_data, data_values, "float,float,float,float,uint32,uint32");

        // Update the flight computer position. TODO: Display on the LCD
        flight_computer_latitude = data_values[0].f;
        flight_computer_longitude = data_values[1].f;
        flight_computer_altitude = data_values[2].f;

        Serial.println("Apid: " + String(apid) + " | Sequence Count: " + String(sequence_count) + " | GPS Epoch: " + String(gps_epoch_time) + "." + String(subseconds) + " | RSSI: " + String(rssi) + " | SNR: " + String(snr) + " | Frequency: " + String(frequency, 8));
        Serial.println("Latitude: " + String(data_values[0].f, 6) + " | Longitude: " + String(data_values[1].f, 6) + " | Altitude: " + String(data_values[2].f));
        Serial.println("Baro Altitude: " + String(data_values[3].f) + " | Satellites: " + String(data_values[4].i32) + " | Info/Error in Queue: " + (data_values[5].i32 == 0 ? "False" : "True"));

        // Update LCD data
        lcd.updateLcdData(flight_computer_latitude, flight_computer_longitude, flight_computer_altitude, rssi, snr);
      }
      else
      {
        Serial.println("Unknown telemetry apid received: " + String(apid));
      }

      // If connected to WiFi send over UDP
      if (communication.connecetedToWiFi && communication.remoteIpKnown)
      {
        communication.tmUdp.beginPacket(communication.tcUdp.remoteIP(), config.wifi_config.tmPort);
        for (size_t i = 0; i < msg_length; i++)
        {
          communication.tmUdp.write(msg[i]);
        }
        communication.tmUdp.endPacket();
      }

      // Free memory
      delete[] packet_data;
    }
    else if (!checksum_good)
    {
      Serial.println("Packet with invalid checksum received!");
    }
    Serial.println();
  }

  // Free memory
  delete[] msg;

  // If connected to WiFi, check for any messages from UDP
  if (communication.connecetedToWiFi)
  {
    int packetSize = communication.tcUdp.parsePacket();
    if (packetSize)
    {
      // Read the packet into packetBuffer
      byte packetBuffer[packetSize];
      communication.tcUdp.read(packetBuffer, packetSize);

      // Create a temporary array to store the packetBuffer without last byte
      // When sending a string over UDP, the last character always got corrupted
      // So a sacrifical char is added to the end of the string and then removed here
      byte tempArray[packetSize - 1];
      for (size_t i = 0; i < packetSize - 1; i++)
      {
        tempArray[i] = packetBuffer[i];
      }

      // Check if heartbeat message was received
      if (memcmp(tempArray, "UDP Heartbeat", sizeof("UDP Heartbeat") - 1) == 0)
      {
        if (!communication.remoteIpKnown)
        {
          communication.remoteIpKnown = true;
          Serial.println("UDP Heartbeat received. Remote IP address is now known: " + communication.tcUdp.remoteIP().toString());
          Serial.println();
        }
        communication.tmUdp.beginPacket(communication.tcUdp.remoteIP(), config.wifi_config.tmPort);
        communication.tmUdp.print("UDP Heartbeat," + String(WiFi.RSSI()));
        communication.tmUdp.endPacket();
        return;
      }

      // Print the received message
      Serial.print("TELECOMMAND PACKET | HEX: ");
      for (size_t i = 0; i < packetSize; i++)
      {
        Serial.print(packetBuffer[i], HEX);
        Serial.print(" ");
      }
      Serial.println();

      // Parse CCSDS telecommand
      packetSize += 2; // +2 for the checksum added later
      byte *packet = new byte[packetSize];
      byte *packet_data = new byte[packetSize];
      uint16_t apid = 0;
      uint16_t sequence_count = 0;
      uint16_t packet_data_length = 0;
      uint16_t packet_id = 0;
      parse_ccsds_telecommand(packetBuffer, apid, sequence_count, packet_id, packet_data, packet_data_length);

      // If the apid is 30, the telecommand is for the rotator angles
      if (apid == 30)
      {
        Serial.println("Rotator angles received");
        Converter data_values[2];
        extract_ccsds_data_values(packet_data, data_values, "float,float");
        Serial.println("Azimuth: " + String(data_values[0].f) + " | Elevation: " + String(data_values[1].f));
        steppers.setRequiredPosition(data_values[0].f, data_values[1].f);
        Serial.println();
        return; // No need to continue
      }

      // YAMCS telecommands are generated without a checksum
      // So the checksum must be added here
      add_crc_16_cciit_to_ccsds_packet(packet, packetSize);

      Serial.print("Adding cheksum! Packet with added checksum: ");
      for (size_t i = 0; i < packetSize; i++)
      {
        Serial.print(packet[i], HEX);
        Serial.print(" ");
      }
      Serial.println();

      // According to the apid do the appropriate action
      if (apid == 10)
      {
        Serial.println("PFC Command received. Sending to Payload flight computer");
        communication.sendRadio(packet, packetSize);
      }
      else if (apid == 20)
      {
        Serial.println("BFC Command received. Sending to Balloon flight computer");
        communication.sendRadio(packet, packetSize);
      }
      else
      {
        Serial.println("Unknown telecommand! Telecommand discarded!");
      }

      Serial.println();
      // Free memory
      delete[] packet;
      delete[] packet_data;
    }
  }
}