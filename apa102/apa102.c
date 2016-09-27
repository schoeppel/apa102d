
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

#include "apa102.h"

// 1 at the start for the start frame
// 1 at the end for the end frame + some extra end frames for weird timing movements
//   (NUM_LEDS / 2 (half Hz clock drift) / 32 (end frame length))
static struct apa102_led leds[1 + NUM_LEDS + 1 + (NUM_LEDS / 2 / 32)];

static int fd = -1;

static float global_dimming = 1.0;

unsigned long long time_ns() {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);

	return (1000000000ULL * ts.tv_sec + ts.tv_nsec);
}

struct apa102_led apa102_rgb(float rf, float gf, float bf) {
	struct apa102_led led;

	unsigned int r = global_dimming*rf*rf * 8191;
	unsigned int g = global_dimming*gf*gf * 8191;
	unsigned int b = global_dimming*bf*bf * 8191;

	if (r > 8191) r = 8191;
	if (g > 8191) g = 8191;
	if (b > 8191) b = 8191;

	unsigned int max = r;
	if (g > max) max = g;
	if (b > max) max = b;

	if (max < 8) {
		led.global = 0xe0 | 1;
		led.r = r;
		led.g = g;
		led.b = b;

	} else if (max < 512) {
		led.global = 0xe0 | 2;
		led.r = r/2;
		led.g = g/2;
		led.b = b/2;

	} else {
		led.global = 0xe0 | 31;
		led.r = r/32;
		led.g = g/32;
		led.b = b/32;
	}

	return led;
}

struct apa102_led apa102_hsv(float h, float s, float v) {
	float r, g, b;
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

	for (unsigned int i = 0; i < NUM_LEDS; i++)
		leds[1+i] = led_value;
}

struct apa102_led* apa102_open() {
	memset(leds, 0, sizeof(struct apa102_led));
	memset(leds + 1 + NUM_LEDS, 0xff, sizeof(struct apa102_led) * 5);

	for (unsigned int i = 0; i < NUM_LEDS; i++) {
		/* 1/2 brightness */
		leds[1+i].global = 0xe0 | 1;

		leds[1+i].r = 0;
		leds[1+i].g = 0;
		leds[1+i].b = 0;
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

	return leds + 1;
}

void apa102_sync() {
	if (fd < 0) return;

	const void* buf = (const void*)&leds[0];
	size_t buf_len = sizeof(leds);

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
}

const char* get_message_value(const char** message, const char* key, const char* default_value) {
	size_t keylen = strlen(key);

	while (*message) {
		if (memcmp(*message, key, keylen) == 0 && (*message)[keylen] == '=')
			return *message + keylen + 1;

		message++;
	}

	return default_value;
}
