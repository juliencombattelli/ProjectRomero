/*
 * acm.h
 *
 *  Created on: 27 nov. 2017
 *      Author: JulienCombattelli
 */

#ifndef ACM_H_
#define ACM_H_


#define UUID_ACM 		0x7DB9
#define UUID_STATE 		0xD288

#define ACM_MODE_MANUAL 0
#define ACM_MODE_AUTONOMOUS 1

struct acm_car
{
	int dir;
	int sonar;
	int new_mode;

	int turbo;
	int moving;
	int mode;
	int idle;
};


void acm_car_init(struct acm_car *car);

#endif /* ACM_H_ */
