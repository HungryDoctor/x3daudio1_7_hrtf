#pragma once
// damn, I can't think of a good name for this file

#include "ChannelMatrix.h"
#include "interop/ISound3DRegistry.h"
#include <XAudio2.h>
#include <limits>

inline void embed_sound_id(float * matrix, UINT32 sourceChannels, UINT32 destChannels, sound_id id)
{
	if (sourceChannels * destChannels < 2)
		throw std::logic_error("To few matrix elements to embed id into it.");

	matrix[0] = std::numeric_limits<float>::quiet_NaN(); // marking as hacked
	matrix[1] = *reinterpret_cast<float*>(&id);
}

inline bool does_matrix_contain_id(const ChannelMatrix & matrix)
{
	return std::isnan(matrix.getValue(0, 0));
}

inline sound_id extract_sound_id(const ChannelMatrix & matrix)
{
	float value = matrix.getValue(1, 0);
	return *reinterpret_cast<sound_id*>(&value);
}