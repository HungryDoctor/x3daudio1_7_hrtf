#pragma once

#include "HrtfData.h"
#include <map>
#include <string>
#include <vector>

class HrtfDataSet
{
public:
	HrtfDataSet(const std::vector<std::wstring> & data_files);

	void GetDirectionData(uint32_t sample_rate, angle_t elevation, angle_t azimuth, DirectionData & ref_data) const;
	void GetDirectionData(uint32_t sample_rate, angle_t elevation, angle_t azimuth, DirectionData & ref_data_left, DirectionData & ref_data_right) const;

	bool has_sample_rate(uint32_t sample_rate) const;
private:
	std::map<uint32_t, HrtfData> m_data; // sample rate - data pairs
};
