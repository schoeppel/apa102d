/*
 * color.c
 *
 *  Created on: 29.08.2015
 *      Author: sebastian
 */

#include "color.h"

#define HI8(a) ((a)>>8)

// h 0 - 256*6-1

struct color_rgb hsv_to_rgbw(uint16_t h, uint8_t s, uint8_t v) {
	uint8_t r, g, b, i, f;
	uint8_t p, q, t;


	if( s == 0 ) {
		r = g = b = v;
	} else {

		i = h / 256;
		f = h % 256;

		p = HI8((uint16_t)v * (255U - (uint16_t)s));
		q = HI8((uint16_t)v * (254U - HI8((uint16_t)s * (uint16_t)f)));
		t = HI8((uint16_t)v * (254U - HI8((uint16_t)s * (255U-(uint16_t)f))));

		switch( i ) {
			default:
			case 0:
				r = v; g = t; b = p; break;
			case 1:
				r = q; g = v; b = p; break;
			case 2:
				r = p; g = v; b = t; break;
			case 3:
				r = p; g = q; b = v; break;
			case 4:
				r = t; g = p; b = v; break;
			case 5:
	 			r = v; g = p; b = q; break;
		}
	}

	struct color_rgb ret;


	ret.r = r;
	ret.g = g;
	ret.b = b;

	return ret;

}
