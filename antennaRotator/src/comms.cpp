#include <comms.h>

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

bool Comms::getAngles(Comms &comms)
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
  return true;
}