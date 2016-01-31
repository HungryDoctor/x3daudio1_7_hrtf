#pragma once

#include <vector>
#include <algorithm>

class ChannelMatrix
{
public:
	ChannelMatrix(const float * values, unsigned int sourceCount, unsigned int destinationCount)
		: m_sourceCount(sourceCount)
		, m_destinationCount(destinationCount)
		, m_values(sourceCount * destinationCount)
	{
		auto size = sourceCount * destinationCount;
		std::copy(values, values + size, std::begin(m_values));
	}

	unsigned getSourceCount() const
	{
		return m_sourceCount;
	}

	unsigned getDestinationCount() const
	{
		return m_destinationCount;
	}

	float getValue(unsigned source, unsigned destination) const
	{
		if (source >= m_sourceCount || destination >= m_destinationCount)
			throw std::logic_error("Accessing element out of matrix bounds.");

		return m_values[destination * m_sourceCount + source];
	}

private:
	unsigned int m_sourceCount;
	unsigned int m_destinationCount;
	std::vector<float> m_values;
};