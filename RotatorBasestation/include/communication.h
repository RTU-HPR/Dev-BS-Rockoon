#pragma once
#include <config.h>

class Communication
{
public:
  RadioLib_Wrapper<radio_module> *_radio;

  // Wifi
  WiFiUDP tcUdp;
  WiFiUDP tmUdp;

  // Config needs to be copied to this variable in beginWiFi()
  // for us to be able to access config values in WiFi events
  Config::WiFi_Config wifi_config;

  bool connecetedToWiFi = false;
  bool remoteIpKnown = false;
  unsigned long lastUdpReceivedMillis = 0;

  int rotatorPositionMessageIndex = 1;
  unsigned long lastRotatorPositionSendMillis = 0;

  /**
   * @brief Initialise the Communication Radio
   * @param config Payload config object
   */
  bool beginRadio(RadioLib_Wrapper<SX1262>::Radio_Config &radio_config);

  /**
   * @brief Sends the provided message using LoRa
   * @param msg Message to send
   * @return Whether the message was sent successfully
   */
  bool sendRadio(String msg);

  void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info);
  void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);

  void beginWiFi(Config::WiFi_Config &wifi_config);
  void parseString(String &input, String *values, size_t maxSize);
};
