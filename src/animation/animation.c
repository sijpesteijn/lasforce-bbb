/*
 * animation.c
 *
 *  Created on: Feb 11, 2016
 *      Author: gijs
 */

#include "../../include/animation/animation.h"
#include <stdio.h>

int destroy_animation(Animation *animation) {
	int i=0;
	for(i=0;i<animation->animationMetadata->nrOfFrames;i++) {
		Frame *frame = animation->frames[i];
		int j = 0;
		for(j=0;j<frame->frameMetadata->totalSegments;j++) {
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
	free(animation->animationMetadata);

	return 0;
}
