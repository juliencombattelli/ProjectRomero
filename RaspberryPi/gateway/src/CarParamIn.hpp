/*
 * acm.h
 *
 *  Created on: 13 dec. 2017
 *      Author: GuillaumeDeBrito
 */

#ifndef ACM_CARPARAMIN_H_
#define ACM_CARPARAMIN_H_

#include <thread>
#include <mutex>
#include "ObstacleDetector.hpp"

#define ACM_MODE_MANUAL 0
#define ACM_MODE_AUTONOMOUS 1

namespace acm
{

struct CarParamIn
{
	CarParamIn()
	{
		obst = 0 ; 
		speed = 0 ;
		dir = 0 ;
		bat = 0;
	}

	std::mutex mutex;
	uint8_t obst;
	obstacle obstacles[6] ; 
	uint8_t speed;
	uint8_t dir;
	uint8_t bat;
};

} // namespace acm

#endif /* ACM_CARPARAMIN_H_ */
