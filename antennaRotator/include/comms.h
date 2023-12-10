#pragma once
#include <config.h>

#include <SPI.h>
#include <RadioLib.h>
//#include <esp_now.h>
//#include <WiFi.h>

class Comms
{
  private:
  // LoRa pins
  const int LORA_MOSI = 10;
  const int LORA_MISO = 11;
  const int LORA_SCK = 9;
  const int LORA_NSS = 8;
  const int LORA_DIO1 = 14;
  const int LORA_NRST = 12;
  const int LORA_BUSY = 13;

  // LoRa object
  SX1262 lora = new Module(LORA_NSS, LORA_DIO1, LORA_NRST, LORA_BUSY);

  // ESP-NOW
  //esp_now_peer_info_t esp_now_peer_info;
 // uint8_t ESP_MAC_ADDRESS[6] = {212, 212, 218, 229, 111, 164}; // REPLACE WITH YOUR RECEIVER MAC Address
  
  // Function declaration
  void init_serial(Config &config);
 // bool init_esp();
  bool init_lora();

  public:
  // Message structure
  struct MESSAGE_TO_ROTATOR_STRUCTURE 
  {
    int elevation;
    int azimuth;
  };
  MESSAGE_TO_ROTATOR_STRUCTURE data_to_rotator;

  // Received message structure
  struct RECEIVED_MESSAGE_STRUCTURE
  {
    String msg = "";
    float rssi;
    float snr;
  };
  RECEIVED_MESSAGE_STRUCTURE received_data;

  // Function declaration
  void init(Config &config);
  bool lora_receive(RECEIVED_MESSAGE_STRUCTURE &received_data);
  bool getAngles(Comms &comms);
  //bool send_data_to_rotator(Comms &comms);
};