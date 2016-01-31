#pragma once

#include <xaudio2.h>
#include <map>
#include <memory>
#include <xaudio2-hook/common_types.h>

class XAudio2SourceVoiceProxy;
class XAudio2VoiceProxy;

class VoiceDescriptor
{
public:
	IXAudio2Voice * proxyVoice;
	IXAudio2Voice * actualVoice;
};

class AudioGraphMapper
{
public:
	// AudioGraphMapper does not own xaudio
	AudioGraphMapper(IXAudio2 * xaudio);
	AudioGraphMapper(const AudioGraphMapper &) = delete;
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
	IXAudio2 * m_xaudio;
	std::map<IXAudio2Voice*, VoiceDescriptor> m_voices;
	VoiceDescriptor * m_masteringVoice;

	VoiceSends mapProxySendsToActualOnes(const VoiceSends & proxySends) const;
	const VoiceDescriptor & getDescriptorForProxyVoice(IXAudio2Voice* pDestination) const;
	VoiceDescriptor & getDescriptorForProxyVoice(IXAudio2Voice* pDestination);
	void setupCommonCallbacks(XAudio2VoiceProxy* proxyVoice, const std::shared_ptr<IXAudio2Voice> & actualVoice);
};