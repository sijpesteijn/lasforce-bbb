/*
 * object.c
 *
 *  Created on: Jan 29, 2016
 *      Author: gijs
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../include/objects/object.h"
#include "../../include/io/core.h"
#include <assert.h>

void Object_destroy(void *self)
{
    Object *obj = self;

    if(obj) {
        if(obj->description) free(obj->description);
        free(obj);
    }
}

void Object_describe(void *self)
{
    Object *obj = self;
    syslog(LOG_INFO, "%s.", obj->description);
}

int Object_init(void *self)
{
    // do nothing really
    return 1;
}

int Object_setCoordinate(void *self, int x, int y)
{
	syslog(LOG_INFO, "%s","You can't change coordinate.");
    return 0;
}

int Object_setRed(void *self, int red)
{
	syslog(LOG_INFO, "%s","You can't change red.");
    return 0;
}

int Object_setBlue(void *self, int blue)
{
	syslog(LOG_INFO, "%s","You can't change blue.");
    return 0;
}

int Object_setGreen(void *self, int green)
{
	syslog(LOG_INFO, "%s","You can't change green.");
    return 0;
}

int Object_sendData(void *self, unsigned char value, int length, unsigned char chip, unsigned char reg)
{
	syslog(LOG_INFO, "%s","You can't send data.");
	return 0;
}

void Object_listen(void *self)
{
	syslog(LOG_INFO, "%s","You can't listen on the socket.");
}

int Object_playAnimation(void *self, void *animation)
{
	syslog(LOG_INFO, "%s","You can't play an animation.");
    return 0;
}

void *Object_new(size_t size, Object proto,  char *description)
{
	syslog(LOG_INFO, "%s - %s","Creating new object.", description);
    // setup the default functions in case they aren't set
    if(!proto.init) proto.init = Object_init;
    if(!proto.describe) proto.describe = Object_describe;
    if(!proto.destroy) proto.destroy = Object_destroy;
    // this seems weird, but we can make a struct of one size,
    // then point a different pointer at it to "cast" it
    Object *el = calloc(1, size);
    *el = proto;

    // copy the description over
    el->description = strdup(description);

    // initialize it with whatever init we were given
    if(!el->init(el)) {
    	syslog(LOG_INFO, "%s","Failed to create object.");
        // looks like it didn't initialize properly
        el->destroy(el);
        return NULL;
    } else {
        // all done, we made an object of any type
        return el;
    }
}
