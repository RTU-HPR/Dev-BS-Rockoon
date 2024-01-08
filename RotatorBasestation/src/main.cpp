#include <Arduino.h>

#include <config.h>
#include <steppers.h>
#include <gps.h>
#include <communication.h>

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
  // Update the GPS
  gps.readGps();

  // If gps data is valid and required time has passed, send the gps data over UDP
  if (gps.gps.location.isValid() && gps.gps.time.isValid() && millis() - communication.lastRotatorPositionSendMillis > config.GPS_SEND_INTERVAL)
  {
    // Create the message
    String msg = config.ROTATOR_TELEMETRY_MESSAGE_HEADER_APID + "," + String(communication.rotatorPositionMessageIndex) + ",";
    msg += String(gps.gps.location.lat(), 7) + "," + String(gps.gps.location.lng(), 7) + "," + String(gps.gps.altitude.meters(), 2);

    // Convert String to char array
    char msgArray[msg.length()];
    msg.toCharArray(msgArray, sizeof(msgArray));

    // Send the message over UDP
    communication.tmUdp.beginPacket(config.wifi_config.remoteIP, config.wifi_config.tmPort);
    communication.tmUdp.print(msgArray);
    communication.tmUdp.endPacket();

    // Print the message
    Serial.println("Rotator GPS position UDP packet sent: " + msg);

    // Update the last send time
    communication.lastRotatorPositionSendMillis = millis();
  }

  // Variables for the message
  String msg = "";
  float rssi = 1;
  float snr = 0;
  double frequency = 0;
  bool checksum_good = false;

  // Check for any messages from Radio
  if (communication._radio->receive(msg, rssi, snr, frequency))
  {
    // Check if checksum matches
    if (communication._radio->check_checksum(msg))
    {
      checksum_good = true;
    }
  }

  // Check for any messages from PC
  if (Serial.available() > 0)
  {
    // Read the message from the Serial port
    msg = Serial.readString();
    // Remove any line ending symbols
    msg.trim();

    // Set the flags
    checksum_good = true;
  }

  // Check if the message is not empty and the checksum is good
  if (msg != "" && checksum_good)
  {
    // Print the received message
    if (rssi < 0)
    {
      Serial.print("RADIO MESSAGE | RSSI: " + String(rssi) + " | SNR: " + String(snr) + " FREQUENCY: " + String(frequency, 8) + " | MSG: ");
    }
    else
    {
      Serial.print("PC COMMAND | MSG: ");
    }
    Serial.println(msg);

    // Append RSSI and SNR to message
    msg += "," + String(rssi, 2) + "," + String(snr, 2);

    // If connected to WiFi send over UDP
    if (communication.connecetedToWiFi)
    {
      // Convert String to char array
      char msgArray[msg.length()];
      msg.toCharArray(msgArray, sizeof(msgArray));

      communication.tmUdp.beginPacket(config.wifi_config.remoteIP, config.wifi_config.tmPort);
      communication.tmUdp.print(msgArray);
      communication.tmUdp.endPacket();

      Serial.println("UDP packet sent");
    }
  }
  else if (msg != "" && !checksum_good)
  {
    Serial.println("Command with invalid checksum received: " + msg);
  }

  // If connected to WiFi, check for any messages from UDP
  if (communication.connecetedToWiFi)
  {
    int packetSize = communication.tcUdp.parsePacket();
    if (packetSize)
    {
      // Read the packet into packetBuffer
      char packetBuffer[packetSize];
      communication.tcUdp.read(packetBuffer, packetSize);

      // Remove the last character from the packet buffer
      // The last character is a sacrifical character to solve a bug where the last character was corrupted
      packetBuffer[packetSize - 1] = '\0';

      // Create a string from the packet buffer
      String receivedMsg = String(packetBuffer);
      // Print the received message
      Serial.println("UDP packet received: " + receivedMsg);

      // Get the callsign from the message
      int commaIndex = receivedMsg.indexOf(',');
      String callsign = receivedMsg.substring(0, commaIndex);

      // If callsign is rtu_rotator, parse the message and set the required position
      if (callsign == "rtu_rotator")
      {
        Serial.println("Rotator angles received");
        String message_string_values[5];
        communication.parseString(receivedMsg, message_string_values, 5);
        steppers.setRequiredPosition((message_string_values[1]).toFloat(), (message_string_values[2]).toFloat());
      }
      // If callsign is rtu_pfc or rtu_bfc, send the message to the appropriate flight computer
      else if (callsign == "rtu_pfc")
      {
        Serial.println("PFC Command received. Sending to Payload flight computer");
        communication.sendRadio(receivedMsg);
      }
      else if (callsign == "rtu_bfc")
      {
        Serial.println("BFC Command received. Sending to Balloon flight computer");
        communication.sendRadio(receivedMsg);
      }
      else
      {
        Serial.println("Unknown callsign! Message discarded");
      }
    }

    // Move the steppers
    steppers.moveSteppers();
  }
}