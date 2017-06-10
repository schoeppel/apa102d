#include "test.h"

#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "apa102.h"

#include "config.h"

struct test_effect_state {
	unsigned int target_led;
	float fade;
	struct hsv_t color;
	struct hsv_t black;
	int test_length_ms;
	unsigned long long next_step;
};


void* test_step(void* last_state,
                const char** message,
                unsigned long long timestamp,
                struct apa102_led* leds,
                int nr_leds,
                int leds_per_meter) {
	struct test_effect_state* current_state = (struct test_effect_state*) last_state;

	if (!last_state) {
		const char* color_code = get_message_value(message, "color", "hsv(0.0,0.3,0.15)");
		struct hsv_t color = parse_hsv_color(color_code);
		current_state = malloc(sizeof(*current_state));

		current_state->target_led = 0;
		current_state->fade = 0.0;
		current_state->color = color;
		current_state->black.h = color.h;
		current_state->black.s = 0.0;
		current_state->black.v = 0.0;
		current_state->test_length_ms = atoi(get_message_value(message, "test_length_ms", "10"));
		current_state->next_step = timestamp;
	}


	unsigned int test_pattern = atoi(get_message_value(message, "pattern", "0"));

	if (test_pattern == 0) {
		if (timestamp < current_state->next_step) {
			return (void*) current_state;
		}

		float fade = current_state->fade;
		int target_led = current_state->target_led;
		struct hsv_t black = current_state->black;
		struct hsv_t color = current_state->color;

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

			current_state->fade = fade + 5.0 / ((float) current_state->test_length_ms);
		}

		current_state->next_step = timestamp + 5;

	} else if (test_pattern == 1) {
		for (unsigned i = 0; i < devconfig->num_leds; i++) {
			leds[i] = apa102_rgb(1.0, 1.0, 1.0);
		}

	} else if (test_pattern == 2) {
		for (unsigned i = 0; i < devconfig->num_leds; i++) {
			if (i % 2 == 0) {
				leds[i] = apa102_rgb(1.0, 1.0, 1.0);
			} else {
				leds[i] = apa102_rgb(0.0, 0.0, 0.0);
			}
		}

	} else if (test_pattern == 3) {
		for (unsigned i = 0; i < devconfig->num_leds; i++) {
			if (i < devconfig->max_current_segment_size) {
				leds[i] = apa102_rgb(1.0, 1.0, 1.0);
			} else {
				leds[i] = apa102_rgb(0.0, 0.0, 0.0);
			}
		}
	} else if (test_pattern == 4) {
		for (unsigned i = 0; i < devconfig->num_leds; i++) {
			leds[i] = apa102_rgb(1.0, 0.0, 0.0);
		}
	} else if (test_pattern == 5) {
		for (unsigned i = 0; i < devconfig->num_leds; i++) {
			leds[i] = apa102_rgb(0.0, 1.0, 0.0);
		}
	} else if (test_pattern == 6) {
		for (unsigned i = 0; i < devconfig->num_leds; i++) {
			leds[i] = apa102_rgb(0.0, 0.0, 1.0);
		}
	} else if (test_pattern == 7) {
		if ((time(NULL) / 10) % 2 == 0) {
			for (unsigned i = 0; i < devconfig->num_leds; i++) {
				leds[i] = apa102_rgb(0.0, 0.0, 0.0);
			}
		} else {
			for (unsigned i = 0; i < devconfig->num_leds; i++) {
				leds[i] = apa102_hsv((float)i/devconfig->num_leds, 0.5, 1.0);
			}
		}
	}


	return (void*) current_state;
}

void test_destroy(void* last_state) {
	free(last_state);
}
