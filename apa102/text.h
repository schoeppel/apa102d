/*
 * text.h
 *
 *  Created on: 27.12.2016
 *      Author: sebastian
 */

#ifndef TEXT_H_
#define TEXT_H_


void* text_step(void* last_state,
                const char** message,
                unsigned long long timestamp,
                struct apa102_led* leds,
                int nr_leds,
                int leds_per_meter);

void text_destroy(void* last_state);




#endif /* TEXT_H_ */
