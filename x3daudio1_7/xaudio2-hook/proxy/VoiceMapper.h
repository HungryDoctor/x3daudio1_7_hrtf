#pragma once
#include "IVoiceMapper.h"
#include <unordered_map>

class VoiceMapper : public IVoiceMapper
{

public:
	VoiceMapper();
	virtual ~VoiceMapper();

	virtual void RememberMasteringVoice(IXAudio2MasteringVoice * original) override;
	virtual void RememberMap(IXAudio2Voice * original, IXAudio2Voice * proxy) override;
	virtual void ForgetMapByOriginal(IXAudio2Voice * original) override;
	virtual void ForgetMapByProxy(IXAudio2Voice * proxy) override;

	IXAudio2MasteringVoice* GetMasteringVoice() const override;

	// Maps send voices from proxy_sends to original ones. If there's no map for any of the proxy_sends, returns this voice itself.
	virtual void MapSendsToOriginal(const XAUDIO2_VOICE_SENDS & proxy_sends, XAUDIO2_VOICE_SENDS & original_sends) const override;

	// Cleans up memory, allocated for original_sends.pSends after MapSendsToOriginal.
	virtual void CleanupSends(XAUDIO2_VOICE_SENDS & original_sends) const override;

	// Maps voice from proxy_voice to original one. If there's no map for proxy_voice, returns proxy_voice itself.
	virtual IXAudio2Voice* MapVoiceToOriginal(IXAudio2Voice * proxy_voice) const override;
private:
	// maps from proxy voice to original voice
	std::unordered_map<IXAudio2Voice*, IXAudio2Voice*> m_map;
	IXAudio2MasteringVoice* m_mastering_voice;
};
