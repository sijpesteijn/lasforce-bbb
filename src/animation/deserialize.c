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
	if (json_is_string(value)) {
		char* val = strdup(json_string_value(value));
//		syslog(LOG_DEBUG, "getString: %s\n", val);
		json_decref(value);
		return val;
	} else {
		syslog(LOG_DEBUG, "Can't find the key: %s in json: %s", key, json_dumps(root, 0));
		return "";
	}
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
	frame->totalSegments = getInt(frameJson, "total_segments");

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

static Animation *deserializeAnimation(json_t* root) {
	Frame **frames = NULL;
	int totalFrames = 0;

	Animation *animation = malloc(sizeof(Animation));
	animation->id = getInt(root, "id");
	animation->repeat = getInt(root, "repeat");
	animation->nrOfFrames = getInt(root, "total_frames");

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
	Animation *animation = deserializeAnimation(root);
	return animation;
}

static Command *deserializeCommand(json_t* root) {
	Command *command = malloc(sizeof(Command));
	command->action = getString(root, "cmd");
	command->value = getString(root, "value");
	return command;
}

Command* command_deserialize(char* smsg, int smsgl) {
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
	Command *command = deserializeCommand(root);
	return command;
}

