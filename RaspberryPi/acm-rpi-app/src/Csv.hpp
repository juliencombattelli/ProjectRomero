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

	Csv(const std::string &fileName)
	{
		m_fileName = fileName;
		m_file = fopen(m_fileName.c_str(), "w");
	}
	~Csv()
	{
		fclose(m_file);
	}

	void generate_csv(const CarParamOut &m_carParamOut, const CarParamIn &m_carParamIn)
	{
		uint8_t speed;
		speed = m_carParamIn.speed * 0.36; // Speed conversion from dm/s to km/h
		fprintf(m_file, "%d;%d;%d;%d;", m_carParamOut.mode, speed, m_carParamIn.dir, (int)m_carParamIn.road_detection);
		for (int i = 0; i < 6; i++)
		{
			fprintf(m_file, "%d;%d;%d;", m_carParamIn.obstacles[i].detected,
					m_carParamIn.obstacles[i].mobile,
					m_carParamIn.obstacles[i].dist);
		}
		fprintf(m_file, "\n");
	}

private:

	std::string m_fileName;
	FILE *m_file;
};

} // namespace acm

#endif /* CSV_HPP_ */
