#include <Arduino.h>
#include <esp_now_wrapper.h>
// path: src/main.cpp
// path: include/esp_now_wrapper.h

bool board1 = true;
void setup()
{
    Serial.begin(115200);
    if (esp_now_wrapper::init(board1, 12))
    {
        Serial.println("esp_now_wrapper::init() success");
    }
    else
    {
        Serial.println("esp_now_wrapper::init() failed");
        while (true)
        {
        };
    }
}

void loop()
{
    String result = esp_now_wrapper::read();
    if (result != "")
    {
        Serial.println("Here");
        result.trim();
        Serial.println(result);
        delay(100);

        esp_now_wrapper::send("Received: " + result);
        Serial.println("Sent data");
    }
}
