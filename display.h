#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <Arduino.h>

String get_display_chars(pokit_measurement_value_t v);
void display_init();
void draw();

#endif // _DISPLAY_H_