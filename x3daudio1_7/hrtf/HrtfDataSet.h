#pragma once

#include "IHrtfDataSet.h"
#include "HrtfData.h"
#include <map>
#include <vector>

class HrtfDataSet : public IHrtfDataSet
{
public:
	HrtfDataSet(const std::vector<std::wstring> & data_files);

	bool has_sample_rate(uint32_t sample_rate) const override;
	const IHrtfData & get_sample_rate_data(uint32_t sampl_rate) const override;
private:
	std::map<uint32_t, HrtfData> m_data; // sample rate - data pairs
};
