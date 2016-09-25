#include "apa102.h"

#include <stdlib.h>

#include "color.h"

extern int running;

int singlecolor_main(struct apa102_led color) {
	struct apa102_led* l = apa102_open();
	if (l == NULL) return 1;

	for (int i = 0; i < NUM_LEDS; i++)
		l[i] = color;

	while(running) {
		apa102_sync();
		sleep(1);
	}

	apa102_close();
	return 0;
}

int rgb_main(float r, float g, float b) {
	return singlecolor_main(apa102_rgb(r, g, b));
}

int hsv_main(float h, float s, float v) {
	return singlecolor_main(apa102_hsv(h, s, v));
}
