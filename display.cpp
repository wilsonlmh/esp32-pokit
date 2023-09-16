#include "shared.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <Arduino.h>
#include <M5StickCPlus.h>
#include "ble_data.h"

#define OHM char(233) // Ω
#define DEG char(247) // °

#define MAX_DECIMAL_DIGITS 4  //uV, uA, uOhm level
#define PADDING_LEFT 2
#define PADDING_RIGHT 2
#define PADDING_TOP 2
#define PADDING_BOTTOM 2

static TFT_eSprite fbuf(&M5.Lcd);  //frame buffer

char* construct_display_chars(pokit_measurement_value_t v, char* r) {
  char unit[3] = { 0 }, new_unit[3] = { 0 };
  float factor = 1.0f;
  uint8_t decimals = MAX_DECIMAL_DIGITS;

  if (v.mode <= 5) {  // inc. ac/dc voltage, current, resistant
    if (fabs(v.value) > 10000000.0f) {
      factor = 0.001f;
      unit[0] = 'M';
    } else {
      if (fabs(v.value) > 1000.0f) {
        factor = 0.001f;
        unit[0] = 'k';
      }
    }
    if (fabs(v.value) < 0.001f) {
      factor = 1000000.0f;
      unit[0] = 'u';
      decimals -= 6;
    } else {
      if (fabs(v.value) < 1.0f) {
        factor = 1000.0f;
        unit[0] = 'm';
        decimals -= 3;
      }
    }
  }
  float value = v.value * factor;

  switch (v.mode) {
    case mode_dc_voltage:
    case mode_ac_voltage:
      sprintf(new_unit, "%sV", unit);
      strcpy(unit, new_unit);
      break;
    case mode_dc_current:
    case mode_ac_current:
      sprintf(new_unit, "%sA", unit);
      strcpy(unit, new_unit);
      break;
    case mode_resistant:
      sprintf(new_unit, "%s%c", unit, OHM);
      strcpy(unit, new_unit);
      break;
    case mode_diode:
      sprintf(unit, "V");
      break;
    case mode_connectivity:
      sprintf(unit, "%c", OHM); 
      break;
    case mode_temperature:
      sprintf(unit, "%cC", DEG);
      break;
  }

  decimals = decimals > MAX_DECIMAL_DIGITS ? 0 : decimals; //overflow
  sprintf(r, "%.*f %s", decimals, value, unit);
  return r;
}

void display_init() {
  M5.Lcd.setRotation(1);
  fbuf.setRotation(1);
  fbuf.setSwapBytes(true);
  fbuf.createSprite(M5.Lcd.width(), M5.Lcd.height());
}

void draw_battery() {
  fbuf.setTextSize(2);
  fbuf.setTextDatum(TL_DATUM);
  fbuf.setTextColor(((255 >> 3) << 11) | ((102 >> 2) << 5) | ((0 >> 3) << 0), TFT_WHITE);  // #FF6600
  String m5_voltage = String(M5.Axp.GetBatVoltage(), 2);
  m5_voltage.concat("V");
  fbuf.drawString(m5_voltage, PADDING_LEFT + 0, PADDING_TOP + 0, 1);
  String m5_current = String(M5.Axp.GetBatCurrent(), 2);
  m5_current.concat("mA");
  fbuf.drawString(m5_current, PADDING_LEFT + 5 * 12 + 4, PADDING_TOP + 0, 1);

  fbuf.setTextDatum(BR_DATUM);
  fbuf.setTextColor(((0 >> 3) << 11) | ((0 >> 2) << 5) | ((255 >> 3) << 0), TFT_WHITE);  // #FF6600
  String pokit_battery = String(get_pokit_current_status().battery);
  pokit_battery.concat("V");
  fbuf.drawString(pokit_battery, (int)(M5.Lcd.width() - PADDING_RIGHT), PADDING_TOP + 0, 1);
}

void draw_mode() {
  fbuf.setTextSize(2);
  fbuf.setTextColor(TFT_BLACK, TFT_WHITE);
  fbuf.setTextDatum(BL_DATUM);
  fbuf.drawString("-   -  ", PADDING_LEFT + 0, (int)(M5.Lcd.height() - 12 - PADDING_BOTTOM), 1);
  fbuf.drawString("~", PADDING_LEFT + 24, (int)(M5.Lcd.height() - 8 - PADDING_BOTTOM), 1);
  fbuf.drawString("~", PADDING_LEFT + 72, (int)(M5.Lcd.height() - 8 - PADDING_BOTTOM), 1);
  fbuf.drawString("V V A A R D X T", PADDING_LEFT + 0, (int)(M5.Lcd.height() - 2 - PADDING_BOTTOM), 1);

  //Selection
  fbuf.fillRect(PADDING_LEFT + (pokit_current_settings.mode - 1) * 24, (int)(M5.Lcd.height() - 4 - PADDING_BOTTOM), 10, 4, RED);
}

void draw_range() {
  //TODO
}

void draw_value() {
  String next_value;
  mutex_lock(pokit_latest_display_value_mutex);
  next_value = String(pokit_latest_display_value);
  mutex_unlock(pokit_latest_display_value_mutex);

  mutex_lock(pokit_connection_mutex);
  if (!pokit_connected) next_value = pokit_found ? "Connecting..." : "Searching...";
  mutex_unlock(pokit_connection_mutex);

  fbuf.setTextSize(3);
  fbuf.setTextColor(TFT_BLACK, TFT_WHITE);
  fbuf.setTextDatum(MC_DATUM);
  fbuf.drawString(next_value, (int)(M5.Lcd.width() / 2), (int)(M5.Lcd.height() / 2), 1);
}

void draw() {
  fbuf.fillSprite(TFT_WHITE);

  draw_value();
  draw_battery();
  draw_mode();
  draw_range();

  fbuf.pushSprite(0, 0);
};