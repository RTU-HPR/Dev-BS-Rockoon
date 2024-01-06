#pragma once
// Main libraries
#include <Arduino.h>
#include <SPI.h>

#include <gps.h>
#include <steppers.h>

// Our wrappers
#include <RadioLib_wrapper.h>

// Used radio module
#define radio_module SX1268

class Config
{
public:
  bool WAIT_PC = false;

  // 433 MHz LoRa
  RadioLib_Wrapper<radio_module>::Radio_Config radio_config{
      .frequency = 434.5,
      .cs = 2,
      .dio0 = 3,
      .dio1 = 5,
      .family = RadioLib_Wrapper<radio_module>::Radio_Config::Chip_Family::Sx126x,
      .rf_switching = RadioLib_Wrapper<radio_module>::Radio_Config::Rf_Switching::Dio2,
      // .rx_enable = 0, // only needed if rf_switching = gpio
      // .tx_enable = 0, // only needed if rf_switching = gpio
      .reset = 8,
      .sync_word = 0xF4,
      .tx_power = 10,
      .spreading = 11,
      .coding_rate = 8,
      .signal_bw = 62.5,
      .frequency_correction = false,
      .spi_bus = &SPI,
  };

  // GPS
  Gps::Gps_Config gps_config{
      .serial = &Serial1,
  };

  // Stepper motors
  const int STEPPER_STEPS_PER_REVOLUTION = 1600;
  const int STEPPER_GEAR_RATIO = 30;
  const int STEPPER_MIN_PULSE_WIDTH = 50;
  const int STEPPER_MAX_SPEED = 800;
  const int STEPPER_MAX_ACCELERATION = 600;

  Steppers::Stepper_Config azimuth_config{
      .step_pin = 48,
      .dir_pin = 47,
      .enable_pin = 42,
      .steps_per_revolution = STEPPER_STEPS_PER_REVOLUTION,
      .gear_ratio = STEPPER_GEAR_RATIO,
      .min_pulse_width = STEPPER_MIN_PULSE_WIDTH,
      .max_speed = STEPPER_MAX_SPEED,
      .max_acceleration = STEPPER_MAX_ACCELERATION,
  };

  Steppers::Stepper_Config elevation_config{
      .step_pin = 45,
      .dir_pin = 39,
      .enable_pin = 33,
      .steps_per_revolution = STEPPER_STEPS_PER_REVOLUTION,
      .gear_ratio = STEPPER_GEAR_RATIO,
      .min_pulse_width = STEPPER_MIN_PULSE_WIDTH,
      .max_speed = STEPPER_MAX_SPEED,
      .max_acceleration = STEPPER_MAX_ACCELERATION,
  };

  // SPI0
  const int SPI0_RX = 4;
  const int SPI0_TX = 7;
  const int SPI0_SCK = 6;

  // PC Serial
  const int PC_BAUDRATE = 115200;

  // Serial 1
  const int SERIAL1_BAUDRATE = 9600;
  const int SERIAL1_RX = 6;
  const int SERIAL1_TX = 7;
};