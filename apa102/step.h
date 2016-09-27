#ifndef STEP_H
#define STEP_H

#include "apa102.h"

struct step_effect_state {
  unsigned int target_led;
  float fade;
  struct hsv_t color;
  struct hsv_t black;
  int step_length_ms;
  unsigned long long next_step;
};

void* step_step(void* last_state,
                const char** message,
                unsigned long long timestamp,
                struct apa102_led* leds,
                int nr_leds,
                int leds_per_meter);

void step_destroy(void* last_state);

#endif
