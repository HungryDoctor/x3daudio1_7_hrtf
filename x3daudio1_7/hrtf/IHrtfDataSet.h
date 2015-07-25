#pragma once
#include "IHrtfData.h"
#include "HrtfTypes.h"
#include <cstdint>

class IHrtfDataSet
{
public:
	virtual bool has_sample_rate(uint32_t sample_rate) const = 0;
	virtual const IHrtfData & get_sample_rate_data(uint32_t sampl_rate) const = 0;
};