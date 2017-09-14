/*
 * off.c
 *
 *  Created on: 28.09.2016
 *      Author: sebastian
 */

#include "color.h"

#include <stdlib.h>


void* color_step(void* last_state,
                const char** message,
                unsigned long long timestamp,
                struct apa102_led* leds,
                int nr_leds,
                int leds_per_meter) {


	struct hsv_t color = parse_hsv_color(get_message_value(message, "color", "hsv(0.12,0.5,1)"));
    
    unsigned int brightness = atoi(get_message_value(message, "brightness", "255"));

    color.v *= (float)brightness / 255.0f;
    
	hsv_fill(&color);
	return last_state;
}

void color_destroy(void* last_state) {

}
