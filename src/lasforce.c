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
#include "../include/animation/deserialize.h"
#include "../include/objects/animation_player.h"
#include "../include/objects/socket_handler.h"
#include "examples.h"
#include "readildafile.h"

Object LaserProto = {
    .init = Laser_init,
	.destroy = Laser_destroy,
	.setCoordinate = Laser_setCoordinate,
	.setRed = Laser_setRed,
	.setBlue = Laser_setBlue,
	.setGreen = Laser_setGreen,
};

Object SocketHandlerProto = {
	.init = SocketHandler_init,
	.describe = SocketHandler_describe,
	.listen = SocketHandler_listen,
	.destroy = SocketHandler_destroy
};

Object PlayerProto = {
	.init = Player_init,
	.playAnimation = Player_playAnimation,
};

void* messageListener(void* param) {
	SocketHandler *handler = NEW(SocketHandler, "Socket handler");
	handler->_(listen)(handler);
	return NULL;
}

char *ilda = "peace.las";

void* animationPlayer(void* param) {
	char* data;
	char file[255];
	strcpy(file, "/root/bbclib/examples/");
	strcat(file, ilda);
	printf("%s", file);
	int i=0, length = readFile(file, &data);
	Laser *laser = NEW(Laser,"Beaglebone Black");
	Player *player = NEWPLAYER(Player, laser, "Animation Player");
	Animation *animation = animation_deserialize(data, length);
	int repeat = animation->animationMetadata->repeat;
	while(i++ < repeat) {
		player->run = 1;
		player->_(playAnimation)(player,animation);
	}

	laser->_(setRed)(laser, 0);
	laser->_(setGreen)(laser, 0);
	laser->_(setBlue)(laser, 0);
	laser->_(destroy)(laser);
	destroy_animation(animation);

	return NULL;
}

int main(int argc, char *argv[]) {
	openlog("lasforce-bbb", LOG_PID | LOG_CONS | LOG_NDELAY | LOG_NOWAIT, LOG_LOCAL0);
	syslog(LOG_INFO, "%s", "Starting LasForce...");

	if (argc > 1) {
		ilda = argv[1];
		strcat(ilda, ".las");
	}
	pthread_t message_listener, animation_player;
//	if (pthread_create(&message_listener,NULL, messageListener, NULL))
//		perror("Can't create message_handler thread");
	if (pthread_create(&animation_player, NULL, animationPlayer, NULL))
		perror("Can't create ilda_player thread");
	void* result;

//	pthread_join(message_listener, &result);
	pthread_join(animation_player, &result);

	syslog(LOG_INFO, "%s", "Stopping LasForce...");
	closelog();
	return 0;
}

