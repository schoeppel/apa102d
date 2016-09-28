#ifndef BUBBLES_H
#define BUBBLES_H


void* bubbles_step(void* last_state,
                const char** message,
                unsigned long long timestamp,
                struct apa102_led* leds,
                int nr_leds,
                int leds_per_meter);

void bubbles_destroy(void* last_state);




#endif
