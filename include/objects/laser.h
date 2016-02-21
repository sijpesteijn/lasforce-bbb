/*
 * laser.h
 *
 *  Created on: Jan 10, 2016
 *      Author: gijs
 */

#ifndef LASER_H_
#define LASER_H_

#include "object.h"
#include "../io/spi.h"
#include "../io/gpio.h"

//#define MCP4922_CS 60;
//#define MCP4902_1_CS 15;
//#define MCP4902_2_CS 48;

#define MCP23S08_ALL_DOWN		0x00
//
#define MCP4902_1_CS	 		0x01		//GPIO_0 0000 0001
#define MCP4902_2_CS 			0x02		//GPIO_1 0000 0010
#define MCP4922_CS 				0x04		//GPIO_2 0000 0100
#define MCP23S08_LDAC			0x08		//GPIO_3 0000 1000
#define MCP23S08_SCK			0x10		//GPIO_4 0001 0000
#define MCP23S08_SDI			0x20		//GPIO_5 0010 0000
#define MCP23S08_SHUTTER		0x40		//GPIO_6 0100 0000
#define MCP23S08_SHUTTER_RETURN	0x80		//GPIO_7 1000 0000
#define AXIS_MIN				0
#define AXIS_MAX				4095

/**
 * Coordinates
 * min: -32768
 * max: +32767
 *
 * Colors:
 * min: 0
 * max: 255
 */

struct Laser {
	Object proto;
	int x;
	int y;
	int pixels_per_bit;
	spi_properties *spi;
	gpio_properties *axis_gpio;
	gpio_properties *axis_ldac_gpio;
	gpio_properties *colors1_gpio;
	gpio_properties *colors2_gpio;
};

typedef struct Laser Laser;

int Laser_init(void *self, void *object);
void Laser_describe(void *self);
int Laser_setCoordinate(void *self, int x, int y);
int Laser_setRed(void *self, int red);
int Laser_setGreen(void *self, int green);
int Laser_setBlue(void *self, int blue);
void Laser_destroy(void *self);
#endif /* LASER_H_ */
