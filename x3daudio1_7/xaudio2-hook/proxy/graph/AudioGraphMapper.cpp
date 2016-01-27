#include "stdafx.h"

#include "AudioGraphMapper.h"

AudioGraphMapper::AudioGraphMapper(IXAudio2* xaudio)
{
}

AudioGraphMapper::AudioGraphMapper(const AudioGraphMapper&)
{
}

AudioGraphMapper::~AudioGraphMapper()
{
}

IXAudio2SourceVoice* AudioGraphMapper::CreateSourceVoice(const WAVEFORMATEX* pSourceFormat, UINT32 Flags, float MaxFrequencyRatio, IXAudio2VoiceCallback* pCallback, const XAUDIO2_VOICE_SENDS* pSendList, const XAUDIO2_EFFECT_CHAIN* pEffectChain)
{
	return nullptr;
}

IXAudio2SubmixVoice* AudioGraphMapper::CreateSubmixVoice(UINT32 InputChannels, UINT32 InputSampleRate, UINT32 Flags, UINT32 ProcessingStage, const XAUDIO2_VOICE_SENDS* pSendList, const XAUDIO2_EFFECT_CHAIN* pEffectChain)
{
	return nullptr;
}

IXAudio2MasteringVoice* AudioGraphMapper::CreateMasteringVoice(UINT32 InputChannels, UINT32 InputSampleRate, UINT32 Flags, UINT32 DeviceIndex, const XAUDIO2_EFFECT_CHAIN* pEffectChain)
{
	return nullptr;
}
