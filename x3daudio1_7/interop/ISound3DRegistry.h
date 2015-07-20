#pragma once

#include "math/math_types.h"

struct Sound3DEntry
{
	float volume_multiplier;
	float elevation;
	float azimuth;
};

typedef int sound_id;

class ISound3DRegistry
{
public:
	virtual ~ISound3DRegistry() = default;
	virtual sound_id CreateEntry(const Sound3DEntry & entry) = 0;
	virtual Sound3DEntry GetEntry(const sound_id & id) const = 0;

	virtual void Clear() = 0;
};
