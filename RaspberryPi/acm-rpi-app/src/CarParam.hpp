/*
 * acm.h
 *
 *  Created on: 27 nov. 2017
 *      Author: JulienCombattelli
 */

#ifndef ACM_H_
#define ACM_H_

#include <thread>
#include <mutex>

#include "ObstacleDetector.hpp"

namespace acm
{

enum AcmMode : uint8_t
{
	ACM_MODE_MANUAL 		= 0,
	ACM_MODE_AUTONOMOUS 	= 1
};


struct CarParamOut
{
	CarParamOut()
	{
		dir = 0;
		sonar = 0;
		new_mode = ACM_MODE_MANUAL;
		mode = ACM_MODE_MANUAL;

		turbo = false;
		moving = false;
		idle = true;

		autonomous_locked=0 ;
	}

	std::mutex mutex;
	uint8_t dir;
	uint8_t sonar;
	uint8_t new_mode;
	uint8_t mode;
	bool turbo;
	bool moving;
	bool idle;

	int autonomous_locked;
};

struct CarParamIn
{
	CarParamIn()
	{
		obst = 0;
		speed = 0;
		dir = 0;
		bat = 0;
		memset(obstacles, 0, sizeof(obstacles));
	}

	std::mutex mutex;
	uint8_t obst;
	obstacle obstacles[6];
	uint8_t speed;
	uint8_t dir;
	uint8_t bat;
};

} // namespace acm

#endif /* ACM_H_ */
