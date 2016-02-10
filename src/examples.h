/*
 * examples.h
 *
 *  Created on: Feb 9, 2016
 *      Author: gijs
 */

#ifndef EXAMPLES_H_
#define EXAMPLES_H_

#include "../include/objects/laser.h"

const int j[12][2] = {{0,47},{28,47},{32,44},{35,41},{37,38},{37,11},{29,11},{29,35},{28,38},{26,39},{7,39},{0,47}};
const int p[29][2] = {{41,49},{49,49},{49,21},{50,19},{53,17},{77,17},{80,19},{81,20},{81,24},{80,27},{78,28},{54,28},
		{54,37},{80,37},{84,34},{87,32},{89,28},{90,25},{90,20},{89,16},{86,13},{82,10},{79,9},{52,9},{47,11},{45,13},
		{43,15},{41,19},{41,49}};
const int o1[19][2] = {{104,37},{133,37},{139,33},{142,30},{144,26},{144,19},{142,15},{139,12},{133,9},{106,9},{100,11},
		{98,13},{96,15},{94,19},{94,25},{96,28},{98,32},{100,34},{104,37}};
const int o2[11][2] = {{106,28},{131,28},{134,26},{135,23},{134,19},{131,17},{106,17},{104,19},{103,22},{104,26},{106,28}};
const int i1[5][2] = {{148,37},{156,37},{156,9},{148,9},{148,37}};
const int i2[5][2] = {{148,6},{156,6},{156,0},{148,0},{148,6}};
const int n[20][2] = {{160,37},{169,37},{169,19},{171,17},{173,16},{197,16},{200,18},{202,18},{202,36},{210,36},{210,19},
		{208,14},{204,10},{199,8},{172,8},{166,10},{164,12},{162,15},{160,17},{160,37}};
const int t[17][2] = {{213,1},{213,26},{215,30},{218,33},{221,35},{224,37},{248,37},{248,28},{225,28},{223,27},{222,24},
		{222,16},{243,16},{243,9},{222,9},{222,1},{213,1}};

void drawLetter(int size, const int letter[size][2], Laser *laser) {
	int i;
	laser->_(setGreen)(laser, 255);
	for(i=0;i<size;i++) {
		int x = (letter[i][0]*10);
		int y = (letter[i][1]*10);
		laser->_(setX)(laser, x);
		laser->_(setY)(laser, y);
		usleep(10);
	}
}

void drawLogo(Laser *laser, int repeat) {
	int i = 0;
	while(i++ < repeat) {
		drawLetter(12,j, laser);
		drawLetter(29,p, laser);
		drawLetter(19,o1, laser);
		drawLetter(11,o2, laser);
		drawLetter(5,i1, laser);
		drawLetter(5,i2, laser);
		drawLetter(20,n, laser);
		drawLetter(17,t, laser);
	}
}

void drawSquare(Laser *laser, int repeat) {
	laser->_(setRed)(laser, 255);
	laser->_(setGreen)(laser, 255);
	laser->_(setBlue)(laser, 255);
	int i = 0, delay = 1000;
	while(i++ < repeat) {
		laser->_(setX)(laser, 0);
		usleep(delay);
		laser->_(setY)(laser, 0);
		usleep(delay);
		laser->_(setX)(laser, 3000);
		usleep(delay);
		laser->_(setY)(laser, 3000);
		usleep(delay);
	}
}

#endif /* EXAMPLES_H_ */
