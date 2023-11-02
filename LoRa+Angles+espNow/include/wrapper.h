#include <Arduino.h>

#define LoRa_MOSI 10
#define LoRa_MISO 11
#define LoRa_SCK 9

#define LoRa_nss 8
#define LoRa_dio1 14
#define LoRa_nrst 12
#define LoRa_busy 13

// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {212, 212, 218, 229, 111, 164};