#pragma once

#include <stdint.h>
#include <vector>
#include <iostream>

typedef float angle_t;

struct DirectionData
{
	std::vector<float> impulse_response;
	uint32_t delay;
};


struct ElevationData
{
	std::vector<DirectionData> azimuths;
};

class HrtfData
{
public:
	HrtfData(std::istream & stream);

	void GetDirectionData(angle_t elevation, angle_t azimuth, DirectionData & ref_data) const;
	void GetDirectionData(angle_t elevation, angle_t azimuth, DirectionData & ref_data_left, DirectionData & ref_data_right) const;

	uint32_t get_sample_rate() const { return m_sample_rate; }
	uint32_t get_respoone_length() const { return m_response_length; }

private:
	uint32_t m_sample_rate;
	uint32_t m_response_length;
	std::vector<ElevationData> m_elevations;
};