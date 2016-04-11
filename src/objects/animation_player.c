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
pthread_mutex_t run_lock = PTHREAD_MUTEX_INITIALIZER;

int Player_init(void *self) {
	Player *plyr = self;
	plyr->run = 0;
	plyr->currentAnimation = NULL;
	player = plyr;
	syslog(LOG_INFO,"Animationplayer: initialized.");
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

static void set_run_state(int state) {
	if (pthread_mutex_lock(&run_lock) != 0) {
		syslog(LOG_ERR,"Animationplayer: Can't get the lock on the run state.");
	}
	syslog(LOG_DEBUG,"Set run state: %i", state);
	player->run = state;
	if (pthread_mutex_unlock(&run_lock) != 0) {
		syslog(LOG_ERR,"Animationplayer: Can't unlock the run state.");
	}
}

static void* run_animation(void *ani) {
	Animation *animation = ani;
	int i, total_frames = animation->total_frames;
	int repeat = animation->repeat;
	player->currentAnimation = animation;
	syslog(LOG_DEBUG,"Animationplayer: starting play of animation %s %i number of times.", animation->name, animation->repeat);
	while(player->run == 1 && (repeat == -1 || repeat > 0)) {
		for(i=0;i<total_frames;i++) {
			frame_draw(animation->frames[i]);
		}
		if (repeat > 0)
			repeat--;
	}
	syslog(LOG_DEBUG,"Animationplayer: stopped animation %s, repeat was %i killed: %s", animation->name, animation->repeat, (animation->repeat == -1 || repeat > 0 ) ? "yes":"no");
	set_run_state(0);
	player->_(douse)(player);
	player->currentAnimation = NULL;
	free_animation(animation);
	return NULL;
}

static void popQueue() {
	if (player->queue->current->next == NULL) {
		free_queue_item(player->queue->current, 0);
		player->queue->current = NULL;
		player->queue->last = NULL;
	} else {
		QueueItem *next = player->queue->current->next;
		free_queue_item(player->queue->current, 0);
		player->queue->current = next;
	}
	syslog(LOG_DEBUG,"Animationplayer: queue popped. %s", getQueueListJson(player->queue));
}

static void spawnPlayer(Animation* animation) {
	void* result;
	if (pthread_join(runner, &result) != 0) {
		syslog(LOG_ERR,"Animationplayer: failed to wait for current player thread.");
	}
	syslog(LOG_DEBUG,"Animationplayer: spawning animation player.");
	if (pthread_create(&runner,NULL, run_animation, (void *)animation)) {
		syslog(LOG_ERR,"Animationplayer: Can't create animation player thread");
		perror("Can't create animation player thread");
	}
}

void Player_listen(void *self) {
	Player *player = self;
	for(;;) {
		if (pthread_mutex_lock(&player->queue->queue_lock) != 0) {
			syslog(LOG_ERR,"Animationplayer: Can't get the lock on the queue.");
		}
		if (player->queue->current == NULL) {
			syslog(LOG_DEBUG,"Waiting...");
			pthread_cond_wait(&player->queue->queue_not_empty, &player->queue->queue_lock);
		}

		syslog(LOG_DEBUG, "Working.....");
		Command *command = player->queue->current->command;

		if (command->action == play) {
			if (player->run == 1 && player->currentAnimation != NULL && player->currentAnimation->repeat == -1) {
				set_run_state(0);
				void* result;
				if (pthread_join(runner, &result) != 0) {
					syslog(LOG_ERR,"Animationplayer: failed to wait for current player thread.");
				}
			}
			if (player->run == 0) {
				set_run_state(1);
				spawnPlayer((Animation*) command->value);
				popQueue();
			}
		}
		if (command->action == stop || command->action == halt) {
			syslog(LOG_ERR,"Animationplayer: halting current animation %s.", player->currentAnimation->name);
			set_run_state(0);
			void* result;
			if (pthread_join(runner, &result) != 0) {
				syslog(LOG_ERR,"Animationplayer: failed to wait for current player thread.");
			}
			popQueue();
		}

		if (pthread_mutex_unlock(&player->queue->queue_lock) != 0) {
			syslog(LOG_ERR,"Animationplayer: Can't release the lock on the queue.");
		}
	}
}
