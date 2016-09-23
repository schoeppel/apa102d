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

struct bubble {
		float pos;
		float size;
};

#define PI 3.1415
#define MAX_BUBBLES 50

struct bubble bubbles[MAX_BUBBLES];

static void render(struct apa102_led* led, unsigned int ledcount, float h, float s, float v) {
	unsigned int border = ledcount / 4;

	float str[ledcount];

	float hc = h + 0.5f;
	if (hc > 1.0) hc -= 1.0f;

	for (unsigned int i = 0; i < border; i++) {
		str[i] = 1.0;
	}

	for (unsigned int i = border; i < ledcount; i++) {
		str[i] = 0.0;
	}

	for (unsigned int i = 0; i < MAX_BUBBLES; i++) {
		if (bubbles[i].pos < 0.0) continue;
		else if (bubbles[i].pos > 1.5) { bubbles[i].pos = -1.0; continue; }
		else { bubbles[i].pos += 0.0002; }

		float led_center = bubbles[i].pos * ledcount;
		for (unsigned int y = led_center - bubbles[i].size * ledcount * PI; y <= led_center + bubbles[i].size * ledcount * PI; y++) {
			float intens = cosf(((float)y - led_center) / (bubbles[i].size * ledcount) );

			if (intens > 0.0 && y < ledcount) {
				str[y] += intens;
			}
		}
	}

	for (unsigned int i = 0; i < ledcount; i++) {
		if (str[i] > 1.0) str[i] = 1.0;

		float hblend = (hc * str[i] + h * (1.0 - str[i]));
		led[i] = apa102_hsv(hblend, s, v);
	}
}

int bubbles_main(float h, float s, float v) {

	struct apa102_led* l = apa102_open();
	if (l == NULL) return 1;
	struct particle* p;

	unsigned int count = 0;

	for (unsigned int i = 0; i < MAX_BUBBLES; i++) {
		bubbles[i].pos = -1.0;
	}

	while (running) {
		/* create */
		int index = -1;
		for (unsigned int i = 0; i < MAX_BUBBLES; i++) {
			if (bubbles[i].pos < 0.0) {
				index = i;
				break;
			}
		}

		if (index >= 0 && rand() % 1024 == 0) {
			bubbles[index].pos = 0.0;
			bubbles[index].size = (rand() % 40 + 10) / 1000.0;
		}

		unsigned long long time_start = time_ns();
		render(l, NUM_LEDS, h, s, v);
		apa102_sync();

		unsigned long long time_end = time_ns();
		unsigned int frame_us = (time_end - time_start) / 1000;
		int sleep = 10000 - frame_us;

		if (sleep > 0) usleep(sleep); else
		printf("warning %i us\n", sleep);

	}

	apa102_close();

	return 0;
}
