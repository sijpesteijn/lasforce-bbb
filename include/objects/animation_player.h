/*
 * player.h
 *
 *  Created on: Feb 11, 2016
 *      Author: gijs
 */
#ifndef PLAYER_H_
#define PLAYER_H_

#include "object.h"
#include "laser.h"
#include "../animation/animation.h"

struct Player {
	Object proto;
	Laser *laser;
	int run;
};

typedef struct Player Player;

int Player_init(void *self, void *object);
int Player_playAnimation(void *self, Animation *animation);

#endif
