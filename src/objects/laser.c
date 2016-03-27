/*
 * laser.c
 *
 *  Created on: Jan 10, 2016
 *      Author: gijs
 */

#include "../../include/objects/laser.h"
#include <fcntl.h>
#include <syslog.h>

FILE *openFile(int gpio_nr) {
	FILE *fd;
	char buf[MAX_BUF];

	snprintf(buf, sizeof(buf), SYSFS_GPIO_DIR "/gpio%d/value", gpio_nr);

	fd = fopen(buf, "w");
	if (fd < 0) {
		printf("gpio/set-value");
		return NULL;
	}

	return fd;
}

void chip_select(gpio_properties *gpio, char *value) {
	fputs(value, gpio->value_file_descriptor);
	fflush(gpio->value_file_descriptor);
}

int Laser_init(void *self) {
	Laser *laser = self;

	laser->pixels_per_bit = 65535/AXIS_MAX;

	// Start log and open slots file
	init_bbc_lib();

	// Load the spi_dev0 device tree overlay
	overlay *ol = malloc(sizeof(overlay));
	ol->file_name = "BBCLIB-SPI0";
	ol->board_name = "BBCLib";
	ol->manufacturer = "BBCLIB";
	ol->part_number = "BBCLIB-SPI0";
	ol->version = "00A0";
	load_device_tree_overlay(ol);
	free(ol);

	// Setup mcp49xx (gpio's)
	laser->axis_gpio = malloc(sizeof(gpio_properties));
	laser->axis_gpio->nr = 60;
	laser->axis_gpio->direction = OUTPUT_PIN;
	laser->axis_ldac_gpio = malloc(sizeof(gpio_properties));
	laser->axis_ldac_gpio->nr = 115;
	laser->axis_ldac_gpio->direction = OUTPUT_PIN;
	laser->colors1_gpio = malloc(sizeof(gpio_properties));
	laser->colors1_gpio->nr = 15;
	laser->colors1_gpio->direction = OUTPUT_PIN;
	laser->colors2_gpio = malloc(sizeof(gpio_properties));
	laser->colors2_gpio->nr = 48;
	laser->colors2_gpio->direction = OUTPUT_PIN;

	// Setup spi bus
	laser->spi = malloc(sizeof(spi_properties));
	laser->spi->spi_id = spi0;
	laser->spi->bits_per_word = 8;
	laser->spi->mode = 0;
	laser->spi->speed = 10000000;
	laser->spi->flags = O_RDWR;

	uint8_t isOpen = spi_open(laser->spi);
	uint8_t isAxisOpen = gpio_open(laser->axis_gpio);
	uint8_t isAxisLDACOpen = gpio_open(laser->axis_ldac_gpio);
	uint8_t isColors1Open = gpio_open(laser->colors1_gpio);
	uint8_t isColors2Open = gpio_open(laser->colors2_gpio);

	if (isOpen == 0 && isAxisOpen == 0 && isAxisLDACOpen == 0 &&  isColors1Open == 0 && isColors2Open == 0) {
		laser->axis_gpio->value_file_descriptor = openFile(laser->axis_gpio->nr);
		laser->axis_ldac_gpio->value_file_descriptor = openFile(laser->axis_ldac_gpio->nr);
		laser->colors1_gpio->value_file_descriptor = openFile(laser->colors1_gpio->nr);
		laser->colors2_gpio->value_file_descriptor = openFile(laser->colors2_gpio->nr);

		chip_select(laser->axis_gpio, "1");
		chip_select(laser->axis_ldac_gpio, "1");
		chip_select(laser->colors1_gpio, "1");
		chip_select(laser->colors1_gpio, "1");
		laser->x = 0;
		laser->y = 0;

		syslog(LOG_INFO, "%s", "Laser initialized.");
		return 1;
	}
	syslog(LOG_INFO, "%s", "FAILED to initialize Laser.");
	return 1;
}

void write8Bits(spi_properties *spi, unsigned char reg, unsigned char value) {
	unsigned char data[2] = {};
	data[0] = reg | ((value & 0xf0) >> 4);
	data[1] = (value & 0x0f) << 4;
	if (spi_send(spi, data, sizeof(data)) == -1) {
		perror("Failed to update output.");
	}
}

void write12Bits(spi_properties *spi, unsigned char reg, unsigned short value) {
	unsigned char data[2] = {};
	data[0] = reg | ((value & 0xff00) >> 8);
	data[1] = (value & 0x00ff);
	if (spi_send(spi, data, sizeof(data)) == -1) {
		perror("Failed to update output.");
	}
}

int Laser_setCoordinate(void *self, int x, int y) {
	Laser *laser = self;
	laser->x = x;
	laser->y = y;
	chip_select(laser->axis_gpio, "0");
	write12Bits(laser->spi, 0x70, x);
	chip_select(laser->axis_gpio, "1");
	chip_select(laser->axis_gpio, "0");
	write12Bits(laser->spi, 0xf0, y);
	chip_select(laser->axis_gpio, "1");
	chip_select(laser->axis_ldac_gpio, "0");
//	usleep(10);
	chip_select(laser->axis_ldac_gpio, "1");
	return 0;
}

int Laser_setRed(void *self, int red) {
	Laser *laser = self;
//	syslog(LOG_DEBUG, "Laser setRed value: %i.", red);
	chip_select(laser->colors1_gpio, "0");
	write8Bits(laser->spi, 0x70, red);
	chip_select(laser->colors1_gpio, "1");
	return 0;
}

int Laser_setGreen(void *self, int green) {
	Laser *laser = self;
//	syslog(LOG_DEBUG, "Laser setGreen value: %i.", green);
	chip_select(laser->colors1_gpio, "0");
	write8Bits(laser->spi, 0xf0, green);
	chip_select(laser->colors1_gpio, "1");
	return 0;
}

int Laser_setBlue(void *self, int blue) {
	Laser *laser = self;
//	syslog(LOG_DEBUG, "Laser setBlue value: %i.", blue);
	chip_select(laser->colors2_gpio, "0");
	write8Bits(laser->spi, 0xf0, blue);
	chip_select(laser->colors2_gpio, "1");
	return 0;
}

void Laser_destroy(void *self) {
	Laser *laser = self;
	free(laser->spi);
	gpio_close(laser->axis_gpio);
	free(laser->axis_gpio);
	gpio_close(laser->colors1_gpio);
	free(laser->colors1_gpio);
	gpio_close(laser->colors2_gpio);
	free(laser->colors2_gpio);
	free(laser);
	syslog(LOG_INFO, "%s", "Laser destroyed.");
}
