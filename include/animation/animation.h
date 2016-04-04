/*
 * ILDA.h
 *
 *  Created on: Feb 23, 2015
 *      Author: gijs
 */

#ifndef INCLUDE_ILDA_H_
#define INCLUDE_ILDA_H_

#include <pthread.h>

typedef enum {
	play,
	stop,
	halt,
	list
} CMD;

typedef enum {
	queue
} INFO;

typedef struct {
	int red;
	int green;
	int blue;
} Color;

typedef struct {
	int x;
	int y;
} Coordinate;

typedef struct {
	Color* color;
	int totalCoordinates;
	Coordinate **coordinates;
} Segment;

typedef struct {
	int totalSegments;
	Segment **segments;
} Frame;

typedef struct {
	int id;
	int repeat;
	int frame_time;
	const char *name;
	int total_frames;
	Frame **frames;
} Animation;

typedef struct {
	CMD action;
	void *value;
} Command;

typedef struct QueueItem {
	Command *command;
	struct QueueItem *next;
} QueueItem;

typedef struct Queue {
	pthread_mutex_t queue_lock;
	pthread_cond_t queue_not_empty;
	pthread_cond_t player_finished;
	struct QueueItem *current;
	struct QueueItem *last;
} Queue;

typedef struct AnimationInfo {
	int repeat;
	const char *name;
} AnimationInfo;

int free_animation(Animation *animation);
int free_queue_item(QueueItem *queueItem, int inclusive_next);

#endif /* INCLUDE_ILDA_H_ */
