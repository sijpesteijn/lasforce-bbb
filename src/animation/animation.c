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

int free_queue_item(QueueItem *queueItem) {
//	free(queueItem->command);
	free(queueItem);
	return 0;
}
