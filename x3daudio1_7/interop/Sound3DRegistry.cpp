#include "stdafx.h"
#include "Sound3DRegistry.h"

Sound3DRegistry Sound3DRegistry::_instance;

Sound3DRegistry & Sound3DRegistry::GetInstance()
{
	return _instance;
}

sound_id Sound3DRegistry::CreateEntry(const Sound3DEntry & entry)
{
	std::lock_guard<std::mutex> lock_guard(m_mutex);
	m_registry.insert(std::make_pair(m_id_counter, entry));
	return m_id_counter++;
}

Sound3DEntry Sound3DRegistry::GetEntry(const sound_id & id) const
{
	std::lock_guard<std::mutex> lock_guard(m_mutex);
	return m_registry.at(id);
}

void Sound3DRegistry::Clear()
{
	m_registry.clear();
}
