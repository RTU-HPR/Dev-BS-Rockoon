#include <Arduino.h>

#include <config.h>
#include <steppers.h>
#include <gps.h>
#include <communication.h>

#include <Ccsds_packets.cpp>

Config config;
Steppers steppers;
Gps gps;
Communication communication;

void setup()
{
  // Start the pc Serial
  Serial.begin(config.PC_BAUDRATE);

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

  // If gps data is valid and required time has passed, send the gps data over UDP
  if (gps.gps.location.isValid() && gps.gps.time.isValid() && millis() - communication.lastRotatorPositionSendMillis > config.GPS_SEND_INTERVAL)
  {
    // Create the message
    String msg = config.ROTATOR_TELEMETRY_MESSAGE_HEADER_APID + "," + String(communication.rotatorPositionMessageIndex) + ",";
    msg += String(gps.gps.location.lat(), 6) + "," + String(gps.gps.location.lng(), 6) + "," + String(gps.gps.altitude.meters(), 2);
    // String msg = config.ROTATOR_TELEMETRY_MESSAGE_HEADER_APID + "," + String(communication.rotatorPositionMessageIndex) + ",";
    // msg += String((float)random(50, 60), 6) + "," + String((float)random(22, 26), 6) + "," + String((float)random(1000, 25000), 2);

    // Convert String to char array
    char msgArray[msg.length()];
    msg.toCharArray(msgArray, sizeof(msgArray));

    // Send the message over UDP
    if (communication.connecetedToWiFi && communication.remoteIpKnown)
    {
      communication.tmUdp.beginPacket(communication.tcUdp.remoteIP(), config.wifi_config.tmPort);
      communication.tmUdp.print(msgArray);
      communication.tmUdp.endPacket();
    }

    // Print the message
    Serial.println("Rotator GPS position UDP packet sent: " + msg);

    // Update the last send time
    communication.lastRotatorPositionSendMillis = millis();
  }

  // Variables for the message
  byte *msg = new byte[256];
  uint16_t msg_length = 0;
  float rssi = 0;
  float snr = 0;
  double frequency = 0;
  bool checksum_good = false;

  if (communication._radio->receive_bytes(msg, msg_length, rssi, snr, frequency))
  {
    // Check if checksum matches
    if (check_crc_16_cciit_of_ccsds_packet(msg, msg_length))
    {
      checksum_good = true;
    }

    if (checksum_good)
    {
      // Print the received message
      Serial.print("RSSI: " + String(rssi) + " | SNR: " + String(snr) + " | FREQUENCY: " + String(frequency, 8) + " | MSG: ");
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
      byte *packet_data = new byte[msg_length - 12];
      uint16_t packet_data_length = 0;
      parse_ccsds(msg, apid, sequence_count, gps_epoch_time, subseconds, packet_data, packet_data_length);
      
      // Print the message
      if (apid == 100 || apid == 200)
      {
        Serial.print("Telemetry message received: ");
        Converter data_values[6];
        extract_ccsds_data_values(packet_data, data_values, "float,float,float,float,uint32,uint32");
        for (size_t i = 0; i < 6; i++)
        {
          if (i == 4 || i == 5)
          {
            Serial.print(data_values[i].i32);
          }
          else
          {
            if (i == 0 || i == 1)
            {
              Serial.print(data_values[i].f, 6);
            }
            else
            {
              Serial.print(data_values[i].f, 2);
            }
          }
          Serial.print(" ");
        }
        Serial.println();
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

        Serial.println("UDP packet sent");
      }

      // Free memory
      delete[] packet_data;
    }
    else if (!checksum_good)
    {
      Serial.println("Packet with invalid checksum received!");
    }
  }
  
  // Free memory
  delete[] msg; // VERY IMPORTANT, otherwise a significant memory leak will occur

  // If connected to WiFi, check for any messages from UDP
  if (communication.connecetedToWiFi)
  {
    int packetSize = communication.tcUdp.parsePacket();
    if (packetSize)
    {
      // Read the packet into packetBuffer
      byte packetBuffer[packetSize];
      communication.tcUdp.read(packetBuffer, packetSize);

      // Remove the last byte from the packet buffer
      // The last byte is a sacrifical one to solve a bug where the last byte was corrupted
      packetBuffer[packetSize - 1] = '\0';
      packetSize -= 1;

      // Check if heartbeat message was received
      if (String((char*)packetBuffer) == "UDP Heartbeat")
      {
        if (!communication.remoteIpKnown)
        {
          communication.remoteIpKnown = true;
          Serial.println("UDP Heartbeat received. Remote IP address is now known: " + communication.tcUdp.remoteIP().toString());
        }
        communication.tmUdp.beginPacket(communication.tcUdp.remoteIP(), config.wifi_config.tmPort);
        communication.tmUdp.print("UDP Heartbeat," + String(WiFi.RSSI()));
        communication.tmUdp.endPacket();
        return;
      }

      // A CCSDS Telecommand packet was received
      uint16_t apid = 0;
      uint16_t sequence_count = 0;
      uint32_t gps_epoch_time = 0;
      uint16_t subseconds = 0;
      byte *packet_data = new byte[packetSize - 12];
      uint16_t packet_data_length = 0;
      parse_ccsds(packetBuffer, apid, sequence_count, gps_epoch_time, subseconds, packet_data, packet_data_length);

      // According to the apid do the appropriate action
      if (apid == 10)
      {
        Serial.println("PFC Command received. Sending to Payload flight computer");
        communication._radio->transmit_bytes(packetBuffer, packetSize);
      }
      else if (apid == 20)
      {
        Serial.println("BFC Command received. Sending to Balloon flight computer");
        communication._radio->transmit_bytes(packetBuffer, packetSize);
      }
      else if (apid == 30)
      {
        Serial.println("Rotator angles received");
        Converter data_values[2];
        extract_ccsds_data_values(packet_data, data_values, "float,float");
        steppers.setRequiredPosition(data_values[0].f, data_values[1].f);
      }
      else
      {
        Serial.println("Unknown telecommand! Telecommand discarded!");
      }
      // Free memory
      delete[] packet_data;
    }
  }
}