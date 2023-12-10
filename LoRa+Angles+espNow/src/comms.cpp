#include <comms.h>

// CALLBACK FUNCTIONS
// Callback function when data is sent
void on_data_sent(const uint8_t *mac_addr, esp_now_send_status_t status) 
{
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// PRIVATE FUNCTIONS
void Comms::init_serial(Config &config)
{
  Serial.begin(config.PC_BAUDRATE);
  if (config.WAIT_FOR_PC)
  {
    while (!Serial)
    {
      delay(100);
    }
  }
}

bool Comms::init_esp()
{
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return false;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of transmitted packet
  esp_now_register_send_cb(on_data_sent);
  
  // Register peer
  memcpy(esp_now_peer_info.peer_addr, ESP_MAC_ADDRESS, 6);
  esp_now_peer_info.channel = 0;  
  esp_now_peer_info.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&esp_now_peer_info) != ESP_OK){
    Serial.println("Failed to add peer");
    return false;
  }

  return true;
}

bool Comms::init_lora()
{
  // Start the SPI bus
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);
  
  // Initialize SX1262 with default settings
  int state = lora.begin();
  /*
  lora.setFrequency(434.5);
  lora.setOutputPower(14);
  lora.setSpreadingFactor(10);
  lora.setCodingRate(7);
  lora.setBandwidth(125);
  lora.setSyncWord(0xF4);
  */
  if (state != RADIOLIB_ERR_NONE)
  {
    Serial.println("LoRa initialization failed with error: " + String(state));
    return false;
  }

  // Start listening to LoRa messages
  state = lora.startReceive();
  if (state != RADIOLIB_ERR_NONE)
  {
    Serial.println("LoRa receive start failed: " + String(state));
    return false;
  }

  return true;
}

// PUBLIC FUNCTIONS
void Comms::init(Config &config)
{
  // Init Serial communication with PC
  init_serial(config);

  // Init ESP-NOW
  if (init_esp())
  {
    Serial.println("ESP-NOW initialized successfully!");
  }
  else
  {
    Serial.println("ESP-NOW initialization failed!");
    Serial.println("Stopping further execution!");
    while(true){}
  }

  // Init LoRa
  if (init_lora())
  {
    Serial.println("LoRa initialized successfully!");
  }
  else
  {
    Serial.println("LoRa initialization failed!");
    Serial.println("Stopping further execution!");
    while(true){}
  }
}

bool Comms::lora_receive(RECEIVED_MESSAGE_STRUCTURE &received_data)
{
  // Read data from LoRa
  String str;

  int state = lora.receive(str);
  if (state != RADIOLIB_ERR_NONE && state != -6)
  {
    Serial.println("LoRa receive error: " + String(state));
    return false;
  }

  if (str == "")
  {
    return false;
  }

  received_data.msg = str;
  received_data.rssi = lora.getRSSI();
  received_data.snr = lora.getSNR();
  Serial.println("Received data: " + received_data.msg);
  Serial.println("RSSI: " + String(received_data.rssi));
  Serial.println("SNR: " + String(received_data.snr));

  // Start receiving again
  // lora.finishTransmit();
  state = lora.startReceive();
  if (state != RADIOLIB_ERR_NONE)
  {
    Serial.println("LoRa receive restart failed: " + String(state));
    return false;
  }

  return true;
}

bool Comms::send_data_to_rotator(Comms &comms)
{
  int azimuth = comms.data_to_rotator.azimuth;
  int elevation = comms.data_to_rotator.elevation;
  struct Data
  {
    int AZIM;
    int ELEV;
  };
  Data data;

  data.AZIM = azimuth;
  data.ELEV = elevation;
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(ESP_MAC_ADDRESS, (uint8_t*) &data, sizeof(data));
  
  if (result != ESP_OK)
  {
    Serial.println("ESP-NOW failed sending data to rotator with error: " + String(result));
    return false;
  }
  return true;
}