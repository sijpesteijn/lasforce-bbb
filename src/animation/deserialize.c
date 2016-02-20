/*
 * deserialize.c
 *
 *  Created on: Feb 10, 2016
 *      Author: gijs
 */

#include "../../include/animation/animation.h"
#include "../../include/jansson/jansson.h"
#include <string.h>
#include <syslog.h>

int getInt(json_t* root, char* key) {
	json_t *value = json_object_get(root, key);
	if (value == NULL) {
		syslog(LOG_DEBUG, "Can't find the key: %s in json: %s", key, json_dumps(root, 0));
		return 0;
	} else {
		int val = json_number_value(value);
//		syslog(LOG_DEBUG, "getInt: %i\n", val);
		json_decref(value);
		return val;
	}
}

char* getString(json_t* root, char* key) {
	json_t *value = json_object_get(root, key);
	if (value == NULL) {
		syslog(LOG_DEBUG, "Can't find the key: %s in json: %s", key, json_dumps(root, 0));
		return "";
	} else {
		char* val = strdup(json_string_value(value));
//		syslog(LOG_DEBUG, "getString: %s\n", val);
		json_decref(value);
		return val;
	}
}

AnimationMetadata *deserializeAnimationMetadata(json_t *animationMetadataJson) {
	AnimationMetadata *animationMetadata = malloc(sizeof(AnimationMetadata));
	animationMetadata->id = getInt(animationMetadataJson, "id");
	animationMetadata->repeat = getInt(animationMetadataJson, "repeat");
	animationMetadata->maxHeight = getInt(animationMetadataJson, "max_height");
	animationMetadata->minHeight = getInt(animationMetadataJson, "min_height");
	animationMetadata->maxWidth = getInt(animationMetadataJson, "max_width");
	animationMetadata->minWidth = getInt(animationMetadataJson, "min_width");
	animationMetadata->nrOfFrames = getInt(animationMetadataJson, "total_frames");
	return animationMetadata;
}

FrameMetadata *deserializeFrameMetadata(json_t *frameMetadataJson) {
	FrameMetadata *frameMetadata = malloc(sizeof(FrameMetadata));
	frameMetadata->id = getInt(frameMetadataJson, "id");
	frameMetadata->repeat = getInt(frameMetadataJson, "repeat");
	frameMetadata->totalSegments = getInt(frameMetadataJson, "total_segments");
	return frameMetadata;
}

Color *deserializeColor(json_t *colorJson) {
	Color *color = malloc(sizeof(Color));
	color->red = json_number_value(json_array_get(colorJson, 0));
	color->green = json_number_value(json_array_get(colorJson, 1));
	color->blue = json_number_value(json_array_get(colorJson, 2));
	return color;
}

Coordinate *deserializeCoordinate(json_t *coordinateJson) {
	Coordinate *coordinate = malloc(sizeof(Coordinate));
	coordinate->x =json_number_value(json_array_get(coordinateJson, 0));
	coordinate->y =json_number_value(json_array_get(coordinateJson, 1));
	return coordinate;
}

Segment *deserializeSegment(json_t *segmentJson) {
	Segment *segment = malloc(sizeof(Segment));

	json_t *colorJson = json_object_get(segmentJson, "color");
	if (colorJson != NULL) {
		segment->color = deserializeColor(colorJson);
	}

	json_t *coordinatesJson = json_object_get(segmentJson, "coordinates");
	if (coordinatesJson != NULL) {
		int totalCoordinates = json_array_size(coordinatesJson);
		Coordinate **coordinates = malloc(totalCoordinates * sizeof(Coordinate));
		int i = 0;
		for(i=0;i<totalCoordinates;i++) {
			json_t *coordinateJson = json_array_get(coordinatesJson, i);
			Coordinate *coordinate = deserializeCoordinate(coordinateJson);
			coordinates[i] = coordinate;
		}
		segment->coordinates = coordinates;
		segment->totalCoordinates = totalCoordinates;
	}
	return segment;
}

Frame *deserializeFrame(json_t* frameJson) {
	Frame *frame = malloc(sizeof(Frame));

	json_t *frameMetadataJson = json_object_get(frameJson, "frame_metadata");
	if (frameMetadataJson != NULL)
		frame->frameMetadata = deserializeFrameMetadata(frameMetadataJson);

	json_t *segmentsJson = json_object_get(frameJson, "segments");
	if (segmentsJson != NULL) {
		int totalSegments = json_array_size(segmentsJson);
		Segment **segments = malloc(totalSegments * sizeof(Segment));
		int i = 0;
		for(i = 0; i < totalSegments; i++) {
			json_t *segmentJson = json_array_get(segmentsJson, i);
			Segment *segment = deserializeSegment(segmentJson);
			segments[i] = segment;
		}
		frame->segments = segments;
	}
	return frame;
}

static Animation *deserialize(json_t* root) {
	AnimationMetadata *aniationMetadata = NULL;
	Frame **frames = NULL;
	int totalFrames = 0;

	json_t *animationMetadataJson = json_object_get(root, "animation_metadata");
	if(animationMetadataJson != NULL) {
		aniationMetadata = deserializeAnimationMetadata(animationMetadataJson);
	}

	json_t *framesJson = json_object_get(root, "frames");
	if(framesJson != NULL) {
		totalFrames = json_array_size(framesJson);
		frames = malloc(totalFrames * sizeof(Frame*));
		int i = 0;
		for(i=0;i<totalFrames;i++) {
			json_t *frameJson = json_array_get(framesJson, i);
			Frame *frame = deserializeFrame(frameJson);
			frames[i] = frame;
		}
	}

	Animation *animation = malloc(sizeof(Animation));
	animation->animationMetadata = aniationMetadata;
	animation->frames = frames;
	return animation;
}

Animation* animation_deserialize(char* smsg, int smsgl) {
	json_t* root;
	json_error_t error;

	root = json_loadb(smsg, smsgl, 0, &error);
	if (!root) {
		fprintf(stderr, "error: on line %d: %s\n", error.line, error.text);
	}
	if (!json_is_object(root)) {
		fprintf(stderr, "error: commit data is not an object\n");
		json_decref(root);
	}
	Animation *animation = deserialize(root);
	return animation;
}
