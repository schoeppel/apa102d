#include "simple_step.h"

#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "apa102.h"
#include "color.h"

extern int running;

int simplestep_main(struct hsv_t color, int step_length_ms) {
	struct hsv_t black = { .h = color.h, .s = 0.0, .v = 0.0 };
	struct timespec wait = { .tv_sec = 0, .tv_nsec =  5 * 1000 * 1000 };

	struct apa102_led* all_leds = apa102_open();

	int current_led = 0;
	float current_fade = 0.0;

	if (all_leds == NULL) return 1;

	hsv_fill(&black);

	while(running) {
		if (current_fade >= 1.0) {
			all_leds[current_led] = apa102_hsv_t(&color);
			all_leds[(current_led + NUM_LEDS - 1) % NUM_LEDS] = apa102_hsv_t(&black);

			current_led = (current_led + 1) % NUM_LEDS;
			current_fade = 0.0;
		} else {
			all_leds[current_led] = hsv_fade(&black, &color, current_fade);
			all_leds[(current_led + NUM_LEDS - 1) % NUM_LEDS] = hsv_fade(&color, &black, current_fade);

			current_fade = current_fade + 5.0 / ((float) step_length_ms);
		}
		apa102_sync();

		nanosleep(&wait, NULL);
	}

	apa102_close();
	return 0;
}
