#include <lcd.h>

bool LCD::begin(int lcd_address, int lcd_columns, int lcd_rows, int update_interval)
{
  _update_interval = update_interval;
  _lcd = new LCD_I2C(lcd_address, lcd_columns, lcd_rows);
  _lcd->begin(false); // Don't call the wire begin() function
  _lcd->display();
  _lcd->backlight();
  return true;
}

void LCD::updateLcdData(float latitude, float longitude, float altitude, float rssi, float snr)
{
  _lcd_data.latitude = latitude;
  _lcd_data.longitude = longitude;
  _lcd_data.altitude = altitude;
  _lcd_data.rssi = rssi;
  _lcd_data.snr = snr;
}

void LCD::updateLcdScreen(unsigned int last_radio_packet_millis, bool connected_to_wifi)
{
  /*
  LCD screen layout (16x4):
  Lat  |00.00000    -000| Rssi
  Lng  |00.00000     -00| Snr
  Alt  |00000.00    0000| Time since last packet
  Dist |000.00     False| Connected to WiFi

  * Left side numbers are left aligned
  * Right side numbers are right aligned

  * Latitude, longitude has 5 decimal places which gives a precision of 1.11 meters
  * Altitude has 2 decimal places (Meters)
  * Distance (Distance from rotator to target) has 2 decimal places (Kilometers)
  * Rssi is a negative integer (-120 - 0)
  * Snr is a positive/negative integer
  * Time since last packet is in seconds (0 - 9999) - 9999/3600 = 2.77 hours
  * Time to next cycle is in seconds (0 - 12)
  */

  _lcd_data.last_radio_packet_millis = last_radio_packet_millis;

  if (millis() - _last_update < _update_interval)
  {
    return;
  }

  _lcd->clear();

  // First row
  _lcd->setCursor(0, 0);
  _lcd->print(_lcd_data.latitude, 5);

  String rssi_str = String((int)_lcd_data.rssi);
  int rssi_len = rssi_str.length();
  _lcd->setCursor(15 - rssi_len, 0);
  _lcd->print(rssi_str);

  // Second row
  _lcd->setCursor(0, 1);
  _lcd->print(_lcd_data.longitude, 5);

  // Right align the snr
  String snr_str = String((int)_lcd_data.snr);
  int snr_len = snr_str.length();
  _lcd->setCursor(15 - snr_len, 1);
  _lcd->print(snr_str);

  // Third row
  _lcd->setCursor(0, 2);
  _lcd->print(_lcd_data.altitude, 2);

  String time_since_last_packet_str = String((millis() - _lcd_data.last_radio_packet_millis) / 1000);
  int time_since_last_packet_len = time_since_last_packet_str.length();
  _lcd->setCursor(15 - time_since_last_packet_len, 2);
  _lcd->print(time_since_last_packet_str);

  // Fourth row
  if (_lcd_data.connected_to_wifi)
  {
    _lcd->setCursor(11, 3);
    _lcd->print("True");
  }
  else
  {
    _lcd->setCursor(10, 3);
    _lcd->print("False");
  }

  _last_update = millis();
}