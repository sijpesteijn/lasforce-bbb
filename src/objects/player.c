/*
 * player.c
 *
 *  Created on: Feb 11, 2016
 *      Author: gijs
 */
#include "../../include/objects/animation_player.h"
#include "../../include/animation/animation.h"

#include <math.h>
#include <time.h>

#define BILLION 1000000000L

int Player_init(void *self) {
	Player *player = self;
	player->run = 0;
	return 1;
}

void Player_douse(void *self) {
	Player *player = self;
	player->laser->_(setRed)(player->laser, 0);
	player->laser->_(setGreen)(player->laser, 0);
	player->laser->_(setBlue)(player->laser, 0);
}

int calculateDistance(int x, int y, Laser *laser) {
	int x1 = abs(x - laser->x);
	int y1 = abs(y - laser->y);
	double c2 = sqrt(x1*x1 + y1*y1);
	return c2;
}

void segment_draw(Laser *laser, Segment *segment) {
	int i, distance, total_coordinates = segment->totalCoordinates;
	for(i=0;i<total_coordinates;i++) {
		int x = (segment->coordinates[i]->x + 22768)/laser->pixels_per_bit;
		int y = abs(-segment->coordinates[i]->y - 20767)/laser->pixels_per_bit;
		distance = calculateDistance(x, y, laser);
		laser->_(setCoordinate)(laser, x, y);
		if (i== 0) {
			laser->_(setRed)(laser, segment->color->red);
			laser->_(setGreen)(laser, segment->color->green);
			laser->_(setBlue)(laser, segment->color->blue);
		}
		usleep(distance/2);
	}
	laser->_(setRed)(laser, 0);
	laser->_(setGreen)(laser, 0);
	laser->_(setBlue)(laser, 0);
}

void frame_draw(Player *player, Frame *frame) {
	int i, total_segments = frame->totalSegments;
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);
	uint64_t diff;
	do {
		for(i=0;i<total_segments;i++) {
			segment_draw(player->laser, frame->segments[i]);
		}
		clock_gettime(CLOCK_MONOTONIC, &end);
		diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;

//		printf("Elapsed: %llu nanoseconds\n", diff);
	} while(diff < player->frame_time * 1000);
}

int Player_playAnimation(void *self, void *other) {
	Player *player = self;
	Animation *animation = other;
	int i, total_frames = animation->nrOfFrames;
	int repeat = animation->repeat;
	while(player->run == 1 && (repeat == -1 || repeat > 0)) {
		for(i=0;i<total_frames;i++) {
			frame_draw(player, animation->frames[i]);
		}
		if (repeat > 0)
			repeat--;
	}
	player->_(douse)(player);

	return 1;
}

void Player_listen(void *self) {
	Player *player = self;
	for(;;) {
		if (player->queue->current == NULL) {
			pthread_cond_wait(&player->queue->queue_not_empty, &player->queue->read_queue_lock);
		}
		printf("DOING STUFF.\n");

//		while(player->run == 1 && player->current_queue_element != NULL) {
//			Queue *current = player->current_queue_element;
//			printf("ELEMENT %i\n", &player->current_queue_element);
//			player->_(playAnimation)(player,animation);
//			destroy_animation(animation);

//			pthread_mutex_lock(&queue->read_queue_lock);
			if (player->queue->current != NULL) {
				printf("Command: %s\n", player->queue->current->command->action);
				if (player->queue->current->next != NULL) {
					printf("More to do!\n");
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
//			pthread_mutex_unlock(&queue->read_queue_lock);

//		}
		printf("DID STUFF.\n");
		sleep(3);
	}
}
