This is a code for a simple LoRa transmitter.
At it's current stage it does the following:
    Sends LoRa packets on the default settings;
    The packets contain lat, lon, and alt, first two remain static, altitude is increased by 1 constantly;
What to know before running it:
    Values aren't real.
    pins are set for Heltec WiFi LoRa V3, change if running on something else.
To-Do:
    comments;
    move transmitter function to the header file described in loraOneWayCommsReceiver.
