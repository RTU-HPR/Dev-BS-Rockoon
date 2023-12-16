#include <Arduino.h>
#include <esp_now_wrapper.h>

unsigned long lastSend = 0;
int i = 1;

void setup()
{
    Serial.begin(115200);
    esp_now_wrapper::init(false, 0);
}

void loop()
{
    String result = esp_now_wrapper::read();
    if (result != "")
    {
        Serial.println(result);
    }

    if (millis() - lastSend > 5000)
    {
        String msg =  "rtu_vip_bfc," + String(i) + ",18:04:43,55.123456,24.987654,123.45,25.67,10,123000,10.5,123.45,15.123456,14.987654,20.52,-50,-2";

        esp_now_wrapper::send(msg);
        delay(10);

        i++;
        lastSend = millis();
    }
}
