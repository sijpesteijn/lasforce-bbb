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

#include "../include/objects/object.h"
#include "../include/objects/socket_handler.h"
#include "../include/objects/laser.h"
#include "../include/objects/animation_player.h"
#include "../include/animation/animation.h"

pthread_mutex_t read_queue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_not_empty=PTHREAD_COND_INITIALIZER;

Object SocketHandlerProto = {
	.init = SocketHandler_init,
	.listen = SocketHandler_listen
};

Object LaserProto = {
    .init = Laser_init,
	.destroy = Laser_destroy,
	.setCoordinate = Laser_setCoordinate,
	.setRed = Laser_setRed,
	.setBlue = Laser_setBlue,
	.setGreen = Laser_setGreen,
};

Object PlayerProto = {
	.init = Player_init,
	.listen = Player_listen,
	.douse = Player_douse
};

void* messageListener(void* param) {
	SocketHandler *handler = NEW(SocketHandler, "Socket handler");
	handler->queue = (Queue *)param;
	handler->_(listen)(handler);
	return NULL;
}

void* animationPlayer(void* param) {
	Player *player = NEW(Player, "Animation Player");
	player->laser = NEW(Laser,"Beaglebone Black");
	player->queue = (Queue *)param;
	player->_(listen)(player);
	return NULL;
}

int main(int argc, char *argv[]) {
	openlog("lasforce-bbb", LOG_PID | LOG_CONS | LOG_NDELAY | LOG_NOWAIT, LOG_LOCAL0);
	syslog(LOG_INFO, "%s", "Starting LasForce...");

	Queue *queue = malloc(sizeof(Queue));
	queue->current = NULL;
	queue->last = NULL;
	queue->read_queue_lock = read_queue_lock;
	queue->queue_not_empty= queue_not_empty;

	pthread_t message_listener, animation_player;
	if (pthread_create(&message_listener,NULL, messageListener, (void *)queue))
		perror("Can't create message_handler thread");
	if (pthread_create(&animation_player, NULL, animationPlayer, (void *)queue))
		perror("Can't create ilda_player thread");
	void* result;

	pthread_join(message_listener, &result);
	pthread_join(animation_player, &result);

	syslog(LOG_INFO, "%s", "Stopping LasForce...");
	closelog();
	return 0;
}

