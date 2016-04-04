/*
 * player.c
 *
 *  Created on: Feb 11, 2016
 *      Author: gijs
 */
#include "../../include/objects/animation_player.h"
#include "../../include/animation/animation.h"
#include "../../include/animation/deserialize.h"
#include "../../include/global.h"
#include "../readildafile.h"

#include <math.h>
#include <time.h>

#define BILLION 1000000000L
static pthread_t runner;
static Player *player;
int id = 0;

int Player_init(void *self) {
	Player *plyr = self;
	plyr->run = 0;
	plyr->currentAnimation = NULL;
	player = plyr;
	syslog(LOG_INFO,"Player: initialized.");
	return 1;
}

void Player_douse(void *self) {
	Player *player = self;
	player->laser->_(setRed)(player->laser, 0);
	player->laser->_(setGreen)(player->laser, 0);
	player->laser->_(setBlue)(player->laser, 0);
	player->laser->_(setCoordinate)(player->laser, 0, 0);
}

static int calculateDistance(int x, int y) {
	int x1 = abs(x - player->laser->x);
	int y1 = abs(y - player->laser->y);
	double c2 = sqrt(x1*x1 + y1*y1);
	return c2;
}

static void segment_draw(Segment *segment) {
	int i, distance, total_coordinates = segment->totalCoordinates;
	for(i=0;i<total_coordinates;i++) {
		int x = (segment->coordinates[i]->x + 22768)/player->laser->pixels_per_bit;
		int y = abs(-segment->coordinates[i]->y - 20767)/player->laser->pixels_per_bit;
		distance = calculateDistance(x, y);
		player->laser->_(setCoordinate)(player->laser, x, y);
		if (i== 0) {
			player->laser->_(setRed)(player->laser, segment->color->red);
			player->laser->_(setGreen)(player->laser, segment->color->green);
			player->laser->_(setBlue)(player->laser, segment->color->blue);
		}
		usleep(distance/2);
	}
	player->laser->_(setRed)(player->laser, 0);
	player->laser->_(setGreen)(player->laser, 0);
	player->laser->_(setBlue)(player->laser, 0);
}

static void frame_draw(Frame *frame) {
	int i, total_segments = frame->totalSegments;
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);
	uint64_t diff;
	do {
		for(i=0;i<total_segments;i++) {
			segment_draw(frame->segments[i]);
		}
		clock_gettime(CLOCK_MONOTONIC, &end);
		diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;

//		printf("Elapsed: %llu nanoseconds\n", diff);
	} while(diff < player->currentAnimation->frame_time * 1000);
}

static void* run_animation(void *ani) {
	Animation *animation = ani;
	int i, total_frames = animation->total_frames;
	int repeat = animation->repeat;
	player->currentAnimation = animation;
	player->run = 1;
	syslog(LOG_DEBUG,"Player: Start play of animation %s %i number of times.", animation->name, animation->repeat);
	while(player->run && (repeat == -1 || repeat > 0)) {
		for(i=0;i<total_frames;i++) {
			frame_draw(animation->frames[i]);
		}
		if (repeat > 0)
			repeat--;
	}
	syslog(LOG_DEBUG,"Player: stopped animation %s, repeat was %i", animation->name, animation->repeat);
	player->run = 0;
	player->_(douse)(player);
	player->currentAnimation = NULL;
	free_animation(animation);
	pthread_cond_signal(&player->queue->player_finished);
	return NULL;
}

static void handleCommand(Command *command) {
	switch(command->action) {
		case stop:
		case halt:
			player->run = 0;
			break;
		case play:
		{
			if (player->run) {
				syslog(LOG_DEBUG,"Player: there is an animation playing.");
				if(player->currentAnimation->repeat == -1) {
					syslog(LOG_DEBUG,"Player: force player to stop current animation.");
					player->run = 0;
				}

				pthread_cond_wait(&player->queue->player_finished, &player->queue->queue_lock);
				void* result;
				syslog(LOG_DEBUG,"Player: waiting for animation to finish. %i", id++);
				if (pthread_join(runner, &result) != 0) {
					syslog(LOG_ERR,"Player: failed to wait for current player thread.");
				}
				syslog(LOG_DEBUG,"Player: Animation finished playing.");
			}
			syslog(LOG_DEBUG,"Player: preparing animation player.");
			Animation *animation = (Animation*)command->value;
			if (pthread_create(&runner,NULL, run_animation, (void *)animation)) {
				syslog(LOG_ERR,"Player: Can't create message_handler thread");
				perror("Can't create message_handler thread");
			}
			break;
		}
		case list:
			syslog(LOG_ERR,"Player: can not handle list requests.");
			break;
	}
	free(command);
}

void Player_listen(void *self) {
	Player *player = self;
	for(;;) {
		if (pthread_mutex_lock(&player->queue->queue_lock) != 0) {
			syslog(LOG_ERR,"Player: Can't get the lock on the queue.");
		}

		if (player->queue->current == NULL) {
			pthread_cond_wait(&player->queue->queue_not_empty, &player->queue->queue_lock);
		}
		syslog(LOG_DEBUG,"Player: queue is not empty.");
		QueueItem *current = player->queue->current;
		handleCommand(current->command);

		if (current->next == NULL) {
			free_queue_item(current, 0);
			player->queue->current = NULL;
			player->queue->last = NULL;
		} else {
			QueueItem *next = current->next;
			free_queue_item(current, 0);
			player->queue->current = next;
		}
		syslog(LOG_DEBUG,"Player: queue popped. %s", getQueueListJson(player->queue));
		if (pthread_mutex_unlock(&player->queue->queue_lock) != 0) {
			syslog(LOG_ERR,"Player: Can't release the lock on the queue.");
		}
	}
}
