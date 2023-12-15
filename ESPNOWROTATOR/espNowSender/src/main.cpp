#include <Arduino.h>
#include <esp_now_wrapper.h>
// path: src/main.cpp
// path: include/esp_now_wrapper.h

bool board1 = false;
void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        delay(5);
    }
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
        Serial.println(result);
    }

    if (Serial.available() > 0)
    {
        String input = Serial.readString();
        Serial.println(input);
        if (!esp_now_wrapper::send(input))
        {
            Serial.println("esp_now_wrapper::send() failed");
        }
        delay(100);
    }
}
