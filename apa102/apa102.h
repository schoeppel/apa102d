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

struct hsv_t {
	float h;
	float s;
	float v;
};

unsigned long long time_ns();
struct apa102_led* apa102_open();
void apa102_sync();
void apa102_close();

struct apa102_led apa102_rgb(float rf, float gf, float bf);
struct apa102_led apa102_hsv(float h, float s, float v);
struct apa102_led apa102_hsv_t(struct hsv_t* hsv);

struct apa102_led hsv_fade(struct hsv_t* source, struct hsv_t* target, float targetPercent);
void hsv_fill(struct hsv_t* color);

const char* get_message_value(const char** message, const char* key, const char* default_value);
struct hsv_t parse_hsv_color(const char* string);

#endif
