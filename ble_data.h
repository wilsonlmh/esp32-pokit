#ifndef _BLE_DATA_H_
#define _BLE_DATA_H_

#include <stdint.h>

// Ref: https://pcolby.github.io/qtpokit/main/int/pokit.html
// Ref: https://www.pokitinnovations.com/wp-content/uploads/D0005250-PokitMeter-Bluetooth-API-Documentation-0_02.pdf

typedef struct {
  uint8_t mode;  // Use this to determine display unit
  float value;
  uint16_t battery;  // not yet know what it is
}  __attribute__ ((__packed__)) pokit_measurement_value_t;

typedef struct {
  uint8_t mode;
  uint8_t range;
  uint32_t interval;  // unit: ms
} __attribute__ ((__packed__)) pokit_settings_t;

typedef struct {
  /*
      0 – IDLE
      1 – MM meas. DC Voltage
      2 – MM meas. AC Voltage
      3 – MM meas. DC Current
      4 – MM meas. AC Current
      5 – MM meas. Resistance
      6 – MM meas. Diode
      7 – MM meas. Continuity
      8 – MM meas. Temperature
      9 – DSO Mode (Sampling)
      10 – Logger Mode (Sampling)
  */
  uint8_t status;
  float battery;
  uint8_t unknown_1;
  //For pokit pro, there's a physical switch on the probe. 0 means V(leftmost), 1 means mA/Ohm/diode etc.(middle), 2 means A(right)
  uint8_t mode_switch;
  uint8_t unknown_2;
} __attribute__ ((__packed__)) pokit_status_t;

#define mode_dc_voltage 1  // Use range voltage_range
#define mode_ac_voltage 2  // Use range voltage_range
#define mode_dc_current 3  // Use range current_range
#define mode_ac_current 4  // Use range current_range
#define mode_resistant 5   // Use range resistant_range
#define mode_diode 6       // Use 255
#define mode_connectivity 7
#define mode_temperature 8

#define voltage_range_0mV_to_300mV 1
#define voltage_range_300mV_to_2V 1
#define voltage_range_2V_to_6V 2
#define voltage_range_6V_to_12V 3
#define voltage_range_12V_to_30V 4
#define voltage_range_30V_to_60V 5
#define voltage_range_auto 255

#define current_range_0A_to_10mA 0
#define current_range_10mA_to_30mA 1
#define current_range_30mA_to_150mA 2
#define current_range_150mA_to_300mA 3
#define current_range_300mA_to_3A 4
#define current_range_range_auto 255

#define resistant_range_0Ohm_to_160Ohm 0
#define resistant_range_160Ohm_to_330Ohm 1
#define resistant_range_330Ohm_to_890Ohm 2
#define resistant_range_890Ohm_to_1500kOhm 3
#define resistant_range_1500Ohm_to_10kOhm 4
#define resistant_range_10kOhm_to_100kOhm 5
#define resistant_range_100kOhm_to_470kOhm 6
#define resistant_range_470kOhm_to_1MOhm 7
#define resistant_range_auto 255

#define diodie_range 255
#define connectivity_range 255
#define temperature_range 255;

pokit_status_t get_pokit_current_status();
pokit_settings_t get_pokit_current_settings();
bool get_pokit_connected();

#endif