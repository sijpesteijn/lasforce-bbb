/*
 * ILDA.h
 *
 *  Created on: Feb 23, 2015
 *      Author: gijs
 */

#ifndef INCLUDE_ILDA_H_
#define INCLUDE_ILDA_H_

#include <pthread.h>

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
	int nrOfFrames;
	Frame **frames;
} Animation;

typedef struct {
	const char *action;
	const char *value;
} Command;

typedef struct QueueItem {
	Command *command;
	struct QueueItem *next;
} QueueItem;

typedef struct Queue {
	pthread_mutex_t read_queue_lock;
	pthread_cond_t queue_not_empty;
	struct QueueItem *current;
	struct QueueItem *last;
} Queue;

int destroy_animation(Animation *animation);
int free_queue_item(QueueItem *queueItem);

#endif /* INCLUDE_ILDA_H_ */
