#include <config.h>

class LCD
{
private:
  LCD_I2C *_lcd;

  unsigned int _update_interval = 0;
  unsigned long _last_update = 0;

  struct Lcd_Data
  {
    float latitude;
    float longitude;
    float altitude;
    float rssi;
    float snr;
    unsigned int last_radio_packet_millis;
    bool connected_to_wifi;
  };
  Lcd_Data _lcd_data = {
      .latitude = 0,
      .longitude = 0,
      .altitude = 0,
      .rssi = 0,
      .snr = 0,
      .last_radio_packet_millis = 0,
      .connected_to_wifi = false,
  };

public:
  /**
   * @brief Construct a new LCD object
  */
  bool begin(int lcd_address, int lcd_columns, int lcd_rows, int update_interval);

  /**
   * @brief Update the LCD data. Call this after each packet is received
   * 
   * @param latitude 
   * @param longitude 
   * @param altitude 
   * @param rssi 
   * @param snr
  */
  void updateLcdData(float latitude, float longitude, float altitude, float rssi, float snr);

  /**
   * @brief Update the LCD screen
  */
  void updateLcdScreen(unsigned int last_radio_packet_millis, bool connected_to_wifi);
};
