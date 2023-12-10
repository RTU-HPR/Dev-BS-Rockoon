#include <Arduino.h>
//Code that that prints out and seperates a string for adding the BS later

// Function to print data captions and values
void printData(const String* captions, const String* values, size_t size) {
  Serial.print("Data: ");
  for (size_t i = 0; i < size; i++) {
    Serial.print(captions[i]);
    Serial.print(": ");
    Serial.print(values[i]);
    Serial.print(", ");
  }
  Serial.println();
}

// Function to parse a string containing comma-separated values as strings
void parseString(const String& input, String* values, size_t maxSize) {
  int startIndex = 0;
  int endIndex = input.indexOf(',');
  size_t index = 0;

  while (endIndex != -1 && index < maxSize) {
    // Extract each substring
    values[index] = input.substring(startIndex, endIndex);

    // Move to the next substring
    startIndex = endIndex + 1;
    endIndex = input.indexOf(',', startIndex);
    index++;
  }

  // Process the last substring
  if (index < maxSize) {
    values[index] = input.substring(startIndex);
  }
}

void setup() {
  Serial.begin(115200);
  String longitude;
  String latitude;
  String altitude;
  // Example usage
  String captions[] = {"Callsign","UnixTime","RSSI", "Latitude", "Longitude", "Altitude", "Speed", "Satellites", "Temperature", "Pressure", "Barometric Altitude"};
  String inputString = {"RTU VIP,18.04,-50,40.123456,-75.987654,123.45,25.5,10,25.67,950.20,100.0"}; //here goes the received LoRa packet
  constexpr size_t maxSize = 13; //number of variables in the message
  String values[maxSize];

  // Call the function to parse the string to and array
  parseString(inputString, values, maxSize);

  // Call the function to print data captions and values
  printData(captions, values, sizeof(captions) / sizeof(captions[0]));

  latitude = values[4]; //these go as an input for angle calculations code
  longitude = values[5];
  altitude = values[6];
}

void loop() {
  // The loop function is not used in this example
}
