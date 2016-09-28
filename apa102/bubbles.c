#include "apa102.h"

#include <inttypes.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "color.h"


#define PI 3.1415
#define MAX_BUBBLES 50


struct bubble {
		float pos;
		float size;
};


struct bubbles_effect_state {
	struct bubble bubbles[MAX_BUBBLES];

	struct hsv_t color1;
	struct hsv_t color2;
};


static void render(struct apa102_led* led, unsigned int ledcount, struct bubbles_effect_state* state) {
	unsigned int border = ledcount / 4;

	float str[ledcount];

	for (unsigned int i = 0; i < border; i++) {
		str[i] = 1.0;
	}

	for (unsigned int i = border; i < ledcount; i++) {
		str[i] = 0.0;
	}

	for (unsigned int i = 0; i < MAX_BUBBLES; i++) {
		if (state->bubbles[i].pos < 0.0) continue;
		else if (state->bubbles[i].pos > 1.5) { state->bubbles[i].pos = -1.0; continue; }
		else { state->bubbles[i].pos += 0.0002; }

		float led_center = state->bubbles[i].pos * ledcount;
		for (unsigned int y = led_center - state->bubbles[i].size * ledcount * PI; y <= led_center + state->bubbles[i].size * ledcount * PI; y++) {
			float intens = cosf(((float)y - led_center) / (state->bubbles[i].size * ledcount) );

			if (intens > 0.0 && y < ledcount) {
				str[y] += intens;
			}
		}
	}

	for (unsigned int i = 0; i < ledcount; i++) {
		if (str[i] > 1.0) str[i] = 1.0;

		led[i] = hsv_fade(&state->color1, &state->color2, str[i]);
	}
}


void* bubbles_step(void* last_state,
                const char** message,
                unsigned long long timestamp,
                struct apa102_led* leds,
                int nr_leds,
                int leds_per_meter) {

	if (last_state == NULL) {
		last_state = malloc(sizeof(struct bubbles_effect_state));
		memset(last_state, 0, sizeof(struct bubbles_effect_state));
	}
	struct bubbles_effect_state* state = (struct bubbles_effect_state*)last_state;

	state->color1 = parse_hsv_color(get_message_value(message, "color", "hsv(0.0,1.0,0.15)"));
	state->color2 = parse_hsv_color(get_message_value(message, "color2", "hsv(0.5,1.0,0.15)"));

	int index = -1;
	for (unsigned int i = 0; i < MAX_BUBBLES; i++) {
		if (state->bubbles[i].pos < 0.0) {
			index = i;
			break;
		}
	}

	if (index >= 0 && rand() % 1024 == 0) {
		state->bubbles[index].pos = 0.0;
		state->bubbles[index].size = (rand() % 40 + 10) / 1000.0;
	}

	render(leds, nr_leds, state);

	return last_state;
}

void bubbles_destroy(void* last_state) {
	free(last_state);
}
