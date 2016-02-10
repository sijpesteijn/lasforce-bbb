/*
 * lasforce.c
 *
 *  Created on: Feb 8, 2016
 *      Author: gijs
 */

#include <stdio.h>
#include <fcntl.h>
#include <syslog.h>
#include <pthread.h>

#include "../include/objects/laser.h"
#include "../include/animation/animation.h"
#include "examples.h"
#include "readildafile.h"

Object LaserProto = {
    .init = Laser_init,
	.destroy = Laser_destroy,
	.setX = Laser_setX,
	.setY = Laser_setY,
	.setRed = Laser_setRed,
	.setBlue = Laser_setBlue,
	.setGreen = Laser_setGreen,
};

void* messageListener(void* param) {
	return NULL;
}

void* animationPlayer(void* param) {
	Laser *laser = NEW(Laser,"Beaglebone Black");

	char *data = readFile("/root/bbclib/jpoint.las");
	Animation *animation = animation_deserialize(data, sizeof(data));
//	drawSquare(laser, 1000);
//	drawLogo(laser, 100);

	laser->_(setRed)(laser, 0);
	laser->_(setGreen)(laser, 0);
	laser->_(setBlue)(laser, 0);
	laser->_(destroy)(laser);


	return NULL;
}

int main(int argc, char *argv[]) {
	openlog("lasforce-bbb", LOG_PID | LOG_CONS | LOG_NDELAY | LOG_NOWAIT, LOG_LOCAL0);
	syslog(LOG_INFO, "%s", "Starting LasForce...");

	pthread_t message_listener, animation_player;
	if (pthread_create(&message_listener,NULL, messageListener, NULL))
		perror("Can't create message_handler thread");
	if (pthread_create(&animation_player, NULL, animationPlayer, NULL))
		perror("Can't create ilda_player thread");
	void* result;

	pthread_join(message_listener, &result);
	pthread_join(animation_player, &result);

	syslog(LOG_INFO, "%s", "Stopping LasForce...");
	closelog();
	return 0;
}

