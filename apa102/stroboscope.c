#include "stroboscope.h"

#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "apa102.h"
#include "color.h"

extern int running;

int stroboscope_main(struct hsv_t color, float percentage) {
	struct hsv_t black = { .h = 0.0, .s = 0.0, .v = 0.0 };
	struct timespec wait_black = { .tv_sec = 0, .tv_nsec =  20 * 1000 * 1000 };
	struct timespec wait_white = { .tv_sec = 0, .tv_nsec =  10 * 1000 * 1000 };

	struct apa102_led* all_leds = apa102_open();
	int is_on = 0;

	if (all_leds == NULL) return 1;

	hsv_fill(&black);

	while(running) {
		if (is_on) {
			hsv_fill(&black);
			is_on = 0;
		} else {
			hsv_fill(&color);
			is_on = 1;
		}
		apa102_sync();

		if (is_on) {
			nanosleep(&wait_white, NULL);
		} else {
			nanosleep(&wait_black, NULL);
		}
	}

	apa102_close();
	return 0;
}
