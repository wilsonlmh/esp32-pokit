#include "shared.h"
#include "ble_data.h"

pokit_status_t get_pokit_current_status() {
  mutex_lock(pokit_status_mutex);
  pokit_status_t tmp = pokit_current_status;
  mutex_unlock(pokit_status_mutex);
  return tmp;
}

pokit_settings_t get_pokit_current_settings() {
  mutex_lock(pokit_settings_mutex);
  pokit_settings_t tmp = pokit_current_settings;
  mutex_unlock(pokit_settings_mutex);
  return tmp;
}

bool get_pokit_connected() {
  mutex_lock(pokit_connection_mutex);
  bool tmp = pokit_connected;
  mutex_unlock(pokit_connection_mutex);
  return tmp;
}