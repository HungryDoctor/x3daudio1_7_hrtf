#include "stdafx.h"
#include "VoiceMapper.h"

#include <algorithm>

VoiceMapper::VoiceMapper()
	: m_mastering_voice(nullptr)
{
}


VoiceMapper::~VoiceMapper()
{
}

void VoiceMapper::RememberMasteringVoice(IXAudio2MasteringVoice* original)
{
	m_mastering_voice = original;
}

void VoiceMapper::RememberMap(IXAudio2Voice * original, IXAudio2Voice * proxy)
{
	m_map.insert(std::make_pair(proxy, original));
}

bool pred(std::pair<IXAudio2Voice *, IXAudio2Voice*> pair)
{
	return pair.second != nullptr;
}

void VoiceMapper::ForgetMapByOriginal(IXAudio2Voice * original)
{
	auto it = std::find_if(m_map.begin(), m_map.end(), pred);
}

void VoiceMapper::ForgetMapByProxy(IXAudio2Voice * proxy)
{
	m_map.erase(proxy);
}

IXAudio2MasteringVoice* VoiceMapper::GetMasteringVoice() const
{
	return m_mastering_voice;
}

void VoiceMapper::MapSendsToOriginal(const XAUDIO2_VOICE_SENDS & proxy_sends, XAUDIO2_VOICE_SENDS & original_sends) const
{
	original_sends.SendCount = proxy_sends.SendCount;
	original_sends.pSends = proxy_sends.SendCount > 0 ? new XAUDIO2_SEND_DESCRIPTOR[proxy_sends.SendCount] : nullptr;

	for (UINT32 i = 0; i < proxy_sends.SendCount; i++)
	{
		original_sends.pSends[i].Flags = proxy_sends.pSends[i].Flags;
		original_sends.pSends[i].pOutputVoice = MapVoiceToOriginal(proxy_sends.pSends[i].pOutputVoice);
	}
}

void VoiceMapper::CleanupSends(XAUDIO2_VOICE_SENDS & original_sends) const
{
	if (original_sends.pSends)
		delete[] original_sends.pSends;
}

IXAudio2Voice* VoiceMapper::MapVoiceToOriginal(IXAudio2Voice * proxy_voice) const
{
	auto found_it = m_map.find(proxy_voice);
	return found_it != m_map.end() ? found_it->second : proxy_voice;
}
