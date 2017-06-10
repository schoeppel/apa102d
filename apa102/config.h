/*
 * config.h
 *
 *  Created on: 27.12.2016
 *      Author: sebastian
 */

#ifndef CONFIG_H_
#define CONFIG_H_

struct device_config {
	unsigned int num_leds;
	unsigned int num_cols;

	unsigned int max_current;
	unsigned int max_current_segment;
	unsigned int max_current_segment_size;
	
	unsigned int poweron_gpio;
};

extern struct device_config *devconfig;

void config_init();

#endif /* CONFIG_H_ */
