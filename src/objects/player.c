/*
 * player.c
 *
 *  Created on: Feb 11, 2016
 *      Author: gijs
 */
#include "../../include/objects/animation_player.h"
#include <math.h>

int Player_init(void *self, void *object) {
	Player *player = self;
	player->laser = object;
	player->run = 0;
	return 1;
}

int printDistance(int x, int y, Laser *laser) {
	int x1 = abs(x - laser->x);
	int y1 = abs(y - laser->y);
	double c2 = sqrt(x1*x1 + y1*y1);
	return c2;
}

void segment_draw(Laser *laser, Segment *segment) {
	int i, distance, total_coordinates = segment->totalCoordinates;
	for(i=0;i<total_coordinates;i++) {
		int x = (segment->coordinates[i]->x + 32768)/laser->pixels_per_bit;
		int y = abs(-segment->coordinates[i]->y - 32767)/laser->pixels_per_bit;
		distance = printDistance(x, y, laser);
		laser->_(setCoordinate)(laser, x, y);
		if (i== 0) {
			laser->_(setRed)(laser, segment->color->red);
			laser->_(setGreen)(laser, segment->color->green);
			laser->_(setBlue)(laser, segment->color->blue);
		}
		usleep(distance/16);
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

int Player_playAnimation(void *self, Animation *animation) {
	Player *player = self;
	int i, total_frames = animation->animationMetadata->nrOfFrames;
	int repeat = animation->animationMetadata->repeat;
	while(repeat-- > 0 && player->run) {
		for(i=0;i<total_frames;i++) {
			frame_draw(player->laser, animation->frames[i]);
		}
	}

	return 1;
}
