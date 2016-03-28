/*
 * player.c
 *
 *  Created on: Feb 11, 2016
 *      Author: gijs
 */
#include "../../include/objects/animation_player.h"
#include "../../include/animation/animation.h"
#include "../readildafile.h"

#include <math.h>
#include <time.h>

#define BILLION 1000000000L
static pthread_t runner;
static Player *player;

int Player_init(void *self) {
	Player *plyr = self;
	plyr->run = 0;
	player = plyr;
	return 1;
}

void Player_douse(void *self) {
	Player *player = self;
	player->laser->_(setRed)(player->laser, 0);
	player->laser->_(setGreen)(player->laser, 0);
	player->laser->_(setBlue)(player->laser, 0);
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

int is_stop_signal(QueueItem *queueItem) {
	if (queueItem != NULL) {
		 const char *action = queueItem->command->action;
		 printf("NEXT: %s\n", action);
		 if (strcmp(action, "halt") == 0 || strcmp(action,"stop") == 0)
			 return 1;
	}
	return 0;
}

static void* run_animation(void *ani) {
	Animation *animation = ani;
	int i, total_frames = animation->nrOfFrames;
	int repeat = animation->repeat;
	player->run = 1;
	while(player->run && (repeat == -1 || repeat > 0)) {
		for(i=0;i<total_frames;i++) {
			frame_draw(animation->frames[i]);
		}
		if (repeat > 0)
			repeat--;
	}
	player->run = 0;
	player->_(douse)(player);

	return NULL;
}

void handleCommand(Command *command) {
	printf("Action: %s\n", command->action);
	if (strcmp(command->action, "halt") == 0) {
		player->run = 0;
	} else if (strcmp(command->action, "play") == 0) {
		char* data;
		char file[255];
		strcpy(file, "/root/bbclib/examples/");
		strcat(file, command->value);
		int length = readFile(file, &data);
		Animation *animation = animation_deserialize(data, length);
		if (pthread_create(&runner,NULL, run_animation, (void *)animation))
			perror("Can't create message_handler thread");

	} else if (strcmp(command->action, "stop") == 0) {
		player->run = 0;
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
				free_queue_item(player->queue->current);
				player->queue->current = next;
			} else {
				free_queue_item(player->queue->current);
				player->queue->current = NULL;
			}
		} else {
			printf("Oops nothing on the queue.\n");
		}
//		pthread_mutex_unlock(&player->queue->read_queue_lock);
	}
}
