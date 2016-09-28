#ifndef STEP_H
#define STEP_H

#include "apa102.h"

void* step_step(void* last_state,
                const char** message,
                unsigned long long timestamp,
                struct apa102_led* leds,
                int nr_leds,
                int leds_per_meter);

void step_destroy(void* last_state);

#endif
