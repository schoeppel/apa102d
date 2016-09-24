#include "apa102.h"

#include <inttypes.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "color.h"

extern int running;

int waves_main(float period, float speed) {

	struct apa102_led* l = apa102_open();
	if (l == NULL) return 1;

	float offset = 0;

	while (running) {
		unsigned long long time_start = time_ns();

		for (int i = 0; i < NUM_LEDS; i++) {
			float x = ((float)i * 3.1415) / (NUM_LEDS * period);

			float v = (sinf( x + offset ) + 1.0) / 2.0;

			l[i] = apa102_hsv(v, 1, 1);

			offset += speed;
			if (offset > 2*3.1415) offset -= 2*3.1415;
		}

		apa102_sync();

		unsigned long long time_end = time_ns();
		unsigned int frame_us = (time_end - time_start) / 1000;
		int sleep = 5000 - frame_us;
		if (sleep > 0) usleep(sleep); else
		printf("warning %i us\n", sleep);
	}

	apa102_close();

	return 0;
}
