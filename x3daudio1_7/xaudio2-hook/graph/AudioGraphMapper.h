#pragma once

#include "EdgeRepository.h"

#include <xaudio2.h>
#include <map>
#include <memory>
#include <xaudio2-hook/common_types.h>


class XAudio2SourceVoiceProxy;
class XAudio2VoiceProxy;

class Node
{
public:
	Node() = default;
	Node(const Node& other) = delete;
	Node& operator=(const Node& other) = delete;

	~Node()
	{
		proxyVoice.reset();
		mainVoice.reset();
		tailVoices.clear(); // must be destroyed last
	}

	std::shared_ptr<IXAudio2Voice> proxyVoice;
	std::shared_ptr<IXAudio2Voice> mainVoice;
	// maps proxy send to actual tail voice
	std::map<IXAudio2Voice*, std::shared_ptr<IXAudio2SubmixVoice>> tailVoices;

	UINT32 actualProcessingStage;
	UINT32 inputChannelsCount;
	UINT32 inputSampleRate;
	UINT32 mainOutputChannelsCount;
};

class AudioGraphMapper
{
public:
	// AudioGraphMapper does not own xaudio
	AudioGraphMapper(IXAudio2 * xaudio);
	AudioGraphMapper(const AudioGraphMapper &) = delete;
	AudioGraphMapper& operator=(const AudioGraphMapper& other) = delete;

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
	std::map<IXAudio2Voice*, std::unique_ptr<Node>> m_nodes;
	Node * m_masteringNode;
	EdgeRepository<Node*> m_edges;

	static XAUDIO2_VOICE_SENDS _emptySends;

	VoiceSends mapProxySendsToActualOnes(const VoiceSends & proxySends) const;
	const Node * getNodeForProxyVoice(IXAudio2Voice* pDestination) const;
	Node * getNodeForProxyVoice(IXAudio2Voice* pDestination);
	void setupCommonCallbacks(XAudio2VoiceProxy* proxyVoice, const std::shared_ptr<IXAudio2Voice> & actualVoice);
	void updateSendsForVoice(XAudio2VoiceProxy* proxyVoice);
};