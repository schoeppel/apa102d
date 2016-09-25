#include "simple_step.h"

#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "apa102.h"
#include "color.h"

extern int running;

int simplestep_main(struct hsv_t color, int step_length_ms) {
	struct hsv_t black = { .h = 0.0, .s = 0.0, .v = 0.0 };
	struct timespec wait = { .tv_sec = 0, .tv_nsec = (step_length_ms - 1) * 1000000 };
	struct timespec fade_wait = { .tv_sec = 0, .tv_nsec = 100000 };

	struct apa102_led black_led = apa102_hsv_t(&black);
	struct apa102_led* l = apa102_open();

	int current_led = 0;

	if (l == NULL) return 1;

	for (int i = 0; i < NUM_LEDS; i++)
		l[i] = black_led;

	while(running) {
		for(int i = 1; i < 11; i = i + 1) {
			l[current_led] = hsv_fade(&black, &color, i / 10.0);
			l[(current_led + NUM_LEDS - 1) % NUM_LEDS] = hsv_fade(&color, &black, i/10.0);

			apa102_sync();
			nanosleep(&fade_wait, NULL);
		}
		current_led = (current_led + 1) % NUM_LEDS;

		if (step_length_ms > 1) {
			nanosleep(&wait, NULL);
		}
	}

	apa102_close();
	return 0;
}
