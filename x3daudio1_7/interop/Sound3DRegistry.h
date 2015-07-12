#pragma once

#include "ISound3DRegistry.h"
#include <unordered_map>
#include <mutex>

class Sound3DRegistry : public ISound3DRegistry
{
public:
	static Sound3DRegistry& GetInstance();

public:
	virtual sound_id CreateEntry(const Sound3DEntry & entry) override;
	virtual Sound3DEntry GetEntry(const sound_id & id) const override;
	virtual void Clear() override;

private:
	sound_id m_id_counter;
	std::unordered_map<sound_id, Sound3DEntry> m_registry;
	mutable std::mutex m_mutex;

	static Sound3DRegistry _instance;
};
