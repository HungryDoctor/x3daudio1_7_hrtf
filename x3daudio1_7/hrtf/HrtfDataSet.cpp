#include "HrtfDataSet.h"
#include <fstream>

HrtfDataSet::HrtfDataSet(const std::vector<std::wstring> & data_files)
{
	for (auto& data_file_name : data_files)
	{
		std::ifstream file(data_file_name, std::fstream::binary);
		HrtfData data(file);
		m_data.insert({ data.get_sample_rate(), std::move(data) });
	}
}

void HrtfDataSet::GetDirectionData(uint32_t sample_rate, angle_t elevation, angle_t azimuth, DirectionData& ref_data) const
{
	m_data.at(sample_rate).GetDirectionData(elevation, azimuth, ref_data);
}

void HrtfDataSet::GetDirectionData(uint32_t sample_rate, angle_t elevation, angle_t azimuth, DirectionData& ref_data_left, DirectionData& ref_data_right) const
{
	m_data.at(sample_rate).GetDirectionData(elevation, azimuth, ref_data_left, ref_data_right);
}

bool HrtfDataSet::has_sample_rate(uint32_t sample_rate) const
{
	return m_data.find(sample_rate) != m_data.end();
}