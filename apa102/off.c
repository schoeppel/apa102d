/*
 * off.c
 *
 *  Created on: 28.09.2016
 *      Author: sebastian
 */


#include "off.h"

void* off_step(void* last_state,
                const char** message,
                unsigned long long timestamp,
                struct apa102_led* leds,
                int nr_leds,
                int leds_per_meter) {

	for (int i = 0; i < nr_leds; i++) {
		leds[i] = apa102_rgb(0.0, 0.0, 0.0);
	}

	return last_state;
}

void off_destroy(void* last_state) {

}
