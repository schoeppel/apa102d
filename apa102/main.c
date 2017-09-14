#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <signal.h>
#include <poll.h>

#include "apa102.h"
#include "config.h"
#include "bubbles.h"
#include "particles.h"
#include "test.h"
#include "off.h"
#include "color.h"
#include "text.h"

#include <mosquitto.h>

char state_topic[128];
char command_topic[128];

char brightness_state_topic[128];
char brightness_command_topic[128];

char rgb_state_topic[128];
char rgb_command_topic[128];

char effect_state_topic[128];
char effect_command_topic[128];


unsigned int value = 1;
unsigned int brightness = 255;
unsigned int rgb[3] = { 10, 10, 10 };
char effect[64] = "off";

char effect_color_spec[128];
char effect_brightness_spec[128];
char* effect_buf[] = { effect_color_spec, effect_brightness_spec, NULL };


struct effect_specification {
	char* name;
	void* (*step)(void*, const char**, unsigned long long, struct apa102_led*, int, int);
	void (*destroy)(void*);
};

static const struct effect_specification effects[] = {
	{
		.name = "off",
		.step = off_step,
		.destroy = off_destroy
	},
	{
		.name = "test",
		.step = test_step,
		.destroy = test_destroy
	},
	{
		.name = "particles",
		.step = particles_step,
		.destroy = particles_destroy
	},
	{
		.name = "bubbles",
		.step = bubbles_step,
		.destroy = bubbles_destroy
	},
	{
		.name = "text",
		.step = text_step,
		.destroy = text_destroy
	},
	{
		.name = "none",
		.step = color_step,
		.destroy = color_destroy
	},
};

const struct effect_specification* current_effect = &effects[0];
void* effect_state = NULL;

volatile int running = 1;

void mqtt_sync_state();

static void sighandler(int sig) {
	running = 0;
}

static void mqtt_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message) {
    char buf[64];
    
    if (! strcmp(message->topic, command_topic)) {
        if (! strcmp(message->payload, "ON")) value = 1; else value = 0;
        
        if (value) {
            mosquitto_publish(mosq, NULL, state_topic, 2, "ON", 2, true);
        } else {
            mosquitto_publish(mosq, NULL, state_topic, 3, "OFF", 2, true);
        }
        
    }  else if (! strcmp(message->topic, brightness_command_topic)) {
        brightness = atoi(message->payload);
        if (brightness > 255) brightness = 255;
        sprintf(buf, "%u", brightness);
        mosquitto_publish(mosq, NULL, brightness_state_topic, strlen(buf), buf, 2, true);
        
    } else if (! strcmp(message->topic, rgb_command_topic)) {
        sscanf(message->payload, "%u,%u,%u", rgb, rgb+1, rgb+2);
        if (rgb[0] > 255) rgb[0] = 255;
        if (rgb[1] > 255) rgb[1] = 255;
        if (rgb[2] > 255) rgb[2] = 255;
        
        sprintf(buf, "%u,%u,%u", rgb[0], rgb[1], rgb[2]);
        mosquitto_publish(mosq, NULL, rgb_state_topic, strlen(buf), buf, 2, true);
        
    } else if (! strcmp(message->topic, effect_command_topic)) {
        snprintf(effect, sizeof(effect), "%s", (char*)message->payload);
        mosquitto_publish(mosq, NULL, effect_state_topic, strlen(effect), effect, 2, true);
    }
    
    mqtt_sync_state();
    
}

static void mqtt_connect_callback(struct mosquitto *mosq, void *userdata, int result) {
	if(!result){
        mosquitto_subscribe(mosq, NULL, command_topic, 2);
        mosquitto_subscribe(mosq, NULL, brightness_command_topic, 2);
        mosquitto_subscribe(mosq, NULL, rgb_command_topic, 2);
        mosquitto_subscribe(mosq, NULL, effect_command_topic, 2);
	} else {
		fprintf(stderr, "Connect failed\n");
	}
}

static void mqtt_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos) {
	int i;

	printf("Subscribed (mid: %d): %d", mid, granted_qos[0]);
	for(i=1; i<qos_count; i++){
		printf(", %d", granted_qos[i]);
	}
	printf("\n");
}

static void mqtt_log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str) {
	printf("%s\n", str);
}


void rgb2hsv(uint8_t src_r, uint8_t src_g, uint8_t src_b, float *dst_h, float *dst_s, float *dst_v) {
    float r = src_r / 255.0f;
    float g = src_g / 255.0f;
    float b = src_b / 255.0f;

    float h, s, v;

    float max = fmaxf(r, fmaxf(g, b));
    float min = fminf(r, fminf(g, b));

    v = max;

    if (max == 0.0f) {
        s = 0;
        h = 0;
    } else if (max - min == 0.0f) {
        s = 0;
        h = 0;
    } else {
        s = (max - min) / max;
        if (max == r) {
            h = 60 * ((g - b) / (max - min)) + 0;
        } else if (max == g) {
            h = 60 * ((b - r) / (max - min)) + 120;
        } else {
            h = 60 * ((r - g) / (max - min)) + 240;
        }
    }

    if (h < 0) h += 360.0f;

    *dst_h = h / 360.0f;
    *dst_s = s;
    *dst_v = v;
}

void mqtt_sync_state() {
    /* switch effect? */
    
    const char* effect_name = effect;
    if (! value) effect_name = "off";
    
	if (strcmp(effect_name, current_effect->name)) {
        current_effect->destroy(effect_state);
        effect_state = NULL;

        for (unsigned int i = 0; i < sizeof(effects)/sizeof(effects[0]); i++) {
            if (strcmp(effects[i].name, effect_name) == 0) {
                current_effect = &effects[i];
            }
        }
    }
    
    float h, s, v;
    rgb2hsv(rgb[0], rgb[1], rgb[2], &h, &s, &v);
    sprintf(effect_color_spec, "color=hsv(%f,%f,%f)", h, s, v);
    sprintf(effect_brightness_spec, "brightness=%u", brightness);
}

int main(int argc, char** argv) {
    char hostname[64];
    gethostname(hostname, sizeof(hostname));
    hostname[63] = '\0';
    
    sprintf(state_topic, "%s/light/status", hostname);
    sprintf(command_topic, "%s/light/switch", hostname);
    sprintf(brightness_state_topic, "%s/brightness/status", hostname);
    sprintf(brightness_command_topic, "%s/brightness/set", hostname);
    sprintf(rgb_state_topic, "%s/rgb/status", hostname);
    sprintf(rgb_command_topic, "%s/rgb/set", hostname);
    sprintf(effect_state_topic, "%s/effect/status", hostname);
    sprintf(effect_command_topic, "%s/effect/set", hostname);
    
    printf("MQTT state topic: %s\n", state_topic);
    
	config_init();
    
    struct mosquitto *mosq = NULL;
    bool clean_session = true;
    mosquitto_lib_init();
	mosq = mosquitto_new(hostname, clean_session, NULL);
    if(! mosq){
		fprintf(stderr, "Error: Out of memory.\n");
		return 1;
	}
	
	mosquitto_log_callback_set(mosq, mqtt_log_callback);
	mosquitto_connect_callback_set(mosq, mqtt_connect_callback);
	mosquitto_message_callback_set(mosq, mqtt_message_callback);
	mosquitto_subscribe_callback_set(mosq, mqtt_subscribe_callback);

	if(mosquitto_connect(mosq, "mqtt", 1883, 60)){
		fprintf(stderr, "Unable to connect.\n");
		return 1;
	}
	
	struct apa102_led* leds = apa102_open();
	
	

	signal(SIGINT, sighandler);

	while (running) {
		unsigned long long frame_start = time_ns();
        
        mosquitto_loop(mosq, 0, 1);
        
		unsigned long long now = time_ns();
		unsigned long long duration = now - frame_start;
		if (duration < 5000000) {
			usleep((5000000 - duration) / 1000);
		}

		/* call current render function */
		effect_state = current_effect->step(effect_state, (const char**)effect_buf, frame_start, leds, devconfig->num_leds, 144);
		apa102_sync();
	}
	
	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();

	current_effect->destroy(effect_state);
	effect_state = NULL;

	/* turn off */
	if (leds == NULL) return 1;
	struct apa102_led black = { 0, 0, 0 };

	for (unsigned int i = 0; i < devconfig->num_leds; i++)
		leds[i] = black;

	apa102_sync();
	apa102_close();

	printf("apa102 exiting\n");

	unlink("/tmp/apa102.pid");
}
