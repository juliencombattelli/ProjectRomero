/*
 * Csv.hpp
 *
 *  Created on: Jan 11, 2018
 *      Author: coegre
 */

#ifndef CSV_HPP_
#define CSV_HPP_


#include "CarParam.hpp"

namespace acm
{

class Csv
{
public:	

	Csv(const std::string &fileName) {
		m_fileName = fileName;
		f = fopen(m_fileName.c_str(), "w");
		}
	~Csv() {fclose(f);}
	
	
	void generate_csv(const CarParamOut &m_carParamOut, const CarParamIn &m_carParamIn){
		uint8_t speed;
		uint8_t currentMode;
		int obst_detected = 0;
		int obst_mobile = 0;
		int obst_min_dist = 0;
		
		speed =  m_carParamIn.speed * 0.36; // Speed conversion from dm/s to km/h
		
		if (m_carParamOut.mode == obstAvoiding || m_carParamOut.mode == manual)
			currentMode = manual;
		
		else if (m_carParamOut.mode == emergencyStop || m_carParamOut.mode == autonomous)
			currentMode = autonomous;
		
		
		fprintf(self->f, "%d;%d;%d;%d;%d;%d;%d;",currentMode, m_carParamOut.requestedMode, speed, m_carParamOut.turbo, 
											   m_carParamIn.dir, m_carParamOut.dir, m_carParamIn.road_detection);
				
		for (int i = 0 ; i < 6 ; i++)
		{
			if (m_carParamIn.obstacles[i].detected == 1 && obst_detected == 0)
				obst_detected = 1;
			if (m_carParamIn.obstacles[i].mobile == 1 && obst_mobile == 0)
				int obst_mobile = 1;
			if (m_carParamIn.obstacles[i].detected == 1)
			{
				if(obst_min_dist == 0)
					obst_min_dist = m_carParamIn.obstacles[i].dist;
				else
				{
					if (m_carParamIn.obstacles[i].dist < obst_min_dist)
						obst_min_dist = m_carParamIn.obstacles[i].dist;					
				}
			}			
		}
		fprintf(self->f, "%d;%d;%d;",obst_detected,obst_mobile,obst_min_dist);		
		fprintf(self->f,"\n");
	};

private:	

	std::string m_fileName;
	FILE *f;
};

} // namespace acm

#endif /* CSV_HPP_ */
