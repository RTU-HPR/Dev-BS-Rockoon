#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <RadioLib.h>

#include "angleCalculations.cpp"

#include <esp_now.h>
#include <WiFi.h>


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("\r\nLast Packet Send Status:\t");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

typedef struct struct_message {
    int ELEV;
    int AZIM;
} struct_message;

class rotatorAngles {
    public:
        uint8_t* broadcastAddress;
        uint32_t LoRa_SCK;
        uint32_t LoRa_MISO;
        uint32_t LoRa_MOSI;
        uint32_t LoRa_nss;

        esp_now_peer_info_t peerInfo;
        // Create a struct_message called myData
        struct_message myData;
        bool recv;

        //Antenna location, ierakstīt reālo
        double antennaLongitude = 24.15;
        double antennaLatitude = 56.94;

        //Raķetes starta vērtība, pagaidām nejauši izvēlēta.
        double rocketLongitude = 56.94628;
        double rocketLatitude = 24.10507;
        double rocketAltitude = 100;
        float RSSI = 0;

        //Saglabā jaunās koordinātes kamēr kamēr jāpārbauda korektums
        double rocketLongitudeNew = 0;
        double rocketLatitudeNew = 0;
        double rocketAltitudeNew = 0;

        //Saglagā pagājušo reālo lokāciju backupam kļūdas gadījumā
        double rocketLongitutePrev = 0;
        double rocketLatitudePrev = 0;
        double rocketAltitudePrev = 0;
        int elevationPrev = 0;
        int azimuthPrev = 0;
        bool backupExists = false;  //Mainīgais tiek pārlikts uz true, kad augšā esošie prev mainīgie iegūst reālas koordinātes, nevis placeholderus

        //Maksimālās pieļaujamās mainīgo starpības starp laika posmiem, tos pārsniedzot pieņem, ka kļūdaini dati.
        double longitudeDelta = 5;
        double latitudeDelta = 5;
        double AltitudeDelta = 1000;

        double rotationPrecision = 2; //Antenas precizitāte grādos, abu virzienu vajadzīgā rotācija tiek noapaļota līdz tuvākajam šī skaitļa reizinājumam

        rotatorAngles(uint8_t ba[], uint32_t SCK, uint32_t MISO, uint32_t  MOSI, uint32_t nss) { 
            broadcastAddress = ba; 
            LoRa_SCK = SCK;
            LoRa_MISO = MISO;
            LoRa_MOSI = MOSI;
            LoRa_nss = nss;
        }
        void setup(SX1262 radio) {
            Serial.begin(115200);
            setWifiStation();
            initESPNOW();
            registerAddPeer();
            SPI.begin(LoRa_SCK, LoRa_MISO, LoRa_MOSI, LoRa_nss);
            initRadio(radio);

            //TODO 
            //setAntennaLocation();
            //setRocketInitLoc();
        }
        void loop(SX1262 radio) {
            if (receiveLocation(radio, true) && checkNewLocation()) { //Strādā, jo abi darbojās un izmaina klases mainīgos un returno tikai vai viss ok, nevis vērtības
                updateLocation();
                int elevation = calculateElevAngle(rocketLatitude, rocketLongitude, antennaLatitude, antennaLongitude, rocketAltitude, rotationPrecision);  //Radiānos
                int azimuth = calculateAzimuth(rocketLatitude, rocketLongitude, antennaLatitude, antennaLongitude, rotationPrecision);
                
                printAngles(elevation, azimuth);
                backup(elevation, azimuth);
                send(broadcastAddress, elevation, azimuth);
            } 
            else if(Serial.available()>0){
                manualSend();
            }
            delay(1000);
        }

    private:
        void setWifiStation() {
            // Set device as a Wi-Fi Station
            WiFi.mode(WIFI_STA);
        }
        void initESPNOW(){
            // Init ESP-NOW
            if (esp_now_init() != ESP_OK) {
                Serial.println("Error initializing ESP-NOW");
                return;
            }
            // Once ESPNow is successfully Init, we will register for Send CB to
            // get the status of Trasnmitted packet
            esp_now_register_send_cb(OnDataSent);
        }
        void registerAddPeer() {
            // Register peer
            memcpy(peerInfo.peer_addr, broadcastAddress, 6);
            peerInfo.channel = 0;  
            peerInfo.encrypt = false;
            // Add peer        
            if (esp_now_add_peer(&peerInfo) != ESP_OK){
                Serial.println("Failed to add peer");
                return;
            }
        }
        void initRadio(SX1262 radio) {        
            // initialize SX1262 with default settings
            Serial.print(F("[SX1262] Initializing ... "));
            int state = radio.begin(434.0F, 125.0F, 10U, 5U, 0xFF);
            if (state == RADIOLIB_ERR_NONE)
            {
                Serial.println(F("success!"));
            }
            else
            {
                Serial.print(F("failed, code "));
                Serial.println(state);
                while (true);
            }
        }
        bool receiveLocation(SX1262 radio, bool debug = false){
            String received;
                int state = radio.receive(received);
                if(state == RADIOLIB_ERR_NONE){
                //TODO saglabāt tīros datus (received) SD vai tml., iespējams vajag piefiksēt arī kļūdas (state)
                parseLocation(received, debug);
                RSSI = radio.getRSSI();
                return true;
            }else{
                Serial.println(state);
                return false;
            }
        }
        void parseLocation(String data, bool debug){
            if (debug) {Serial.println("rawData: " + String(data));}

            //String apstrāde
            String delimiter = ",";
            rocketLatitudeNew = data.substring(0, data.indexOf(delimiter)).toDouble();
            data = data.substring(data.indexOf(delimiter)+1);
            rocketLongitudeNew = data.substring(0, data.indexOf(delimiter)).toDouble();
            rocketAltitudeNew = data.substring(data.indexOf(delimiter)+1).toDouble();
            if (debug) {Serial.println("Lat: " + String(rocketLatitudeNew) + "Lng: " + String(rocketLongitudeNew) + "Alt: " + String(rocketAltitudeNew));}

        }
        bool checkNewLocation() {
            return true; //jo šī daļa nav pabeigta, vajag failsafe, ja pirmais nebija pareizs
            // Ja dati ir inherently nepareizi, piem, negatīvas koordinātes
            if (rocketLatitudeNew < 0 || rocketLatitudeNew > 180) {return false;}
            if (rocketLongitudeNew < 0 || rocketLongitudeNew > 360) {return false;}

            // Ja kādas vērtības izmaiņas pārsniedz izvēlētās robežvērtības (piem. pārleca vairākus koordināšu grādus)
            if (abs(rocketAltitude - rocketAltitudeNew) > AltitudeDelta) {return false;}
            if (abs(rocketLatitude - rocketLatitudeNew) > latitudeDelta) {return false;}
            if (abs(rocketLongitude - rocketLongitudeNew) > longitudeDelta) {return false;}

            if (rocketAltitudeNew < 0) {rocketAltitude = 0;}

            return true;
        }
        void updateLocation() {
            // Globālajos mainīgajos ieraksta jaunās vērtības
            rocketLatitude = rocketLatitudeNew;
            rocketLongitude = rocketLongitudeNew;
            rocketAltitude = rocketAltitudeNew;
        }
        void printAngles(int elevation, int azimuth){
            Serial.println("elevation: " + String(elevation) + " azimuth:" + String(azimuth) + " RSSI:" + String(RSSI));
        }
        void backup(int elevation, int azimuth){
            //TODO saglabāt visus šos mainīgos SD vai tml.
            elevationPrev = elevation;
            azimuthPrev = azimuth;
            rocketLongitutePrev = rocketLongitude;
            rocketLatitudePrev = rocketLatitude;
            rocketAltitudePrev = rocketAltitude;
            backupExists = true; //Lai ir zināms, ka ir backups, nevis placeholder vērtības
        }
        void send(uint8_t broadcastAddress[], int elevation, int azimuth) { 
            // Set values to send
            myData.ELEV = elevation;
            myData.AZIM = azimuth;

            // Send message via ESP-NOW
            esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

            if (result == ESP_OK) {
                Serial.println("Sent with success");
            }
            else {
                Serial.println("Error sending the data");
            }
        }
        void manualSend(){
                String a = Serial.readString();
                String b=a;
                int sk = a.indexOf(',');
                String az = a.substring(0,sk);
                String el = b.substring(sk+1, b.length());
                myData.AZIM =  az.toInt();
                myData.ELEV= el.toInt();
                //Serial.println(myData.AZIM + ", " + myData.ELEV);
                Serial.println();
                esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
                if (result == ESP_OK) {
                    Serial.println("Sent with success - " + String(myData.AZIM) + "," + String(myData.ELEV));
                    delay(4000); 
                }
        }
};