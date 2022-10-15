#include <M5StickCPlus.h>
#include "shared.h"
#include "ble_data.h"
#include "display.h"
#include "ble_com.h"

// #include <WiFi.h>
#include "esp_pm.h"
#include "driver/adc.h"

uint8_t current_brightness = 7;

void setup() {

  Serial.begin(115200);
  Serial.setTimeout(10);

  // //Config DFS
  // esp_pm_config_esp32_t pm_config;
  // pm_config.max_freq_mhz = 80;
  // pm_config.min_freq_mhz = 10;  //RTC_CPU_FREQ_XTAL,;
  // pm_config.light_sleep_enable = false;
  // ESP_ERROR_CHECK(esp_pm_configure(&pm_config));
  adc_power_off();
  // // WiFi.mode(WIFI_OFF);

  M5.begin();
  M5.Axp.ScreenBreath(current_brightness);

  display_init();
  init_pokit();
}

void loop() {
  M5.update();
  if (M5.BtnA.wasPressed()) {
    pokit_current_settings.mode += 1;
    pokit_current_settings.mode = pokit_current_settings.mode >= 9 ? 1 : pokit_current_settings.mode;
    update_pokit_settings();
  }

  if (M5.BtnB.wasPressed()) {
    current_brightness++;
    current_brightness >= 15 && (current_brightness = 7);
    M5.Axp.ScreenBreath(current_brightness);
  }

  draw();

  if (Serial.available()) {
    Serial.readString();
    ESP.restart();
  }
  delay(100);
}