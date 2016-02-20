/*
 * animation.c
 *
 *  Created on: Feb 11, 2016
 *      Author: gijs
 */

#include "../../include/animation/animation.h"
#include "../../include/objects/laser.h"
#include <math.h>
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

int one = 0,two=0;

int printDistance(int x, int y, Laser *laser) {
	int x1 = abs(x - laser->x);
	int y1 = abs(y - laser->y);
	double c2 = sqrt(x1*x1 + y1*y1);
	return c2;
}

void segment_draw(Laser *laser, Segment *segment) {
	int i, total_coordinates = segment->totalCoordinates;
	for(i=0;i<total_coordinates;i++) {
		int x = (segment->coordinates[i]->x*10);
		int y = (segment->coordinates[i]->y*10);
		int distance = printDistance(x, y, laser);
		laser->_(setCoordinate)(laser, x, y);
		if (i== 0) {
			laser->_(setRed)(laser, segment->color->red);
			laser->_(setGreen)(laser, segment->color->green);
			laser->_(setBlue)(laser, segment->color->blue);
		}
		usleep(distance + 10);
	}
	laser->_(setRed)(laser, 0);
	laser->_(setGreen)(laser, 0);
	laser->_(setBlue)(laser, 0);
}

void frame_draw(Laser *laser, Frame *frame) {
	int i, total_segments = frame->frameMetadata->totalSegments;
	int repeat = frame->frameMetadata->repeat;
	while(repeat-- > 0) {
		for(i=0;i<total_segments;i++) {
			segment_draw(laser, frame->segments[i]);
		}
	}
}

void animation_draw(Laser *laser, Animation *animation) {
	int i, total_frames = animation->animationMetadata->nrOfFrames;
	int repeat = animation->animationMetadata->repeat;
//	while(repeat-- > 0) {
	for(;;) {
		for(i=0;i<total_frames;i++) {
			frame_draw(laser, animation->frames[i]);
		}
	}
}
