#pragma once
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>

namespace esp_now_wrapper
{
    uint8_t broadcastAddress[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    esp_now_peer_info_t peerInfo;
    String buf_recv = "";
    String buf_send = "";
    bool initialized = false;

    void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
    {
        //Serial.println("Sent");
        // Serial.println("Data sent");
        // BROKEN. SENDS DATA SUCCESSFULLY BUT RETURNS -1 STATUS CODE
        // if (status == ESP_NOW_SEND_SUCCESS)
        // {
        //     Serial.println("Send success");
        // }
        // else
        // {
        //     Serial.println(status);
        //     Serial.println("Send failed");
        // }
    }

    void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
    {
        // copy incoming data to buf_recv which is a string
        String data((const char *)(incomingData), len);
        buf_recv += data;
    }
    /**
     * @brief Set the up espnow object
     *
     * @param board1  true if this is board 1, false if this is board 2
     * @param wifi_channel the wifi channel to use either 12 or 13 both board need the same channel
     */
    bool init(bool board1 = true, int wifi_channel = 0)
    {
        WiFi.mode(WIFI_STA);

        Serial.print("ESP32 MAC Address: ");
        Serial.println(WiFi.macAddress());

        // if (esp_wifi_set_channel(wifi_channel, WIFI_SECOND_CHAN_NONE) != ESP_OK)
        // {
        //     Serial.println("Error changing WiFi channel");
        //     return false;
        // }

        if (esp_now_init() != ESP_OK)
        {
            Serial.println("Error initializing ESP-NOW");
            return false;
        }
        esp_now_register_send_cb(OnDataSent);
        // choose which mac address to use
        if (board1)
        {
            // Rotator esp
            uint8_t temp[] = {0xF4, 0x12, 0xFA, 0x6A, 0x1D, 0x18};
            memcpy(broadcastAddress, temp, sizeof(temp));
        }
        else
        {
            // Different esp
            uint8_t temp[] = {0xF4, 0x12, 0xFA, 0x6A, 0x53, 0x80};
            memcpy(broadcastAddress, temp, sizeof(temp));
        }

        memcpy(peerInfo.peer_addr, broadcastAddress, 6);
        peerInfo.channel = 0;
        peerInfo.encrypt = false;

        if (esp_now_add_peer(&peerInfo) != ESP_OK)
        {
            Serial.println("Failed to add peer");
            return false;
        }

        esp_now_register_recv_cb(OnDataRecv);
        initialized = true;
        return true;
    }
    /**
     * @brief
     *
     * @param msg
     * @return true msg sent successfully
     * @return false msg failed to send
     */
    bool send(const String &msg)
    {
        if (initialized == false)
        {
            Serial.println("ESP-NOW not initialized");
            return false;
        }
        // convert msg to uint8_t array
        uint8_t buf_send[msg.length()+1];
        msg.getBytes(buf_send, msg.length()+1);

        esp_err_t result = esp_now_send(broadcastAddress, buf_send, msg.length());

        if (result == ESP_OK)
        {
            return true;
        }
        else if (result == ESP_ERR_ESPNOW_ARG)
        {
            Serial.println("ESP_ERR_ESPNOW_ARG");
            return false;
        }
        else if (result == ESP_ERR_ESPNOW_INTERNAL)
        {
            Serial.println("ESP_ERR_ESPNOW_INTERNAL");
            return false;
        }
        else if (result == ESP_ERR_ESPNOW_NO_MEM)
        {
            Serial.println("ESP_ERR_ESPNOW_NO_MEM");
            return false;
        }
        else if (result == ESP_ERR_ESPNOW_IF)
        {
            Serial.println("ESP_ERR_ESPNOW_IF");
            return false;
        }
        return false;
    }

    String read()
    {
        if (initialized == false)
        {
            Serial.println("ESP-NOW not initialized");
            return "";
        }

        String result = buf_recv;
        buf_recv = "";
        return result;
    }
}
