#include <RadioLib.h>
#include <Arduino.h>
#include <SPI.h>
//pin definitions
#define loraSCK 9
#define loraSS 8
#define loraRST 12
#define loraBUSY 13 
#define loraDIO1 14
#define loraMISO 11
#define loraMOSI 10

SX1262 radio = new Module(loraSS, loraDIO1, loraRST, loraBUSY);
namespace lora
{
    void setup(){   
        Serial.begin(115200);
       SPI.begin(loraSCK, loraMISO, loraMOSI, loraSS);
       Serial.println("Initalising LoRa");
       radio.begin(434, 125, 9, 7, 0x34, 10);
    }
    void receive(){
        String received;
        int state = radio.receive(received);
        if(state == RADIOLIB_ERR_NONE){
            Serial.println(received);
        }else{
            Serial.println("error");
        }
    }
    void send(String message){
        Serial.print("Transmitting:");
        int state = radio.transmit(message);
        if(state == RADIOLIB_ERR_NONE){
            Serial.println("success");
        }else if(state == RADIOLIB_ERR_PACKET_TOO_LONG){
            Serial.println("failed, packet too long");
        }
        
    }
}