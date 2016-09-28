#ifndef PARTICLES_H
#define PARTICLES_H

void* particles_step(void* last_state,
                const char** message,
                unsigned long long timestamp,
                struct apa102_led* leds,
                int nr_leds,
                int leds_per_meter);

void particles_destroy(void* last_state);




#endif
