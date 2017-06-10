#ifndef APA102_H
#define APA102_H

#include <inttypes.h>

struct apa102_led_data {
	// global brightness (0xE0 | 1 to 31)
	uint8_t global;

	uint8_t b;
	uint8_t g;
	uint8_t r;
} __attribute__((packed));

struct apa102_led {
	float r;
	float g;
	float b;
};

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

unsigned char xy_valid( int x,  int y);
unsigned int from_xy(unsigned int x, unsigned int y);
unsigned int get_width();
unsigned int get_height();

unsigned int is_set(struct apa102_led *l);

void print_hsv(struct hsv_t* hsv);

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

#endif
