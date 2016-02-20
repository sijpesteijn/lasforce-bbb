/*
 * commands.h
 *
 *  Created on: Feb 19, 2016
 *      Author: gijs
 */

#ifndef COMMANDS_H_
#define COMMANDS_H_

typedef struct {
	char *command;
	int command_length;
	char *payload;
	int payload_length;
} Command;

#endif /* COMMANDS_H_ */
