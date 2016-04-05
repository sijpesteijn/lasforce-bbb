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
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../../include/global.h"

#define PORT 5555

int openSocket() {
	int reuse = 1;
	int listener_d = socket(PF_INET, SOCK_STREAM, 0);
	if (listener_d == -1) {
		syslog(LOG_ERR, "Sockethandler: Can't open socket.");
		perror("Can't open socket.");
		exit(1);
	}
	if (setsockopt(listener_d, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse,
			sizeof(int)) == -1) {
		syslog(LOG_ERR,
				"Sockethandler: Can't set the reuse option on the socket.");
		perror("Can't set the reuse option on the socket.");
		exit(1);
	}
	struct sockaddr_in name;
	name.sin_family = PF_INET;
	name.sin_port = (in_port_t) htons(PORT);
	name.sin_addr.s_addr = htonl(INADDR_ANY);

	int connection = bind(listener_d, (struct sockaddr*) &name, sizeof(name));
	if (connection == -1) {
		syslog(LOG_ERR, "Sockethandler: Can't bind to socket.");
		perror("Can't bind to socket.");
		exit(1);
	}

	if (listen(listener_d, 10) == -1) {
		syslog(LOG_ERR, "Sockethandler: Can't listen.");
		perror("Can't listen.");
		exit(1);
	}

	return listener_d;
}

int SocketHandler_init(void *self) {
	SocketHandler *handler = self;
	handler->socket = openSocket();
	syslog(LOG_INFO, "Sockethandler: listing on port: %i", PORT);
	return 1;
}

int getMessageLength(int connect_d) {
	int message_size_length = 13;
	char message_size[message_size_length];
	memset(message_size, 0, message_size_length * sizeof(char));
	int n = read(connect_d, message_size, message_size_length - 1);
	if (n < 0) {
		syslog(LOG_ERR, "Sockethandler: Can't read size from socket.");
		perror("Can't read size from socket.");
		exit(1);
	}
	message_size[n + 1] = '\0';
	syslog(LOG_DEBUG, "Sockethandler: get message length: %s", message_size);
	return atol(message_size);
}

socket_message* getQueueList(Queue *queue) {
	socket_message *smsg = malloc(sizeof(socket_message));

	char *listJson = getQueueListJson(queue);
	int len = strlen(listJson) + 12;

	char message_length[13];
	sprintf(message_length, "%012d", strlen(listJson));
	message_length[12] = '\0';

	char *content = (char*) malloc(len + 1);
	strcpy(content, message_length);
	strcat(content, listJson);
	smsg->content = content;
	smsg->length = len;
	return smsg;
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
		syslog(LOG_ERR, "Sockethandler: Can't allocate memory for message");
		perror("Can't allocate memory for message.");
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
			syslog(LOG_ERR, "Sockethandler: error reading message from socket");
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
//	syslog(LOG_DEBUG,"Sockethandler: msg received len: %lu %s", smsg->length, smsg->content);
	return smsg;
}

int sendSocketMessage(int connect_d, socket_message *smsg) {
	syslog(LOG_DEBUG, "Sockethandler: Sending message: %s", smsg->content);
	int n = write(connect_d, smsg->content, smsg->length);
	if (n < 0)
		syslog(LOG_ERR, "Sockethandler: Can't send socket message.");
	return n;
}

void SocketHandler_listen(void *self) {
	SocketHandler *handler = self;
	struct sockaddr_storage client_addr;
	unsigned int address_size = sizeof(client_addr);
	syslog(LOG_INFO, "Sockethandler waiting for connections.");
	for (;;) {
		int connect_d = accept(handler->socket, (struct sockaddr*) &client_addr,
				&address_size);
		if (connect_d == -1) {
			syslog(LOG_ERR, "Sockethandler: Can't open secondary socket.");
			perror("Sockethandler: Can't open secondary socket.");
			exit(1);
		}
		syslog(LOG_DEBUG,
				"Sockethandler: new socket connection. Waiting for messages...");
		socket_message* message = NULL;
		int handle = 1;
		while (handle) {
			message = readSocketMessage(connect_d);
			if (message->length > 0) {
				Command *command = command_deserialize(message->content,
						message->length);
				free(message);
				if (command == NULL) {
					syslog(LOG_ERR,
							"Sockethandler: Could not create command out of message.");
					continue;
				}
				QueueItem *queue_item = malloc(sizeof(QueueItem));
				queue_item->command = command;
				queue_item->next = NULL;

				if (pthread_mutex_lock(&handler->queue->queue_lock) != 0) {
					syslog(LOG_ERR,
							"Sockethandler: Could not get a lock on the queue");
					continue;
				}

				if (command->action == halt) {
					QueueItem *current = handler->queue->current;
					free_queue_item(current, 1);
					handler->queue->current = queue_item;
					handler->queue->last = queue_item;
				} else {
					if (command->action == list) {
						sendSocketMessage(connect_d,
								getQueueList(handler->queue));
					} else {
						if (handler->queue->last == NULL) {
							handler->queue->last = queue_item;
						} else {
							handler->queue->last->next = queue_item;
							handler->queue->last = handler->queue->last->next;
						}

						if (handler->queue->current == NULL) {
							syslog(LOG_DEBUG,
									"Sockethandler: First queue item placed.");
							handler->queue->current = queue_item;
						}
					}
					if (handler->queue->current != NULL) {
						syslog(LOG_DEBUG, "Sockethandler: Queue pushed. %s",
								getQueueListJson(handler->queue));
						pthread_cond_signal(&handler->queue->queue_not_empty);
					}
				}
				if (pthread_mutex_unlock(&handler->queue->queue_lock) != 0) {
					syslog(LOG_ERR,
							"Sockethandler: Could not unlock the queue");
					continue;
				}
			} else {
				syslog(LOG_ERR,
						"Sockethandler: I don't understand the message. Closing connection.");
				handle = 0;
			}
		}
		close(connect_d);
		syslog(LOG_DEBUG, "Sockethandler: socket connection closed.");
	}
}

