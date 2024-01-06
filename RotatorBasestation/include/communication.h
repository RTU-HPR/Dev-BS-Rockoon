#pragma once
#include <config.h>

class Communication
{
public:
  RadioLib_Wrapper<radio_module> *_radio;

  // Wifi
  WiFiUDP tcUdp;
  WiFiUDP tmUdp;

  bool connecetedToWiFi = false;

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
  bool sendError(String errorString);

  void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info);
  void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);

  void beginWiFi(WiFi_config &wifi_config);
  void parseString(String &input, String *values, size_t maxSize);
};
