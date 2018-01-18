/*
 * acm.h
 *
 *  Created on: 27 nov. 2017
 *      Author: JulienCombattelli
 */

#ifndef ACM_H_
#define ACM_H_

#include <cstring>

#include "ObstacleDetector.hpp"

namespace acm
{

enum AcmMode : uint8_t
{
	ACM_MODE_MANUAL 		= 0,
	ACM_MODE_AUTONOMOUS 	= 1
};

enum class RoadDetection_t : uint8_t
{
	middle = 0,
	right,
	rightcrit,
	left,
	leftcrit
};

enum class Direction_t : uint8_t
{
	middle = 0,
	leftCrit,
	rightCrit,
	left,
	right
};

struct CarParamOut
{
	CarParamOut()
	{
		dir = Direction_t::middle;
		sonar = 0;
		requestedMode = ACM_MODE_MANUAL;
		mode = ACM_MODE_MANUAL;

		turbo = false;
		moving = false;
		idle = true;

		autonomousLocked=0 ;
	}

	Direction_t dir;
	uint8_t sonar;
	uint8_t requestedMode;
	uint8_t mode;
	bool turbo;
	bool moving;
	bool idle;

	int autonomousLocked;
};

struct CarParamIn
{
	CarParamIn()
	{
		obst = 0;
		speed = 0;
		dir = Direction_t::middle;
		bat = 0;
		roadDetection = RoadDetection_t::middle;
		memset(obstacles, 0, sizeof(obstacles));
	}

	uint8_t obst;
	obstacle obstacles[6];
	uint8_t speed;
	Direction_t dir;
	uint8_t bat;
	RoadDetection_t roadDetection ;
};

} // namespace acm

#endif /* ACM_H_ */
