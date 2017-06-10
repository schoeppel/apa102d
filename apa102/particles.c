#include "apa102.h"

#include <inttypes.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "config.h"

#define MAX_PARTICLES 200

struct particle {
	uint32_t flags;

	float pos;
	float len;

	struct apa102_led color;

	float speed;
	float expand_speed;

	float age;
	float ttl;
};


struct particles_effect_state {
	unsigned int count;
	struct particle particles[MAX_PARTICLES];
	unsigned int col_matrix[0];
};

static struct particle* create_particle(unsigned int flags, struct particles_effect_state* state) {
	unsigned int start = 0;

	if (! flags) {
		start = 64;
	}

	for (int i = start; i < MAX_PARTICLES; i++) {
		if (state->particles[i].flags == 0) {
			memset(&state->particles[i], 0, sizeof(struct particle));
			state->particles[i].flags = 1;

			return &state->particles[i];
		}
	}

	return NULL;
}

static void destroy_particle(struct particle* p) {
	p->flags = 0;
}

static void target_add(struct apa102_led* target, struct particle* src, float factor) {
	float r = target->r + src->color.r * factor;
	target->r = (r > 1.0 ? 1.0 : r);

	float g = target->g + src->color.g * factor;
	target->g = (g > 1.0 ? 1.0 : g);

	float b = target->b + src->color.b * factor;
	target->b = (b > 1.0 ? 1.0 : b);
}

static void particle_col(struct particle* a, struct particle* b, struct particles_effect_state* state) {
	if (a->speed * b->speed < 0) {
		if (fabs(fabs(a->speed) - fabs(b->speed))  > 0.2)
			return;

		float pos = (a->pos + b->pos) / 2;
		float len = (a->len + b->len) / 2;
		float speed = (a->speed + b->speed) / 2;
		float ttl = (a->ttl + b->ttl) / 2;

		struct particle* n = create_particle(0, state);
		if (n) {
			n->pos = pos;
			n->len = len;
			n->speed = speed;

			n->color.r = 255;
			n->color.g = 255;
			n->color.b = 255;

			if (ttl == 0.0) {
				n->ttl = 0.2;
				n->expand_speed = 0.2;
			} else {
				n->ttl = ttl;
			}
		}

		for (int i = 0; i < 8; i++) {
			struct particle* n = create_particle(0, state);
			if (n) {

			n->flags |= 2;
			n->pos = pos;
			n->len = 1.0 / (float) devconfig->num_leds;
			n->speed =  (rand() % 64) / 256.0 + speed;

			n->ttl = 0.6;

			if (rand() % 2 == 0) {
				n->speed *= -1;
			}

			if (rand() % 2 == 0) {
				n->color = a->color;
			} else {
				n->color = b->color;
			}

			}

		}

		destroy_particle(a);
		destroy_particle(b);

	}
}

static void render(struct apa102_led* target, unsigned int num_leds, struct particle* src, unsigned int num_particles, struct particles_effect_state* state) {
	static unsigned long long time_last;
	unsigned long long time_current = time_ns();

	unsigned int time_diff = time_current - time_last;
	if (time_last == 0) time_diff = 0;

	for (unsigned int i = 0; i < num_leds; i++) {
		target[i].r = 0;
		target[i].g = 0;
		target[i].b = 0;
	}

	memset(state->col_matrix, 0xff, sizeof(state->col_matrix));

	for (unsigned int i = 0; i < num_particles; i++) {
		if (! src[i].flags)
			continue;

		if (src[i].flags & 2)
			src[i].speed -= (1.0 * time_diff) / 1e9;
		else
			src[i].speed -= (.1 * time_diff) / 1e9;

		src[i].pos += (src[i].speed * time_diff) / 1e9;
		src[i].age += (float)time_diff / 1e9;

		if (src[i].expand_speed != 0.0) {
			src[i].len += (src[i].expand_speed * time_diff) / 1e9;

		}

		float led_start = (src[i].pos - src[i].len) * num_leds;
		float led_end =   (src[i].pos + src[i].len) * num_leds;

		float led_start_a = ceil(led_start);
		float led_end_a = floor(led_end);

		if (
				(led_start_a < -1.0 && led_end_a < -1.0) ||
				(led_start_a > num_leds && led_end_a > num_leds)
		) {
			destroy_particle(src + i);
			continue;
		}

		float ttl_fact = 1.0;
		if (src[i].ttl > 0) {
			if (src[i].expand_speed) {
				ttl_fact = 1.0 - powf(src[i].age / src[i].ttl, 0.2);
			} else {
				ttl_fact = 1.0 - powf(src[i].age / src[i].ttl, 2);
			}
			if (ttl_fact < 0) {
				destroy_particle(src + i);
				continue;
			}
		}

		for (int j = led_start_a - 1; j <= led_end_a; j++) {
			if (j < 0 || j >= (int)num_leds)
				continue;

			if (j < (int)led_start_a) {
				float fact = pow(led_start_a - led_start, 1)  * ttl_fact;
				target_add(target + j, src + i, fact);
			} else if (j == (int)led_end_a) {
				 float fact = pow(led_end - led_end_a, 1) * ttl_fact;
				 target_add(target + j, src + i, fact);

			} else {
				target_add(target + j, src + i, ttl_fact);
			}

			if (i < 64) {

			if (state->col_matrix[j] != 0xffffffff) {
				particle_col(src + state->col_matrix[j], src + i, state);
				if (src[i].flags == 0)
					continue;

			} else {
				state->col_matrix[j] = i;
			}

			}
		}
	}

	time_last = time_current;
}


void* particles_step(void* last_state,
                const char** message,
                unsigned long long timestamp,
                struct apa102_led* leds,
                int nr_leds,
                int leds_per_meter) {

	if (last_state == NULL) {
		int state_size = sizeof(struct particles_effect_state) + sizeof(unsigned int) * devconfig->num_leds;

		last_state = calloc(1, state_size);
	}

	struct particles_effect_state* state = (struct particles_effect_state*)last_state;

	unsigned int particle_freq = 50;

	if (state->count++ % particle_freq == 0) {
		struct particle* p = create_particle(1, state);
		if (p != NULL) {
			p->color = apa102_hsv((float)rand() / RAND_MAX, 1.0, 1.0);

			p->speed = (rand() % 32) / 100.0 + 0.4;
			p->pos = 0;
			p->len = (rand() % 100) / 10000.0;

			if (rand() % 2 == 0) {
				p->pos = 1;
				p->speed = -1.0 + (rand() % 20) / 100.0;
			}
		}
	}

	render(leds, nr_leds, state->particles, MAX_PARTICLES, state);
	return last_state;
}

void particles_destroy(void* last_state) {
	free(last_state);
}
