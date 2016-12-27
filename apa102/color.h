/*
 * color.h
 *
 *  Created on: 29.08.2015
 *      Author: sebastian
 */

#ifndef COLOR_H_
#define COLOR_H_

#include <inttypes.h>

struct color_rgb {
	uint8_t dummy;
	uint8_t r;
	uint8_t g;
	uint8_t b;
} __attribute__((packed)) __attribute__((deprecated));

struct color_rgb hsv_to_rgbw(uint16_t h, uint8_t s, uint8_t v)  __attribute__((deprecated));

#endif /* COLOR_H_ */
