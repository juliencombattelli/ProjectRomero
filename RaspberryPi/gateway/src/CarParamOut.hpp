/*
 * acm.h
 *
 *  Created on: 27 nov. 2017
 *      Author: JulienCombattelli
 */

#ifndef ACM_CARPARAMOUT_H_
#define ACM_CARPARAMOUT_H_

#include <thread>
#include <mutex>

#define ACM_MODE_MANUAL 0
#define ACM_MODE_AUTONOMOUS 1

namespace acm
{

struct CarParamOut
{
	CarParamOut()
	{
		dir = 2;
		sonar = 0;
		new_mode = 0;

		turbo = false;
		moving = false;
		mode = ACM_MODE_MANUAL;
		idle = true;
		
		autonomous_locked=0 ; 
	}

	std::mutex mutex;
	int dir;
	int sonar;
	int new_mode;

	int turbo;
	int moving;
	int mode;
	int idle;

	int autonomous_locked; 
};

} // namespace acm

#endif /* ACM_CARPARAMOUT_H_ */
