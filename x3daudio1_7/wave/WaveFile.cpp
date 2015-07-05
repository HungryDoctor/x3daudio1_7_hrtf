#include "stdafx.h"
#include "WaveFile.h"

#include "logger.h"

struct wavfile_header
{
	char chunk_id[4];
	int chunk_size;
	char format_id[4];
	char subchunk_id[4];
	int subchunk_size;
	short audio_format;
	short num_channels;
	int sample_rate;
	int byte_rate;
	short block_align;
	short bits_per_sample;
	char data_tag[4];
	int data_length;
};

WaveFile::WaveFile(const std::wstring & file_name, unsigned int num_channels, unsigned int samples_per_second, int bits_per_sample)
	: m_stream(file_name, std::fstream::out | std::fstream::binary)
	, m_data_size(0)
{
	wavfile_header header;

	header.chunk_id[0] = 'R'; header.chunk_id[1] = 'I'; header.chunk_id[2] = 'F'; header.chunk_id[3] = 'F';
	header.chunk_size = 0; // leave for now
	header.format_id[0] = 'W'; header.format_id[1] = 'A'; header.format_id[2] = 'V'; header.format_id[3] = 'E';
	header.subchunk_id[0] = 'f'; header.subchunk_id[1] = 'm'; header.subchunk_id[2] = 't'; header.subchunk_id[3] = ' ';
	header.subchunk_size = 16;
	header.audio_format = 1;
	header.num_channels = num_channels;
	header.sample_rate = samples_per_second;
	header.byte_rate = samples_per_second * num_channels * (bits_per_sample / 8);
	header.block_align = bits_per_sample / 8;
	header.bits_per_sample = bits_per_sample;
	header.data_tag[0] = 'd'; header.data_tag[1] = 'a'; header.data_tag[2] = 't'; header.data_tag[3] = 'a';
	header.data_length = 0;

	m_stream.write(reinterpret_cast<char*>(&header), sizeof(header));
	m_stream.flush();
}

WaveFile::~WaveFile()
{
	int riff_length = m_data_size + sizeof(wavfile_header) - offsetof(wavfile_header, chunk_size);
	m_stream.seekp(offsetof(wavfile_header, data_length));
	m_stream.write(reinterpret_cast<char *>(&m_data_size), sizeof(uint32_t));

	m_stream.seekp(offsetof(wavfile_header, chunk_size));
	m_stream.write(reinterpret_cast<char *>(&riff_length), sizeof(uint32_t));

	m_stream.flush();

	m_stream.close();
}

void WaveFile::AppendData(const uint8_t * buffer, size_t length)
{
	m_stream.write(reinterpret_cast<const char*>(buffer), length);
	m_stream.flush();
	m_data_size += length;
}
