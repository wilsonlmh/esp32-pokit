#ifndef _SHARED_H_
#define _SHARED_H_

#include "ble_data.h"
#include <Arduino.h>

extern bool pokit_connected;
extern bool pokit_found;

extern pokit_measurement_value_t pokit_current_measurement;
extern pokit_status_t pokit_current_status;
extern pokit_settings_t pokit_current_settings;

extern String pokit_latest_display_value;

#endif // _SHARED_H_