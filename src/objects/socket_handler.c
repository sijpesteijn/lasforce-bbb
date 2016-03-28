/*
 * socket_handler.c
 *
 *  Created on: Feb 18, 2016
 *      Author: gijs
 */

#include "../../include/objects/socket_handler.h"
#include "../../include/animation/deserialize.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int openSocket() {
	int reuse = 1;
	int listener_d = socket(PF_INET, SOCK_STREAM, 0);
	if (listener_d == -1) {
		perror("Can't open socket");
		exit(1);
	}
	if (setsockopt(listener_d, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse,
			sizeof(int)) == -1) {
		perror("Can't set the reuse option on the socket");
		exit(1);
	}
	struct sockaddr_in name;
	name.sin_family = PF_INET;
	name.sin_port = (in_port_t) htons(5555);
	name.sin_addr.s_addr = htonl(INADDR_ANY);

	int connection = bind(listener_d, (struct sockaddr*) &name, sizeof(name));
	if (connection == -1) {
		perror("Can't bind to socket");
		exit(1);
	}

	if (listen(listener_d, 10) == -1) {
		perror("Can't listen");
		exit(1);
	}

	return listener_d;
}


int SocketHandler_init(void *self) {
	SocketHandler *handler = self;
	handler->socket = openSocket();
	return 1;
}

int getMessageLength(int connect_d) {
	int message_size_length = 13;
	char message_size[message_size_length];
	memset(message_size,0,message_size_length*sizeof(char));
	int n = read(connect_d, message_size, message_size_length - 1);
	if (n < 0) {
		perror("Can't reading size from socket");
		exit(1);
	}
	message_size[n + 1] = '\0';
	return atol(message_size);
}

socket_message* readSocketMessage(int connect_d) {
	unsigned long message_length = getMessageLength(connect_d);
	socket_message* smsg;
	if (message_length == 0) {
		 smsg = malloc(sizeof(socket_message));
		smsg->content = "";
		smsg->length = 0;
		return smsg;
	}
	char* message = malloc(message_length + 1);
	if (message == NULL) {
		perror("Can't allocate memory for message");
		exit(1);
	}
	bzero(message, message_length + 1);

	int socket_size = 4096; //32768;
	if (socket_size > message_length) {
		socket_size = message_length;
	}

	unsigned long bytes_read = 0;
	int n = 0;
	while (bytes_read < message_length) {
		char buffer[socket_size];
		n = read(connect_d, buffer, socket_size);
		if (n < 0) {
			perror("ERROR reading message from socket");
			exit(1);
		}
		buffer[n] = '\0';
		if (bytes_read == 0) {
			strcpy(message, buffer);
		} else {
			strcat(message, buffer);
		}
		bytes_read += n;
	}
	smsg = malloc(sizeof(socket_message));
	smsg->content = message;
	smsg->length = message_length;
	return smsg;
}

void freeSocketMessage(socket_message* socketMessage) {
	if (socketMessage != NULL) {
		free(socketMessage->content);
		free(socketMessage);
	}
}

void SocketHandler_listen(void *self) {
	SocketHandler *handler = self;
	struct sockaddr_storage client_addr;
	unsigned int address_size = sizeof(client_addr);
	for(;;) {
		int connect_d = accept(handler->socket, (struct sockaddr*) &client_addr, &address_size);
		if (connect_d == -1) {
			perror("Can't open secondary socket");
			exit(1);
		}
		socket_message* message = NULL;
		for(;;) {
			message = readSocketMessage(connect_d);
			if(message->length > 0) {
				printf("Msg: %s\n", message->content);
				Command *command = command_deserialize(message->content, message->length);
				QueueItem *queue_item = malloc(sizeof(struct QueueItem));
				queue_item->command = command;
				queue_item->next = NULL;

				pthread_mutex_lock(&handler->queue->read_queue_lock);

				if (command->action == halt) {
					QueueItem *current = handler->queue->current;
					free_queue_item(current, 1);
					handler->queue->current = queue_item;
					handler->queue->last = queue_item;
				} else {
					if (handler->queue->last == NULL) {
						handler->queue->last = queue_item;
					} else {
						handler->queue->last->next = queue_item;
						handler->queue->last = handler->queue->last->next;
					}

					if (handler->queue->current == NULL) {
						printf("NEW QUEUE ELEMENT\n");
						handler->queue->current = queue_item;
					}
				}
				pthread_mutex_unlock(&handler->queue->read_queue_lock);
				pthread_cond_signal(&handler->queue->queue_not_empty);
				freeSocketMessage(message);
			}
		}
		close(connect_d);
	}
}

