#include "simple_step.h"

#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "apa102.h"
#include "color.h"

void* step_step(void* last_state,
                char** message,
                unsigned long long timestamp,
                struct apa102_led* leds,
                int nr_leds,
                int leds_per_meter) {
	struct step_effect_state* current_state = (struct step_effect_state*) last_state;

	if (!last_state) {
		struct hsv_t color = {
			.h = ,
			.s = ,
			.v =
		};
		current_state = malloc(sizeof(*current_state));

		current_state->target_led = 0;
		current_state->fade = 0.0;
		current_state->color = color;
		current_state->black = 	{ .h = color.h, .s = 0.0, .v = 0.0 };
		current_state->next_step = timestamp;
	}

	if (timestamp < current_state->next_step) { return; }

	float fade = current_state->fade;
	int target_led = current_state->target_led;
	struct hsv_t black = current_state->black;

	int prev_led = (target_led + nr_leds - 1) % nr_leds;
	if (fade >= 1.0) {
		// Switch to the next LED
		int next_led = (target_led + 1) % nr_leds;

		leds[target_led] = apa102_hsv_t(&color);
		leds[prev_led] = apa102_hsv_t(&black);

		current_state->target_led = next_led;
		current_state->fade = 0.0;
	} else {
		// Fade the current led and the previous LED
		leds[target_led] = hsv_fade(&black, &color, fade);
		leds[prev_led] = hsv_fade(&color, &black, fade);

		current_state->fade = fade + 5.0 / ((float) step_length_ms);
	}

	current_state->next_step = timestamp + 5;

	return (void*) current_state;
}

void step_destroy(void* last_state) {
	free(last_state);
}
