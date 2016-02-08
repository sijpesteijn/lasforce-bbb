/*
 * laser.c
 *
 *  Created on: Jan 10, 2016
 *      Author: gijs
 */

#include "../../include/objects/laser.h"

int Laser_init(void *self) {
	Laser *laser = self;
	unsigned char mcp23s08_setup[] = { 0x40, 0x00, 0x00, };

	init_bbc_lib();
	overlay *ol = malloc(sizeof(overlay));
	ol->file_name = "BBCLIB-SPI0";
	ol->board_name = "BBCLib";
	ol->manufacturer = "BBCLIB";
	ol->part_number = "BBCLIB-SPI0";
	ol->version = "00A0";
	load_device_tree_overlay(ol);
	free(ol);

	laser->spi = malloc(sizeof(spi_properties));
	laser->spi->spi_id = spi0;
	laser->spi->bits_per_word = 8;
	laser->spi->mode = 0;
	laser->spi->speed = 10000000;
	laser->spi->flags = O_RDWR;

	uint8_t isOpen = spi_open(laser->spi);

	if (isOpen == 0) {
		spi_send(laser->spi, mcp23s08_setup, sizeof(mcp23s08_setup));
//		syslog(LOG_INFO, "%s", "Laser initialized.");
		return 1;
	}
//	syslog(LOG_INFO, "%s", "FAILED to initialize Laser.");
	return 1;
}

int set_MCP23S08_GPIO_values(spi_properties *spi, unsigned char value) {
	unsigned char mcp23s08_gpios[] = { 0x40, 0x09, 0x00, };
	mcp23s08_gpios[2] = value;
//	dprint(value);
	spi_send(spi, mcp23s08_gpios, sizeof(mcp23s08_gpios));
	return 1;
}

int send_MCP49x2_values(spi_properties *spi, unsigned char value) {
	int bit_position = 8;
	while(bit_position > 0) {
		unsigned char bit_to_send = (value & 0x80) >> 2;
//		dprint(bit_to_send);
		unsigned char clkdata = bit_to_send;
		set_MCP23S08_GPIO_values(spi, clkdata);
		clkdata = (MCP23S08_SCK | bit_to_send);
		set_MCP23S08_GPIO_values(spi, clkdata);
		bit_position--;
		value = value << 1;
	}
	return 1;
}

void set_MCP49x2(unsigned char data[2], unsigned char chip, spi_properties* spi) {
	set_MCP23S08_GPIO_values(spi, chip | MCP23S08_SCK | MCP23S08_SDI);
	set_MCP23S08_GPIO_values(spi, MCP23S08_SCK | MCP23S08_SDI);
	set_MCP23S08_GPIO_values(spi, MCP23S08_SDI);
	set_MCP23S08_GPIO_values(spi, MCP23S08_ALL_DOWN);
	send_MCP49x2_values(spi, data[0]);
	send_MCP49x2_values(spi, data[1]);
	set_MCP23S08_GPIO_values(spi, MCP23S08_SCK | MCP23S08_SDI);
	set_MCP23S08_GPIO_values(spi, chip | MCP23S08_SCK | MCP23S08_SDI);
}

int Laser_sendData(void *self, unsigned char value, int length, unsigned char  chip, unsigned char reg) {
	Laser *laser = self;
	unsigned char data[2] = {};
	data[0] = reg | ((value & 0xf0) >> 4);
	data[1] = (value & 0x0f) << 4;
	set_MCP49x2(data, chip, laser->spi);
//	syslog(LOG_INFO, "value = %i  chip=%i  register=0x%02x", value, chip, reg);
	return 0;
}

int Laser_setX(void *self, int x) {
	Laser *laser = self;
	int value = x;
//	syslog(LOG_INFO, "Laser setX value: %i.", value);
	laser->_(sendData)(self, value, 16, MCP4922_CS, 0x70);
	return 0;
}

int Laser_setY(void *self, int y) {
	Laser *laser = self;
	int value = y;
//	syslog(LOG_INFO, "Laser setY value: %i.", value);
	laser->_(sendData)(laser, value, 16, MCP4922_CS, 0xf0);
	return 0;
}

int Laser_setRed(void *self, int red) {
	Laser *laser = self;
//	syslog(LOG_INFO, "Laser setRed value: %i.", red);
	laser->_(sendData)(laser, red, 8, MCP4902_1_CS, 0x70);
	return 0;
}

int Laser_setGreen(void *self, int green) {
	Laser *laser = self;
//	syslog(LOG_INFO, "Laser setGreen value: %i.", green);
	laser->_(sendData)(laser, green, 8, MCP4902_1_CS, 0xf0);
	return 0;
}

int Laser_setBlue(void *self, int blue) {
	Laser *laser = self;
	laser->_(sendData)(laser, blue, 8, MCP4902_2_CS, 0x70);
//	syslog(LOG_INFO, "Laser setBlue value: %i.", blue);
	return 0;
}

void Laser_destroy(void *self) {
	Laser *laser = self;
	free(laser->spi);
	free(laser);
//	syslog(LOG_INFO, "%s", "Laser destroyed.");
}
