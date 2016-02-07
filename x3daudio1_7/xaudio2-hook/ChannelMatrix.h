#pragma once

#include <vector>
#include <algorithm>

class ChannelMatrix
{
public:
	ChannelMatrix()
		: m_sourceCount(0)
		, m_destinationCount(0)
	{

	}

	ChannelMatrix(unsigned int sourceCount, unsigned int destinationCount)
		: m_sourceCount(sourceCount)
		, m_destinationCount(destinationCount)
		, m_values(sourceCount * destinationCount)
	{
		
	}

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
		_ASSERT(source < m_sourceCount || destination < m_destinationCount);

		return m_values[destination * m_sourceCount + source];
	}

	void setValue(unsigned source, unsigned destination, float value)
	{
		_ASSERT(source < m_sourceCount || destination < m_destinationCount);

		m_values[destination * m_sourceCount + source] = value;
	}

private:
	unsigned int m_sourceCount;
	unsigned int m_destinationCount;
	std::vector<float> m_values;
};