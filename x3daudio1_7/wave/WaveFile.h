#pragma once

#include <string>
#include <cinttypes>
#include <fstream>

class WaveFile
{
public:
	WaveFile(const std::wstring & file_name, unsigned int num_channels, unsigned int samples_per_second, int bits_per_sample);
	~WaveFile();
	void AppendData(const uint8_t * buffer, size_t length);

private:
	std::ofstream m_stream;
	uint32_t m_data_size;
};