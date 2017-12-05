/*
 * acm.c
 *
 *  Created on: 27 nov. 2017
 *      Author: JulienCombattelli
 */


#include "acm.h"
#include <stdbool.h>

void acm_car_init(struct acm_car *car)
{
	car->dir = 2;
	car->sonar = 0;
	car->new_mode = 0;

	car->turbo = false;
	car->moving = false;
	car->mode = ACM_MODE_MANUAL;
	car->idle = true;
}
