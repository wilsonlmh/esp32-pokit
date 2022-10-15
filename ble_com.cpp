#include "shared.h"
#include <Arduino.h>
#include "display.h"
#include "ble_data.h"
#include <BLEDevice.h>

static BLEUUID POKIT_MANUFACTURER_CHAR("00002a29-0000-1000-8000-00805f9b34fb");
static BLEUUID POKIT_MULTIMETER_BLE_SVC("e7481d2f-5781-442e-bb9a-fd4e3441dadc");
static BLEUUID POKIT_METER_STATUS_BLE_SVC("57d3a771-267c-4394-8872-78223e92aec4");

static BLEUUID POKIT_STATUS_BLE_SVC("57d3a771-267c-4394-8872-78223e92aec5");
static BLEUUID POKIT_STATUS_CHAR("3dba36e1-6120-4706-8dfd-ed9c16e569b6");

static BLEUUID POKIT_MM_BLE_SVC("e7481d2f-5781-442e-bb9a-fd4e3441dadc");
static BLEUUID POKIT_MM_SETTINGS_CHAR("53dc9a7a-bc19-4280-b76b-002d0e23b078");
static BLEUUID POKIT_MM_READ_CHAR("047d3559-8bee-423a-b229-4417fa603b90");

static BLEScan* ble_scan;
static BLEAdvertisedDevice* pokit_device;
static BLEClient* pokit_client;
static BLERemoteService* pokit_status_svc;
static BLERemoteCharacteristic* pokit_status_char;
static BLERemoteService* pokit_mm_svc;
static BLERemoteCharacteristic* pokit_mm_read_char;
static BLERemoteCharacteristic* pokit_mm_settings_char;

bool pokit_connected = false;
bool pokit_found = false;

pokit_measurement_value_t pokit_current_measurement = { 0 };
pokit_status_t pokit_current_status = { 0 };
pokit_settings_t pokit_current_settings = {
  .mode = mode_dc_voltage,
  .range = voltage_range_auto,
  .interval = 200,
};

String pokit_latest_display_value = "";

pokit_measurement_value_t parse_pokit(uint8_t* bytes) {
  pokit_measurement_value_t new_measurement;
  memcpy((void*)&new_measurement.value, (void*)bytes + 1, 4);

  new_measurement.mode = bytes[5];
  return new_measurement;
}

void _update_pokit_settings(void* pvParameters) {
  if (pokit_connected) {
    switch (pokit_current_status.mode_switch) {
      case 0:
        if (pokit_current_settings.mode != mode_dc_voltage && pokit_current_settings.mode != mode_ac_voltage) {
          pokit_current_settings.mode = mode_dc_voltage;
        }
        break;
      case 1:
        if (pokit_current_settings.mode < 3) {
          pokit_current_settings.mode = mode_resistant;
        }
        break;
      case 2:
        if (pokit_current_settings.mode != mode_dc_current && pokit_current_settings.mode != mode_dc_current) {
          pokit_current_settings.mode = mode_dc_current;
        }
        break;
    }
    // Serial.printf("Writing mm_settings, mode=%d, range=%d, update_interval=%d\n", pokit_current_settings.mode, pokit_current_settings.range, pokit_current_settings.interval);
    pokit_mm_settings_char->writeValue((uint8_t*)&pokit_current_settings, sizeof(pokit_current_settings), true);
    // Serial.println("Wrtie done");
  }
  vTaskDelete(NULL);
}

void update_pokit_settings() {
    xTaskCreate(_update_pokit_settings, "update_pokit_settings", 10000, NULL, 1, NULL);
}

class pokit_mm_scan_callback_c : public BLEAdvertisedDeviceCallbacks {
  /* Called for each advertising BLE server. */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("Found: ");
    Serial.println(advertisedDevice.toString().c_str());

    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(POKIT_STATUS_BLE_SVC)) {
      ble_scan->stop();
      if (pokit_device != nullptr) free(pokit_device);
      pokit_device = new BLEAdvertisedDevice(advertisedDevice);
      pokit_found = true;
    }
  }
};

class pokit_mm_client_callback_c : public BLEClientCallbacks {
  void onConnect(BLEClient* client_ptr) {
    // Serial.println("onConnect");
  }

  void onDisconnect(BLEClient* pclient) {
    pokit_connected = false;
    Serial.println("onDisconnect");
  }
};

static pokit_mm_client_callback_c pokit_mm_client_callback;
static pokit_mm_scan_callback_c pokit_mm_scan_callback;

void pokit_mm_notify_callback(BLERemoteCharacteristic* ble_char, uint8_t* data, size_t len, bool is_notify) {
  pokit_current_measurement = parse_pokit((uint8_t*)data);
  pokit_latest_display_value = get_display_chars(pokit_current_measurement);
  Serial.println(pokit_latest_display_value);
}

void pokit_status_notify_callback(BLERemoteCharacteristic* ble_char, uint8_t* data, size_t len, bool is_notify) {
  if (len == 8) {  //For pokit pro, len should be 8
    memcpy((void*)&pokit_current_status, (void*)data, sizeof(pokit_current_status));
    update_pokit_settings();
    Serial.printf("pokit_current_status = {\n  status: %d,\n  battery: %f,\n  unknown_1: %d,\n  mode_switch: %d,\n  unknown_2: %d\n}\n", pokit_current_status.status, pokit_current_status.battery, pokit_current_status.unknown_1, pokit_current_status.mode_switch, pokit_current_status.unknown_2);
    // delay(100);
  } else {
    // Serial.printf("pokit_status_notify_callback() len is %d\n", len);
  }
}

void connect_pokit(void* parameter) {
  while (1) {
    if (!pokit_connected && pokit_found) {
      pokit_found = false;
      // Serial.println("connect_pokit()");
      pokit_client = BLEDevice::createClient();
      pokit_client->setClientCallbacks(&pokit_mm_client_callback);
      pokit_client->connect(pokit_device);
      Serial.println("Connected");
      // Serial.println(pokit_client->toString().c_str());

      //connect to POKIT_STATUS_BLE_SVC, accquire POKIT_STATUS_CHAR, then subscribe to POKIT_STATUS_CHAR
      pokit_status_svc = pokit_client->getService(POKIT_STATUS_BLE_SVC);
      if (pokit_status_svc == nullptr) {
        Serial.println("No POKIT_STATUS_BLE_SVC!");
        pokit_client->disconnect();
        continue;
      }
      Serial.println("Got POKIT_STATUS_BLE_SVC");

      pokit_status_char = pokit_status_svc->getCharacteristic(POKIT_STATUS_CHAR);
      if (pokit_status_char == nullptr) {
        Serial.println("No POKIT_STATUS_CHAR!");
        pokit_client->disconnect();
        continue;
      }
      Serial.println("Got POKIT_STATUS_CHAR");

      if (pokit_status_char->canNotify()) {
        pokit_status_char->registerForNotify(pokit_status_notify_callback);
      } else {
        pokit_client->disconnect();
        Serial.println("Can't subscribe to POKIT_STATUS_CHAR!");
        continue;
      }
      Serial.println("Subscribed to POKIT_STATUS_CHAR!");

      //connect to POKIT_MM_BLE_SVC, accquire POKIT_MM_READ_CHAR & POKIT_MM_SETTINGS_CHAR, then subscribe to POKIT_MM_READ_CHAR
      pokit_mm_svc = pokit_client->getService(POKIT_MM_BLE_SVC);
      if (pokit_mm_svc == nullptr) {
        Serial.println("No POKIT_MM_BLE_SVC!");
        pokit_client->disconnect();
        continue;
      }
      Serial.println("Got POKIT_MM_BLE_SVC");

      pokit_mm_read_char = pokit_mm_svc->getCharacteristic(POKIT_MM_READ_CHAR);
      if (pokit_mm_read_char == nullptr) {
        Serial.println("No POKIT_MM_READ_CHAR!");
        pokit_client->disconnect();
        continue;
      }
      Serial.println("Got POKIT_MM_READ_CHAR");

      pokit_mm_settings_char = pokit_mm_svc->getCharacteristic(POKIT_MM_SETTINGS_CHAR);
      if (pokit_mm_settings_char == nullptr) {
        Serial.println("No POKIT_MM_SETTINGS_CHAR!");
        pokit_client->disconnect();
        continue;
      }
      Serial.println("Got POKIT_MM_SETTINGS_CHAR");

      if (pokit_mm_read_char->canNotify()) {
        pokit_mm_read_char->registerForNotify(pokit_mm_notify_callback);
      } else {
        pokit_client->disconnect();
        Serial.println("Can't subscribe to POKIT_MM_READ_CHAR!");
        continue;
      }
      Serial.println("Subscribed to POKIT_MM_READ_CHAR!");

      pokit_connected = true;
      if (pokit_status_char->canRead()) {
        std::string value = pokit_status_char->readValue();
        uint8_t* data = (uint8_t*)reinterpret_cast<const uint8_t*>(&value[0]);
        pokit_status_notify_callback(pokit_status_char, data, value.length(), false);
      }
      // update_pokit_settings();
    }
    delay(100);
  }
}

void search_pokit(void* parameter) {
  while (1) {
    if (!pokit_connected && !pokit_found) {
      ble_scan->setAdvertisedDeviceCallbacks(&pokit_mm_scan_callback);
      ble_scan->setInterval(1349);
      ble_scan->setWindow(449);
      ble_scan->setActiveScan(true);
      ble_scan->start(5, false);
    }
    delay(5000);
  }
}

void _init_pokit(void* pvParameters)  {
  BLEDevice::init("pokit-client");
  ble_scan = BLEDevice::getScan();

  xTaskCreate(search_pokit, "search_pokit", 10000, NULL, 10, NULL);
  xTaskCreate(connect_pokit, "connect_pokit", 10000, NULL, 5, NULL);
  vTaskDelete(NULL);
}

void init_pokit() {
  xTaskCreate(_init_pokit, "init_pokit", 20000, NULL, 1, NULL); //use async task to avoid stack not deep enough    
}

