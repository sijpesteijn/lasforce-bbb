/*
 * deserialize.h
 *
 *  Created on: Feb 10, 2016
 *      Author: gijs
 */

#ifndef ANIMATION_DESERIALIZE_H_
#define ANIMATION_DESERIALIZE_H_

#include "animation.h"

Animation* animation_deserialize(const char* smsg, int smsgl);
Command* command_deserialize(const char* smsg, int smsgl);

#endif /* ANIMATION_DESERIALIZE_H_ */
