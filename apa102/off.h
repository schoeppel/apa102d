#ifndef OFF_H
#define OFF_H

#include "apa102.h"

void* off_step(void* last_state,
                const char** message,
                unsigned long long timestamp,
                struct apa102_led* leds,
                int nr_leds,
                int leds_per_meter);

void off_destroy(void* last_state);

#endif
