#pragma once

#include <xaudio2.h>

#include <functional>
#include <vector>
#include <map>
#include <cstdint>

#include "common_types.h"


class XAudio2VoiceProxy
{
public:
	

	// void (eventSource)
	typedef std::function<void(XAudio2VoiceProxy *)> immediate_voice_callback;

	// void (eventSource, operationSet)
	typedef std::function<void(XAudio2VoiceProxy *, UINT32)> voice_callback;

	// void (eventSource, effectIndex, operationSet)
	typedef std::function<void(XAudio2VoiceProxy *, UINT32, UINT32)> effect_voice_callback;

	// void (eventSource, pDestinationVoice, operationSet)
	typedef std::function<void(XAudio2VoiceProxy *, IXAudio2Voice *, UINT32)> filter_voice_callback;

	// void (eventSource, pDestinationVoice, operationSet)
	typedef std::function<void(XAudio2VoiceProxy *, IXAudio2Voice *, UINT32)> matrix_voice_callback;

	// void (eventSource, [ref] outputMatrices, [ref] outputFilterParams)
	typedef std::function<void(XAudio2VoiceProxy *, matrices_map &, filter_params_map &)> sends_voice_callback;


	XAudio2VoiceProxy(UINT32 inputChannels, UINT32 inputSampleRate, UINT32 flags, UINT32 processingStage, const VoiceSends & sends, const effect_chain & effectChain);
	XAudio2VoiceProxy(const XAudio2VoiceProxy &) = delete;
	virtual ~XAudio2VoiceProxy();

	UINT32 getInputChannels() const;
	UINT32 getInputSampleRate() const;
	UINT32 getFlags() const;
	void setOutputVoices(const VoiceSends & sends);
	void internalSetEffectChain(const effect_chain& chain);
	void setEffectChain(const effect_chain & chain);
	void setEffectEnabled(UINT32 effectIndex, bool isEnabled, UINT32 operationSet);
	bool getIsEffectEnabled(UINT32 effectIndex) const;
	void setEffectParameters(UINT32 effectIndex, const std::vector<int8_t> & parameters, UINT32 operationSet);
	const std::vector<int8_t> & getEffectParameters(UINT32 effectIndex) const;
	void setFilterParameters(const XAUDIO2_FILTER_PARAMETERS & parameters, UINT32 operationSet);
	XAUDIO2_FILTER_PARAMETERS getFilterParameters() const;
	void setOutputFilterParameters(IXAudio2Voice * pDestinationVoice, const XAUDIO2_FILTER_PARAMETERS & parameters, UINT32 operationSet);
	XAUDIO2_FILTER_PARAMETERS getOutputFilterParameters(IXAudio2Voice * pDestinationVoice) const;
	void setVolume(float volume, UINT32 operationSet);
	float getVolume() const;
	void setChannelVolumes(const std::vector<float> & volumes, UINT32 operationSet);
	std::vector<float> getChannelVolumes() const;
	void setOutputMatrix(IXAudio2Voice * pDestinationVoice, const ChannelMatrix & matrix, UINT32 operationSet);
	ChannelMatrix getOutputMatrix(IXAudio2Voice * pDestinationVoice) const;

	UINT32 getProcessingStage() const
	{
		return m_processingStage;
	}

	const VoiceSends & getSends() const
	{
		return m_sends;
	}

	const effect_chain & getEffectChain() const
	{
		return m_effectChain;
	}

	void destroyVoice();

	sends_voice_callback onSetOutputVoices;
	immediate_voice_callback onSetEffectChain;
	effect_voice_callback onSetEffectEnabled;
	effect_voice_callback onSetEffectParameters;
	voice_callback onSetFilterParameters;
	filter_voice_callback onSetOutputFilterParameters;
	voice_callback onSetVolume;
	voice_callback onSetChannelVolumes;
	matrix_voice_callback onSetOutputMatrix;
	immediate_voice_callback onDestroyVoice;
	
protected:
	UINT32 m_inputSampleRate;
private:
	const UINT32 m_inputChannels;
	const UINT32 m_flags;
	const UINT32 m_processingStage;
	VoiceSends m_sends;

	effect_chain m_effectChain;
	std::vector<bool> m_effectActivity;
	std::vector<std::vector<int8_t>> m_effectParameters;
	XAUDIO2_FILTER_PARAMETERS m_filterParameters;
	filter_params_map m_outputFilterParameters;
	float m_volume;
	std::vector<float> m_channelVolumes;
	matrices_map m_outputMatrices;
};
