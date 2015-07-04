#pragma once

#include <XAudio2.h>

class IVoiceMapper
{
public:
	virtual ~IVoiceMapper() = default;

	virtual void RememberMap(IXAudio2Voice * original, IXAudio2Voice * proxy) = 0;
	virtual void ForgetMapByOriginal(IXAudio2Voice * original) = 0;
	virtual void ForgetMapByProxy(IXAudio2Voice * proxy) = 0;

	virtual void MapSendsToOriginal(const XAUDIO2_VOICE_SENDS & proxy_sends, XAUDIO2_VOICE_SENDS & original_sends) const = 0;
	virtual void CleanupSends(XAUDIO2_VOICE_SENDS & original_sends) const = 0;
	virtual IXAudio2Voice * MapVoiceToOriginal(IXAudio2Voice * proxy_voice) const = 0;
};