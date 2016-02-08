/*
 * object.h
 *
 *  Created on: Jan 29, 2016
 *      Author: gijs
 */

#ifndef EXAMPLES_OBJECTS_OBJECT_H_
#define EXAMPLES_OBJECTS_OBJECT_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    char *description;
    int (*init)(void *self);
    void (*describe)(void *self);
    void (*destroy)(void *self);
    int (*setX)(void *self, int x);
    int (*setY)(void *self, int y);
    int (*setRed)(void *self, int red);
    int (*setBlue)(void *self, int blue);
    int (*setGreen)(void *self, int green);
    int (*sendData)(void *self, unsigned char value, int length, unsigned char chip, unsigned char reg);
} Object;

int Object_init(void *self);
void Object_describe(void *self);
void Object_destroy(void *self);
int Object_setX(void *self, int x);
int Object_setY(void *self, int y);
int Object_setRed(void *self, int red);
int Object_setBlue(void *self, int blue);
int Object_setGreen(void *self, int green);
int Object_sendData(void *self, unsigned char value, int length, unsigned char chip, unsigned char reg);
void *Object_new(size_t size, Object proto, char *description);

#define NEW(T, N) Object_new(sizeof(T), T##Proto, N)
#define _(N) proto.N



#endif /* EXAMPLES_OBJECTS_OBJECT_H_ */
