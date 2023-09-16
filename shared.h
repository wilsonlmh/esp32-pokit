#ifndef _SHARED_H_
#define _SHARED_H_

#include "ble_data.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <Arduino.h>

extern bool pokit_connected;
extern bool pokit_found;

extern pokit_measurement_value_t pokit_current_measurement;
extern pokit_status_t pokit_current_status;
extern pokit_settings_t pokit_current_settings;

extern char pokit_latest_display_value[20];

extern xSemaphoreHandle pokit_connection_mutex;
extern xSemaphoreHandle pokit_status_mutex;
extern xSemaphoreHandle pokit_settings_mutex;
extern xSemaphoreHandle pokit_latest_display_value_mutex;

#define mutex_lock(lock)  do {} while (xSemaphoreTake(lock, 0xFFFF) != pdPASS)
#define mutex_unlock(lock)  xSemaphoreGive(lock)

#endif // _SHARED_H_