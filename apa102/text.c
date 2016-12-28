/*
 * text.c
 *
 *  Created on: 27.12.2016
 *      Author: sebastian
 */

#include "apa102.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define _____ 0
#define ____X 1
#define ___X_ 2
#define ___XX 3
#define __X__ 4
#define __X_X 5
#define __XX_ 6
#define __XXX 7
#define _X___ 8
#define _X__X 9
#define _X_X_ 10
#define _X_XX 11
#define _XX__ 12
#define _XX_X 13
#define _XXX_ 14
#define _XXXX 15
#define X____ 16
#define X___X 17
#define X__X_ 18
#define X__XX 19
#define X_X__ 20
#define X_X_X 21
#define X_XX_ 22
#define X_XXX 23
#define XX___ 24
#define XX__X 25
#define XX_X_ 26
#define XX_XX 27
#define XXX__ 28
#define XXX_X 29
#define XXXX_ 30
#define XXXXX 31
/*
	_____,
	_____,
	_____,
	_____,
	_____,


*/

unsigned char font5[] = {
		__X__,
		__X__,
		__X__,
		_____,
		__X__,

		_X_X_,
		_X_X_,
		_____,
		_____,
		_____,

		_X_X_,
		XXXXX,
		_X_X_,
		XXXXX,
		_X_X_,

		_XXXX,
		X_X__,
		_XXX_,
		__X_X,
		XXXX_,

		XX__X,
		XX_X_,
		__X__,
		_X_XX,
		X__XX,

		_XX__,
		X_X__,
		_XX_X,
		X__X_,
		_XX_X,

		__X__,
		__X__,
		_____,
		_____,
		_____,

		__X__,
		_X___,
		_X___,
		_X___,
		__X__,

		__X__,
		___X_,
		___X_,
		___X_,
		__X__,

		_X_X_,
		__X__,
		_X_X_,
		_____,
		_____,

		__X__,
		__X__,
		XXXXX,
		__X__,
		__X__,

		_____,
		_____,
		_____,
		__X__,
		_X___,

		_____,
		_____,
		XXXXX,
		_____,
		_____,

		_____,
		_____,
		_____,
		_XX__,
		_XX__,


		____X,
		___X_,
		__X__,
		_X___,
		X____,

		_XXX_,
		X___X,
		X___X,
		X___X,
		_XXX_,

		__XX_,
		_X_X_,
		___X_,
		___X_,
		___X_,

		XXXX_,
		____X,
		__XX_,
		_X___,
		XXXXX,

		XXXX_,
		____X,
		__XX_,
		____X,
		XXXX_,

		___X_,
		__XX_,
		_X_X_,
		XXXXX,
		___X_,

		XXXXX,
		X____,
		XXXX_,
		____X,
		XXXX_,

		_XXXX,
		X____,
		XXXX_,
		X___X,
		_XXX_,

		XXXXX,
		____X,
		___X_,
		__X__,
		_X___,

		_XXX_,
		X___X,
		_XXX_,
		X___X,
		_XXX_,

		_XXX_,
		X___X,
		_XXXX,
		____X,
		XXXX_,

		_____,
		__X__,
		_____,
		__X__,
		_____,

		_____,
		__X__,
		_____,
		__X__,
		_X___,

		___X_,
		__X__,
		_X___,
		__X__,
		___X_,

		_____,
		XXXXX,
		_____,
		XXXXX,
		_____,

		_X___,
		__X__,
		___X_,
		__X__,
		_X___,

		XXX__,
		___X_,
		__X__,
		_____,
		__X__,

		_XXX_,
		X_X_X,
		X__XX,
		X____,
		_XXX_,


		__X__,
		_X_X_,
		X___X,
		XXXXX,
		X___X,

		XXXX_,
		X___X,
		XXXXX,
		X___X,
		XXXX_,

		_XXXX,
		X____,
		X____,
		X____,
		_XXXX,

		XXXX_,
		X___X,
		X___X,
		X___X,
		XXXX_,

		XXXXX,
		X____,
		XXXX_,
		X____,
		XXXXX,

		XXXXX,
		X____,
		XXXX_,
		X____,
		X____,

		_XXXX,
		X____,
		X__XX,
		X___X,
		_XXXX,

		X___X,
		X___X,
		XXXXX,
		X___X,
		X___X,

		XXXXX,
		__X__,
		__X__,
		__X__,
		XXXXX,

		XXXXX,
		____X,
		____X,
		____X,
		__XX_,

		X__XX,
		X_X__,
		XX___,
		X_X__,
		X__XX,

		X____,
		X____,
		X____,
		X____,
		XXXXX,

		X___X,
		XX_XX,
		X_X_X,
		X___X,
		X___X,

		X___X,
		XX__X,
		X_X_X,
		X__XX,
		X___X,

		_XXX_,
		X___X,
		X___X,
		X___X,
		_XXX_,

		XXXX_,
		X___X,
		XXXX_,
		X____,
		X____,

		_XXX_,
		X___X,
		X___X,
		X__XX,
		_XXXX,

		XXXX_,
		X___X,
		XXXX_,
		X__X_,
		X___X,

		_XXXX,
		X____,
		_XXX_,
		____X,
		XXXX_,

		XXXXX,
		__X__,
		__X__,
		__X__,
		__X__,

		X___X,
		X___X,
		X___X,
		X___X,
		_XXX_,

		X___X,
		X___X,
		X___X,
		_X_X_,
		__X__,

		X___X,
		X___X,
		X_X_X,
		XX_XX,
		X___X,

		X___X,
		_X_X_,
		__X__,
		_X_X_,
		X___X,

		X___X,
		_X_X_,
		__X__,
		_X___,
		X____,

		XXXXX,
		___X_,
		__X__,
		_X___,
		XXXXX,
};

struct text_effect_state {
	struct hsv_t color;
	struct hsv_t bgcolor;
	char text[10];
};



static void render_char(struct apa102_led* led, unsigned int xo, unsigned int yo, struct hsv_t *color, char c) {
	unsigned int index = c - '!';
	if (index > sizeof(font5)/5) return;

	for (int x = 0; x < 5; x++) {
		for (int y = 0; y < 5; y++) {
			if (font5[index*5+y] & (0x10 >> x)) {
				led[from_xy(x + xo, y + yo)] = apa102_hsv_t(color);


			}
		}
	}
}

static void render(struct apa102_led* led, unsigned int ledcount, struct text_effect_state* state) {
	hsv_fill(&state->bgcolor);

	for (unsigned int i = 0; i < sizeof(state->text); i++) {
		render_char(led, i*6, 0, &state->color, state->text[i]);
	}
}


void* text_step(void* last_state,
                const char** message,
                unsigned long long timestamp,
                struct apa102_led* leds,
                int nr_leds,
                int leds_per_meter) {

	if (last_state == NULL) {
		last_state = malloc(sizeof(struct text_effect_state));
		memset(last_state, 0, sizeof(struct text_effect_state));
	}
	struct text_effect_state* state = (struct text_effect_state*)last_state;

	state->color = parse_hsv_color(get_message_value(message, "color", "hsv(0.0,1.0,0.15)"));
	state->bgcolor = parse_hsv_color(get_message_value(message, "bgcolor", "hsv(0.0,1.0,0.0)"));

	strncpy(state->text, get_message_value(message, "text", "Hallo Welt"), sizeof(state->text));

	render(leds, nr_leds, state);

	return last_state;
}

void text_destroy(void* last_state) {
	free(last_state);
}


