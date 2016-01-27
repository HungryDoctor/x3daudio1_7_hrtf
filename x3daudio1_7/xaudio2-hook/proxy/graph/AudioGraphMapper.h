#pragma once

#include <xaudio2.h>

class AudioGraphMapper
{
public:
	// AudioGraphMapper does not own xaudio
	AudioGraphMapper(IXAudio2 * xaudio);
	AudioGraphMapper(const AudioGraphMapper &);
	~AudioGraphMapper();

	IXAudio2SourceVoice * CreateSourceVoice(
		const WAVEFORMATEX * pSourceFormat,
		UINT32 Flags,
		float MaxFrequencyRatio,
		IXAudio2VoiceCallback * pCallback,
		const XAUDIO2_VOICE_SENDS * pSendList,
		const XAUDIO2_EFFECT_CHAIN * pEffectChain);

	IXAudio2SubmixVoice * CreateSubmixVoice(
		UINT32 InputChannels,
		UINT32 InputSampleRate,
		UINT32 Flags,
		UINT32 ProcessingStage,
		const XAUDIO2_VOICE_SENDS * pSendList,
		const XAUDIO2_EFFECT_CHAIN * pEffectChain);

	IXAudio2MasteringVoice * CreateMasteringVoice(
		UINT32 InputChannels,
		UINT32 InputSampleRate,
		UINT32 Flags,
		UINT32 DeviceIndex,
		const XAUDIO2_EFFECT_CHAIN * pEffectChain);


private:

};