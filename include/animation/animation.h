/*
 * ILDA.h
 *
 *  Created on: Feb 23, 2015
 *      Author: gijs
 */

#ifndef INCLUDE_ILDA_H_
#define INCLUDE_ILDA_H_

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
	int totalSegments;
	int id;
} FrameMetadata;

typedef struct {
	Color* color;
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

void destroyAnimation(Animation *animation);

#endif /* INCLUDE_ILDA_H_ */
