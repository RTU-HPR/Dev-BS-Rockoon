#include <Arduino.h>
#include <esp_now_wrapper.h>

void setup()
{
    Serial.begin(115200);
    while (!Serial)
    {
        delay(1);
    }
    esp_now_wrapper::init(true, 0);
}

void loop()
{
    String result = esp_now_wrapper::read();
    if (result != "")
    {
        result.trim();
        result += "\n";
        Serial.println(result);
    }

    if (Serial.available() > 0)
    {
        String msg = Serial.readString();
        esp_now_wrapper::send(msg);
    }
}
