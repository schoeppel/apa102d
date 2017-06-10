#ifndef COLOR_H
#define COLOR_H

#include "apa102.h"

void* color_step(void* last_state,
                const char** message,
                unsigned long long timestamp,
                struct apa102_led* leds,
                int nr_leds,
                int leds_per_meter);

void color_destroy(void* last_state);

#endif
