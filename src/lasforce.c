/*
 * lasforce.c
 *
 *  Created on: Feb 8, 2016
 *      Author: gijs
 */

#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>

#include "../include/objects/object.h"
#include "../include/objects/socket_handler.h"
#include "../include/objects/laser.h"
#include "../include/objects/animation_player.h"
#include "../include/animation/animation.h"
#include "../include/jansson/jansson.h"

// Create a lock and thread condition
pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t queue_not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t player_finished = PTHREAD_COND_INITIALIZER;

Object SocketHandlerProto = { .init = SocketHandler_init, .listen =
		SocketHandler_listen };

Object LaserProto = { .init = Laser_init, .setCoordinate = Laser_setCoordinate,
		.setRed = Laser_setRed, .setBlue = Laser_setBlue, .setGreen =
				Laser_setGreen, };

Object PlayerProto = { .init = Player_init, .listen = Player_listen, .douse =
		Player_douse };

void* messageListener(void* param) {
	SocketHandler *handler = NEW(SocketHandler, "Socket handler");
	if (handler == NULL) {
		syslog(LOG_ERR, "Lasforce: Can't create socket handler object.");
		exit(1);
	}
	handler->queue = (Queue *) param;
	handler->_(listen)(handler);
	pthread_exit(NULL);
	return NULL;
}

void* animationPlayer(void* param) {
	Player *player = NEW(Player, "Animation Player");
	if (player == NULL) {
		syslog(LOG_ERR, "Lasforce: Can't create animation player object.");
		exit(1);
	}
	player->laser = NEW(Laser, "Laser");
	if (player->laser == NULL) {
		syslog(LOG_ERR, "Lasforce: Can't create laser object.");
		exit(1);
	}

	player->queue = (Queue *) param;
	player->_(listen)(player);
	pthread_exit(NULL);
	return NULL;
}

int main(int argc, char *argv[]) {
	openlog("lasforce-bbb", LOG_PID | LOG_CONS | LOG_NDELAY | LOG_NOWAIT,
			LOG_LOCAL0);
	setlogmask(LOG_UPTO(LOG_DEBUG));

	syslog(LOG_INFO, "%s", "Starting LasForce...");

	Queue *queue = malloc(sizeof(Queue));
	if (queue == NULL) {
		syslog(LOG_ERR, "Lasforce: Can't allocate memory for queue.");
		exit(1);
	}
	queue->current = NULL;
	queue->last = NULL;
	queue->queue_lock = queue_lock;
	queue->queue_not_empty = queue_not_empty;
	queue->player_finished = player_finished;

	pthread_t message_listener, animation_player;
	if (pthread_create(&message_listener, NULL, messageListener,
			(void *) queue))
		perror("Can't create message_handler thread");
	if (pthread_create(&animation_player, NULL, animationPlayer,
			(void *) queue))
		perror("Can't create ilda_player thread");
	void* result;

	pthread_join(message_listener, &result);
	pthread_join(animation_player, &result);
	pthread_mutex_destroy(&queue_lock);

	syslog(LOG_INFO, "%s", "Stopping LasForce...");
	free_queue_item(queue->current, 1);
	free(queue);
	closelog();
	pthread_exit(NULL);
	return 0;
}

