#include "HrtfData.h"
#include "Endianness.h"
#include <algorithm>

typedef uint8_t impulse_response_length_t;
typedef uint8_t elevations_count_t;
typedef uint8_t azimuth_count_t;
typedef uint32_t sample_rate_t;
typedef int16_t sample_t;
typedef uint8_t delay_t;

const double pi = 3.1415926535897932385;

template <typename T>
void read_stream(std::istream & stream, T & value)
{
	stream.read(reinterpret_cast<std::istream::char_type*>(&value), sizeof(value));
	from_little_endian_inplace(value);
}

HrtfData::HrtfData(std::istream & stream)
{
	const char required_magic[] = { 'M', 'i', 'n', 'P', 'H', 'R', '0', '1' };
	char actual_magic[sizeof(required_magic) / sizeof(required_magic[0])];

	stream.read(actual_magic, sizeof(actual_magic));
	if (!std::equal(std::begin(required_magic), std::end(required_magic), std::begin(actual_magic), std::end(actual_magic)))
	{
		throw std::logic_error("Bad file format.");
	}

	sample_rate_t sample_rate;
	impulse_response_length_t impulse_response_length;
	elevations_count_t elevations_count;

	read_stream(stream, sample_rate);
	read_stream(stream, impulse_response_length);
	read_stream(stream, elevations_count);

	std::vector<ElevationData> elevations(elevations_count);

	for (elevations_count_t i = 0; i < elevations_count; i++)
	{
		azimuth_count_t azimuth_count;
		read_stream(stream, azimuth_count);
		elevations[i].azimuths.resize(azimuth_count);
	}

	const float normalization_factor = 1.0f / float(1 << (sizeof(sample_t) * 8 - 1));

	for (auto& elevation : elevations)
	{
		for (auto& azimuth : elevation.azimuths)
		{
			azimuth.impulse_response.resize(impulse_response_length);
			for (auto& sample : azimuth.impulse_response)
			{
				sample_t sample_from_file;
				read_stream(stream, sample_from_file);
				sample = sample_from_file * normalization_factor;
			}
		}
	}

	for (auto& elevation : elevations)
	{
		for (auto& azimuth : elevation.azimuths)
		{
			delay_t delay;
			read_stream(stream, delay);
			azimuth.delay = delay;
		}
	}

	m_elevations = std::move(elevations);
	m_response_length = impulse_response_length;
	m_sample_rate = sample_rate;
}

void HrtfData::GetDirectionData(angle_t elevation, angle_t azimuth, DirectionData& ref_data) const
{
	_ASSERT(elevation >= -angle_t(pi * 0.5));
	_ASSERT(elevation <= angle_t(pi * 0.5));
	_ASSERT(azimuth >= 0.0);

	const float azimuth_mod = std::fmod(azimuth, angle_t(pi * 2.0));

	const angle_t elevation_scaled = (elevation + angle_t(pi * 0.5)) * (m_elevations.size() - 1) / angle_t(pi);
	const size_t elevation_index0 = static_cast<size_t>(elevation_scaled);
	const size_t elevation_index1 = std::min(elevation_index0 + 1, m_elevations.size() - 1);
	const float elevation_fractional_part = elevation_scaled - std::floor(elevation_scaled);

	const angle_t azimuth_scaled0 = (azimuth_mod + 2 * angle_t(pi)) * m_elevations[elevation_index0].azimuths.size() / angle_t(2 * pi);
	const size_t azimuth_index00 = static_cast<size_t>(azimuth_scaled0) % m_elevations[elevation_index0].azimuths.size();
	const size_t azimuth_index01 = static_cast<size_t>(azimuth_scaled0 + 1) % m_elevations[elevation_index0].azimuths.size();
	const float azimuth_fractional_part0 = azimuth_scaled0 - std::floor(azimuth_scaled0);

	const angle_t azimuth_scaled1 = (azimuth_mod + 2 * angle_t(pi)) * m_elevations[elevation_index1].azimuths.size() / angle_t(2 * pi);
	const size_t azimuth_index10 = static_cast<size_t>(azimuth_scaled1) % m_elevations[elevation_index1].azimuths.size();
	const size_t azimuth_index11 = static_cast<size_t>(azimuth_scaled1 + 1) % m_elevations[elevation_index1].azimuths.size();
	const float azimuth_fractional_part1 = azimuth_scaled1 - std::floor(azimuth_scaled1);

	const float blend_factor_00 = (1.0f - elevation_fractional_part) * (1.0f - azimuth_fractional_part0);
	const float blend_factor_01 = (1.0f - elevation_fractional_part) * (azimuth_fractional_part0);
	const float blend_factor_10 = (elevation_fractional_part)* (1.0f - azimuth_fractional_part1);
	const float blend_factor_11 = (elevation_fractional_part)* (azimuth_fractional_part1);

	float delay =
		m_elevations[elevation_index0].azimuths[azimuth_index00].delay * blend_factor_00
		+ m_elevations[elevation_index0].azimuths[azimuth_index01].delay * blend_factor_01
		+ m_elevations[elevation_index1].azimuths[azimuth_index10].delay * blend_factor_10
		+ m_elevations[elevation_index1].azimuths[azimuth_index11].delay * blend_factor_11;

	ref_data.delay = static_cast<uint32_t>(std::round(delay));

	if (ref_data.impulse_response.size() < m_response_length)
		ref_data.impulse_response.resize(m_response_length);

	for (size_t i = 0; i < m_response_length; i++)
	{
		ref_data.impulse_response[i] =
			m_elevations[elevation_index0].azimuths[azimuth_index00].impulse_response[i] * blend_factor_00
			+ m_elevations[elevation_index0].azimuths[azimuth_index01].impulse_response[i] * blend_factor_01
			+ m_elevations[elevation_index1].azimuths[azimuth_index10].impulse_response[i] * blend_factor_10
			+ m_elevations[elevation_index1].azimuths[azimuth_index11].impulse_response[i] * blend_factor_11;
	}
}

void HrtfData::GetDirectionData(angle_t elevation, angle_t azimuth, DirectionData& ref_data_left, DirectionData& ref_data_right) const
{
	_ASSERT(elevation >= -angle_t(pi * 0.5));
	_ASSERT(elevation <= angle_t(pi * 0.5));
	_ASSERT(azimuth >= 0.0);
	_ASSERT(azimuth < angle_t(pi * 2.0));

	GetDirectionData(elevation, azimuth, ref_data_left);
	GetDirectionData(elevation, angle_t(2 * pi) - azimuth, ref_data_right);
}