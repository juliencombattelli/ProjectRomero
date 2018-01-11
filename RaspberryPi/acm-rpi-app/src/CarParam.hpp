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
		dir = 2;
		sonar = 0;
		new_mode = ACM_MODE_MANUAL;
		mode = ACM_MODE_MANUAL;

		turbo = false;
		moving = false;
		idle = true;

		autonomous_locked=0 ;
	}

	uint8_t dir;
	uint8_t sonar;
	uint8_t new_mode;
	uint8_t mode;
	bool turbo;
	bool moving;
	bool idle;

	int autonomous_locked;
};

enum class RoadDetection_t : uint8_t
{
	middle = 0,
	right,
	rightcrit,
	left,
	leftcrit
};

struct CarParamIn
{
	CarParamIn()
	{
		obst = 0;
		speed = 0;
		dir = 0;
		bat = 0;
		road_detection = RoadDetection_t::middle;
		memset(obstacles, 0, sizeof(obstacles));
	}

	uint8_t obst;
	obstacle obstacles[6];
	uint8_t speed;
	uint8_t dir;
	uint8_t bat;
	RoadDetection_t road_detection ;
};

} // namespace acm

#endif /* ACM_H_ */
