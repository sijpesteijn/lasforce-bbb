/*
 * animation.c
 *
 *  Created on: Feb 11, 2016
 *      Author: gijs
 */

#include "../../include/animation/animation.h"
#include <stdio.h>
#include <stdlib.h>

int destroy_animation(Animation *animation) {
	int i=0;
	for(i=0;i<animation->nrOfFrames;i++) {
		Frame *frame = animation->frames[i];
		int j = 0;
		for(j=0;j<frame->totalSegments;j++) {
			Segment *segment = frame->segments[j];
			free(segment->color);
			int c = 0;
			for(c=0;c<sizeof(segment->coordinates);c++) {
				free(segment->coordinates[c]);
			}
			free(segment);
		}
		free(frame);
	}
	free(animation->frames);
	free(animation);

	return 0;
}

int free_command(Command *command) {
	if (command->value != NULL) {
		free(command->value);
	}
	free(command);
	return 0;
}

int free_queue_item(QueueItem *queueItem, int inclusive_next) {
	if (queueItem != NULL) {
		free_command(queueItem->command);
		if (inclusive_next) {
			free_queue_item(queueItem->next, inclusive_next);
		}
		free(queueItem);
	}
	return 0;
}
