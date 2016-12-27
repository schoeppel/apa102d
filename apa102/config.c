/*
 * config.c
 *
 *  Created on: 27.12.2016
 *      Author: sebastian
 */

#include "config.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct device_config *devconfig = NULL;

#define CONFIG_FILE_PATH "/etc/apa102d.conf"

void config_init() {
	static struct device_config config;

	config.num_leds = 288;
	config.num_cols = 1;

	config.max_current = 1000;
	config.max_current_segment = 1000;
	config.max_current_segment_size = 288;

	FILE* f = fopen(CONFIG_FILE_PATH, "r");
	if (f) {
		char lb[128];

		while (fgets(lb, sizeof(lb), f)) {
			char* sep = strchr(lb, ':');
			if (! sep) continue;

			*sep = '\0';

			if (! strcmp(lb, "num_leds")) {
				config.num_leds = atoi(sep + 1);
			} else if (! strcmp(lb, "num_cols")) {
				config.num_cols = atoi(sep + 1);
			} else if (! strcmp(lb, "max_current")) {
				config.max_current = atoi(sep + 1);
			} else if (! strcmp(lb, "max_current_segment")) {
				config.max_current_segment = atoi(sep + 1);
			} else if (! strcmp(lb, "max_current_segment_size")) {
				config.max_current_segment_size = atoi(sep + 1);
			}
		}

	} else {
		printf("Could not open config file: %s. Using defaults.\n", CONFIG_FILE_PATH);
	}

	printf("device config:\n");
	printf("num_leds: %u\n", config.num_leds);
	printf("num_cols: %u\n", config.num_leds);
	printf("max_current: %u\n", config.max_current);
	printf("max_current_segment: %u\n", config.max_current_segment);
	printf("max_current_segment_size: %u\n", config.max_current_segment_size);

	devconfig = &config;

}
