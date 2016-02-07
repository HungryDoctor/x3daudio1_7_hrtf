#include "stdafx.h"
#include "HrtfDataSet.h"
#include <fstream>
#include <algorithm>

HrtfDataSet::HrtfDataSet(const std::vector<std::wstring> & data_files)
{
	for (auto& data_file_name : data_files)
	{
		std::ifstream file(data_file_name, std::fstream::binary);

		if (!file.is_open())
			throw std::logic_error("Cannot open file");

		HrtfData data(file);
		m_data.insert({ data.get_sample_rate(), std::move(data) });
	}
}

bool HrtfDataSet::has_sample_rate(uint32_t sample_rate) const
{
	return m_data.find(sample_rate) != m_data.end();
}

const IHrtfData & HrtfDataSet::get_sample_rate_data(uint32_t sampl_rate) const
{
	return m_data.at(sampl_rate);
}
