/*
 * lasforce.c
 *
 *  Created on: Feb 8, 2016
 *      Author: gijs
 */

#include <stdio.h>
#include "../include/objects/laser.h"

Object LaserProto = {
    .init = Laser_init,
	.destroy = Laser_destroy,
	.setX = Laser_setX,
	.setY = Laser_setY,
	.setRed = Laser_setRed,
	.setBlue = Laser_setBlue,
	.setGreen = Laser_setGreen,
    .sendData = Laser_sendData
};

int main(int argc, char *argv[]) {
	printf("Starting lasforce.\n");
	Laser *laser = NEW(Laser,"Beaglebone Black");
	return 0;
}

