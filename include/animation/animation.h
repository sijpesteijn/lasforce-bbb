/*
 * ILDA.h
 *
 *  Created on: Feb 23, 2015
 *      Author: gijs
 */

#ifndef INCLUDE_ILDA_H_
#define INCLUDE_ILDA_H_

#include "../objects/laser.h"

typedef struct {
	int id;
	int repeat;
	int maxWidth;
	int maxHeight;
	int minWidth;
	int minHeight;
	int nrOfFrames;
} AnimationMetadata;

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
	int id;
	int totalSegments;
	int repeat;
} FrameMetadata;

typedef struct {
	Color* color;
	int totalCoordinates;
	Coordinate **coordinates;
} Segment;

typedef struct {
	FrameMetadata *frameMetadata;
	Segment **segments;
} Frame;

typedef struct {
	AnimationMetadata *animationMetadata;
	Frame **frames;
} Animation;

int destroy_animation(Animation *animation);
void animation_draw(Laser *laser, Animation *animation);

#endif /* INCLUDE_ILDA_H_ */
