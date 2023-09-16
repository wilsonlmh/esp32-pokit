#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <Arduino.h>

char* construct_display_chars(pokit_measurement_value_t v, char* r);
void display_init();
void draw();

#endif // _DISPLAY_H_