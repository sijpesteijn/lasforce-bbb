/*
 * player.c
 *
 *  Created on: Feb 11, 2016
 *      Author: gijs
 */
#include "../../include/objects/animation_player.h"
#include "../../include/animation/animation.h"
#include "../../include/animation/deserialize.h"
#include "../readildafile.h"

#include <math.h>
#include <time.h>

#define BILLION 1000000000L
static pthread_t runner;
static Player *player;

int Player_init(void *self) {
	Player *plyr = self;
	plyr->run = 0;
	plyr->repeat_animation = 0;
	player = plyr;
	return 1;
}

void Player_douse(void *self) {
	Player *player = self;
	player->laser->_(setRed)(player->laser, 0);
	player->laser->_(setGreen)(player->laser, 0);
	player->laser->_(setBlue)(player->laser, 0);
	player->laser->_(setCoordinate)(player->laser, 0, 0);
}

int calculateDistance(int x, int y) {
	int x1 = abs(x - player->laser->x);
	int y1 = abs(y - player->laser->y);
	double c2 = sqrt(x1*x1 + y1*y1);
	return c2;
}

void segment_draw(Segment *segment) {
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

void frame_draw(Frame *frame) {
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
	} while(diff < player->frame_time * 1000);
}

static void* run_animation(void *ani) {
	Animation *animation = ani;
	int i, total_frames = animation->nrOfFrames;
	int repeat = player->repeat_animation;
	player->run = 1;
	while(player->run && (repeat == -1 || repeat > 0)) {
		for(i=0;i<total_frames;i++) {
			frame_draw(animation->frames[i]);
		}
		if (repeat > 0)
			repeat--;
	}
	printf("Finished with: %i\n", animation->nrOfFrames);
	player->run = 0;
	player->_(douse)(player);

	return NULL;
}

void handleCommand(Command *command) {
	switch(command->action) {
		case stop:
		case halt:
			player->run = 0;
			break;
		case play:
		{
			if (player->run) {
				if (player->repeat_animation == -1) // force stop if infinete animation
					player->run = 0;
				void* result;
				printf("Waiting...\n");
				pthread_join(runner, &result); // wait for the animation to finish.
			}
			char* data;
			char file[255];
			strcpy(file, "/root/bbclib/examples/");
			AnimationInfo *info = (AnimationInfo *)command->value;
			strcat(file, info->name);
			int length = readFile(file, &data);
			Animation *animation = animation_deserialize(data, length);
			player->repeat_animation = info->repeat;
			if (pthread_create(&runner,NULL, run_animation, (void *)animation))
				perror("Can't create message_handler thread");
			break;
		}
	}
}

void Player_listen(void *self) {
	Player *player = self;
	player->frame_time = 1000;
	for(;;) {
		if (player->queue->current == NULL) {
			pthread_cond_wait(&player->queue->queue_not_empty, &player->queue->read_queue_lock);
		}
//		pthread_mutex_lock(&player->queue->read_queue_lock);
		if (player->queue->current != NULL) {
			printf("Start thread if no other animation is playing.\n");
			handleCommand(player->queue->current->command);
			if (player->queue->current->next != NULL) {
				QueueItem *next = player->queue->current->next;
				free_queue_item(player->queue->current, 0);
				player->queue->current = next;
			} else {
				free_queue_item(player->queue->current, 0);
				player->queue->current = NULL;
			}
		} else {
			printf("Oops nothing on the queue.\n");
		}
//		pthread_mutex_unlock(&player->queue->read_queue_lock);
	}
}
