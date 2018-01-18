/*
 * Csv.hpp
 *
 *  Created on: Jan 11, 2018
 *      Author: coegre
 */

#ifndef LOGGER_HPP_
#define LOGGER_HPP_

#include "CarParam.hpp"
#include "fstream"

namespace acm
{

class CsvLogger
{
public:

	CsvLogger(const std::string &fileName)
	{
		m_fileName = fileName;
		m_file = fopen(m_fileName.c_str(), "w");
	}
	~CsvLogger()
	{
		fclose(m_file);
	}

	void generate_csv(const CarParamOut &m_carParamOut, const CarParamIn &m_carParamIn)
	{
		uint8_t speed;
		speed = m_carParamIn.speed * 0.36; // Speed conversion from dm/s to km/h
		fprintf(m_file, "%d;%d;%d;%d;", m_carParamOut.mode, speed, (int)m_carParamIn.dir, (int)m_carParamIn.roadDetection);
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

class TimeLogger
{
public:

	TimeLogger(const std::string &fileName)
	{
		m_fileName = fileName;
		m_file.open(m_fileName);
	}
	~TimeLogger() = default;

	void write(const std::string& header, double time)
	{
		m_file << header << time << std::endl;
	}

private:

	std::string m_fileName;
	std::ofstream m_file;
};

} // namespace acm

#endif /* LOGGER_HPP_ */
