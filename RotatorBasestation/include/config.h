#pragma once

// Main libraries
#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <TinyGPS++.h>
#include <AccelStepper.h>

// Our wrappers
#include <RadioLib_wrapper.h>

// Used radio module
#define radio_module SX1262
#define radio_module_family Sx126x
#define radio_module_rf_switching Disabled

class Config
{
public:
  // CONFIG STRUCT DECLARATIONS
  struct Stepper_Config
  {
    int step_pin;
    int dir_pin;
    int enable_pin;
    int steps_per_revolution;
    int gear_ratio;
    int min_pulse_width;
    int max_speed;
    int max_acceleration;
  };

  struct WiFi_Config
  {
    char ssid[32];
    char pass[32];
    unsigned int tmPort;
    unsigned int tcPort;
  };

  struct Gps_Config {
    HardwareSerial *serial;
  };

  // CONFIG DEFINITIONS
  // RADIO
  RadioLib_Wrapper<radio_module>::Radio_Config radio_config{
      .frequency = 434.5, // Frequency
      .cs = 8,            // Chip select
      .dio0 = 13,         // Busy
      .dio1 = 14,         // Interrupt action
      .family = RadioLib_Wrapper<radio_module>::Radio_Config::Chip_Family::radio_module_family,
      .rf_switching = RadioLib_Wrapper<radio_module>::Radio_Config::Rf_Switching::radio_module_rf_switching,
      // If using GPIO pins for RX/TX switching, define pins used for RX and TX control
      .rx_enable = -1,
      .tx_enable = -1,
      .reset = 12,
      .sync_word = 0xF4,
      .tx_power = 10,
      .spreading = 11,
      .coding_rate = 8,
      .signal_bw = 62.5,
      .frequency_correction = true,
      .spi_bus = &SPI // SPI bus used by radio
  };
  // Create radio object and pass error function if not passed will use serial print
  RadioLib_Wrapper<radio_module> radio = RadioLib_Wrapper<radio_module>(nullptr, 5);

  // GPS
  Gps_Config gps_config{
      .serial = &Serial1,
  };

  // STEPPERS
  const int STEPPER_STEPS_PER_REVOLUTION = 1600;
  const int STEPPER_GEAR_RATIO = 30;
  const int STEPPER_MIN_PULSE_WIDTH = 50;
  const int STEPPER_MAX_SPEED = 800;
  const int STEPPER_MAX_ACCELERATION = 600;

  Stepper_Config azimuth_config{
      .step_pin = 48,
      .dir_pin = 47,
      .enable_pin = 42,
      .steps_per_revolution = STEPPER_STEPS_PER_REVOLUTION,
      .gear_ratio = STEPPER_GEAR_RATIO,
      .min_pulse_width = STEPPER_MIN_PULSE_WIDTH,
      .max_speed = STEPPER_MAX_SPEED,
      .max_acceleration = STEPPER_MAX_ACCELERATION,
  };

  Stepper_Config elevation_config{
      .step_pin = 45,
      .dir_pin = 39,
      .enable_pin = 33,
      .steps_per_revolution = STEPPER_STEPS_PER_REVOLUTION,
      .gear_ratio = STEPPER_GEAR_RATIO,
      .min_pulse_width = STEPPER_MIN_PULSE_WIDTH,
      .max_speed = STEPPER_MAX_SPEED,
      .max_acceleration = STEPPER_MAX_ACCELERATION,
  };

  // WIFI
  /**
   * @brief WiFi config
   * @param ssid The SSID (Name) of the network to connect to
   * @param pass The password of the network to connect to
   * @param tmPort The port to send telemetry messages to
   * @param tcPort The port to receive telecommand messages from
   * @note IMPORTANT: When changing the network configuration, comment out the old configuration
   * and add a comment about the new network configuration being changed to.
   */
  WiFi_Config wifi_config{
      "Samsung S20", // Gundars phone hotspot name
      "123456789",   // Gundars phone hotspot password
      10035,         // This ports should not change
      10045          // This ports should not change
  };

  const int GPS_SEND_INTERVAL = 10000;
  const int ROTATOR_TELEMETRY_APID = 50;
  const String ROTATOR_TELEMETRY_MESSAGE_HEADER_APID = "rtu_rotator," + String(ROTATOR_TELEMETRY_APID);

  // SPI0
  const int SPI0_RX = 11;
  const int SPI0_TX = 10;
  const int SPI0_SCK = 9;

  // PC Serial
  const int PC_BAUDRATE = 115200;

  // Serial 1
  const int SERIAL1_BAUDRATE = 9600;
  const int SERIAL1_RX = 6;
  const int SERIAL1_TX = 7;
};