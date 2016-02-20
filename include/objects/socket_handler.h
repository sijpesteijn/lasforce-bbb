/*
 * socket_handler.h
 *
 *  Created on: Feb 18, 2016
 *      Author: gijs
 */

#ifndef OBJECTS_SOCKET_HANDLER_H_
#define OBJECTS_SOCKET_HANDLER_H_

#include "object.h"

typedef struct {
	char *content;
	unsigned long length;
} socket_message;


struct SocketHandler {
	Object proto;
	int socket;
};

typedef struct SocketHandler SocketHandler;

int SocketHandler_init(void *self);
void SocketHandler_describe(void *self);
void SocketHandler_listen(void *self);
void SocketHandler_destroy(void *self);


#endif /* OBJECTS_SOCKET_HANDLER_H_ */
