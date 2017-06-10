#include "apa102.h"

#include "config.h"

#include <string.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <time.h>
#include <math.h>



// 1 at the start for the start frame
// 1 at the end for the end frame

static int poweron_state = 0;

static struct apa102_led *leds = NULL;
static int fd = -1;

unsigned long long time_ns() {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);

	return (1000000000ULL * ts.tv_sec + ts.tv_nsec);
}

struct apa102_led apa102_rgb(float rf, float gf, float bf) {
	struct apa102_led led;
	led.r = rf;
	led.g = gf;
	led.b = bf;
	return led;
}

struct apa102_led apa102_hsv(float h, float s, float v) {
	float r = 0.0, g = 0.0, b = 0.0;
	float F, M, N, K;
	int i;

	if (s == 0.0) {
		r = g = b = v;
	} else {
		if (h == 1.0)
			h = 0.0;
		h *= 6.0;
		i = floorf(h);
		F = h - i;
		M = v * (1 - s);
		N = v * (1 - s * F);
		K = v * (1 - s * (1 - F));

		switch (i) {
			case 0: r = v; g = K; b = M; break;
			case 1: r = N; g = v; b = M; break;
			case 2: r = M; g = v; b = K; break;
			case 3: r = M; g = N; b = v; break;
			case 4: r = K; g = M; b = v; break;
			case 5: r = v; g = M; b = N; break;
		}
	}

	return apa102_rgb(r, g, b);
}

struct apa102_led apa102_hsv_t(struct hsv_t* hsv) {
	return apa102_hsv(hsv->h, hsv->s, hsv->v);
}

struct apa102_led hsv_fade(struct hsv_t* source, struct hsv_t* target, float targetPercent) {
	float sourcePercent = 1 - targetPercent;
	return apa102_hsv(
    source->h * sourcePercent + target->h * targetPercent,
		source->s * sourcePercent + target->s * targetPercent,
		source->v * sourcePercent + target->v * targetPercent
	);
}

void hsv_fill(struct hsv_t* color) {
	struct apa102_led led_value = apa102_hsv_t(color);

	for (unsigned int i = 0; i < devconfig->num_leds; i++)
		leds[i] = led_value;
}

struct apa102_led* apa102_open() {
	char path[64];
	snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d", devconfig->poweron_gpio);
	if (access(path, F_OK)) {
		FILE* f = fopen("/sys/class/gpio/export", "w");
		if (f) {
			fprintf(f, "%d", devconfig->poweron_gpio);
			fclose(f);
		}
	}

	fd = open("/dev/spidev0.0", O_RDWR);
	if (fd < 0) {
		perror("spidev open");
		fd = open("/dev/null", O_RDWR);
	}

	uint32_t max_hz = 10000000;
	if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &max_hz) < 0) {
		perror("set speed");

		/*close(fd);
		fd = -1;
		return NULL;*/
	}

	uint8_t mode = SPI_MODE_3;

	if (ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0) {
                perror("mode");

                /*close(fd);
                fd = -1;
                return NULL;*/
        }

	leds = malloc(devconfig->num_leds * sizeof(leds[0]));

	return leds;
}

static void apa102_poweron(int power) {
	char path[128];
	snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", devconfig->poweron_gpio);
	FILE* f = fopen(path, "w");
	if (! f) return;
	
	if (power) fprintf(f, "high");
	else fprintf(f, "low");
	
	fclose(f);
}

static int apa102_currentcontrol(struct apa102_led* data, unsigned int len) {
	float segment_current[len / devconfig->max_current_segment_size];
	memset(segment_current, 0, sizeof(segment_current));

	float current = 0.0f;

	for (unsigned int i = 0; i < len; i++) {
		float led_current = data[i].r * 23.1f  + data[i].g * 18.6f  + data[i].b * 18.3f;
		segment_current[i / devconfig->max_current_segment_size] += led_current;
		current += led_current;
	}

	if (current < 0.001) return 0;

	float power_factor = 1.0f;
	if (current > devconfig->max_current) {
		power_factor = devconfig->max_current / current;
	}

	for (unsigned int i = 0; i < len / devconfig->max_current_segment_size; i++) {
		float factor = devconfig->max_current_segment / segment_current[i];
		if (factor < power_factor) power_factor = factor;
	}

	if (power_factor >= 0.999f) return 1;

	for (unsigned int i = 0; i < len; i++) {
		data[i].r *= power_factor;
		data[i].g *= power_factor;
		data[i].b *= power_factor;
	}
	
	return 1;
}

void apa102_sync() {
	if (fd < 0) return;

	struct apa102_led_data data[1 + devconfig->num_leds + devconfig->num_leds/64 + 1];
	struct apa102_led data_gamma[devconfig->num_leds];

	for (unsigned int i = 0; i < devconfig->num_leds; i++) {
		data_gamma[i].r = leds[i].r * leds[i].r;
		data_gamma[i].g = leds[i].g * leds[i].g;
		data_gamma[i].b = leds[i].b * leds[i].b;
	}

	int on = apa102_currentcontrol(data_gamma, devconfig->num_leds);
	if (poweron_state != on) {
		apa102_poweron(on);
		poweron_state = on;
	}
	

	memset(data, 0, sizeof(data[0]));
	memset(data + 1 + devconfig->num_leds, 0xff, sizeof(data[0]) * (devconfig->num_leds/64 + 1));


	for (unsigned int i = 0; i < devconfig->num_leds; i++) {
		unsigned int r = data_gamma[i].r * 8191;
		unsigned int g = data_gamma[i].g * 8191;
		unsigned int b = data_gamma[i].b * 8191;

		if (r > 8191) r = 8191;
		if (g > 8191) g = 8191;
		if (b > 8191) b = 8191;

		unsigned int max = r;
		if (g > max) max = g;
		if (b > max) max = b;

		if (max < 8) {
			data[i+1].global = 0xe0 | 1;
			data[i+1].r = r;
			data[i+1].g = g;
			data[i+1].b = b;

		} else if (max < 512) {
			data[i+1].global = 0xe0 | 2;
			data[i+1].r = r/2;
			data[i+1].g = g/2;
			data[i+1].b = b/2;

		} else {
			data[i+1].global = 0xe0 | 31;
			data[i+1].r = r/32;
			data[i+1].g = g/32;
			data[i+1].b = b/32;
		}
	}

	const void* buf = (const void*)&data[0];
	size_t buf_len = sizeof(data);

	while (buf_len > 0) {
		int bytes = write(fd, buf, buf_len);

		if (bytes == -1) {
			if (errno == EINTR)
				continue;
			else
				return;
		}

		buf += bytes;
		buf_len -= bytes;
	}
}

void apa102_close() {
	close(fd);
	fd = -1;

	free(leds);
	leds = NULL;
}

const char* get_message_value(const char** message, const char* key, const char* default_value) {
	size_t keylen = strlen(key);

	while (*message) {
		if (strlen(*message) > keylen + 1
		    && memcmp(*message, key, keylen) == 0
		    && (*message)[keylen] == '=')
			return *message + keylen + 1;

		message++;
	}

	return default_value;
}

struct hsv_t parse_hsv_color(const char* string) {
	struct hsv_t result;
	int num_scanned = sscanf(string, "hsv(%f,%f,%f)", &result.h, &result.s, &result.v);

	if (num_scanned != 3) {
		result.h = 0.0f;
		result.s = 0.0f;
		result.v = 0.0f;
	}
	return result;
}

unsigned char xy_valid( int x,  int y) {
	if (x < 0 || y < 0) return 0;

	if (x >= devconfig->num_leds / devconfig->num_cols) return 0;
	if (y >= devconfig->num_cols) return 0;

	return 1;

}

unsigned int from_xy(unsigned int x, unsigned int y) {
	unsigned int row_len = (devconfig->num_leds / devconfig->num_cols);

	if (x >= row_len) return 0;
	if (y >= devconfig->num_cols) return 0;

	if (y % 2 == 1) {
		x = row_len - 1 - x;
	}

	return y*row_len + x;
}

unsigned int get_width() {
	return (devconfig->num_leds / devconfig->num_cols);

}

unsigned int get_height() {
	return devconfig->num_cols;
}

unsigned int is_set(struct apa102_led *l) {
	if (l->r > 0.01 || l->g > 0.01 || l->b > 0.01) return 1;

	return 0;

}

void print_hsv(struct hsv_t* hsv) {
	printf("hsv(%f,%f,%f)\n", hsv->h, hsv->s, hsv->v);
}
