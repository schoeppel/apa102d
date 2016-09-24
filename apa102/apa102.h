#ifndef APA102_H
#define APA102_H

#define NUM_LEDS 288

#include <inttypes.h>

struct apa102_led {
  // global brightness (0xE0 | 1 to 31)
	uint8_t global;

	uint8_t b;
	uint8_t g;
	uint8_t r;
} __attribute__((packed));

unsigned long long time_ns();
struct apa102_led* apa102_open();
void apa102_sync();
void apa102_close();

struct apa102_led apa102_rgb(float rf, float gf, float bf);
struct apa102_led apa102_hsv(float h, float s, float v);

#endif
