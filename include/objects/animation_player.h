/*
 * player.h
 *
 *  Created on: Feb 11, 2016
 *      Author: gijs
 */
#ifndef PLAYER_H_
#define PLAYER_H_

#include "object.h"
#include "../animation/animation.h"
#include "laser.h"

struct Player {
	Object proto;
	Laser *laser;
	Queue *queue;
	int run;
	int repeat_animation;
	double frame_time;
};

typedef struct Player Player;

int Player_init(void *self);
void Player_listen(void *self);
void Player_douse(void *self);

#endif
