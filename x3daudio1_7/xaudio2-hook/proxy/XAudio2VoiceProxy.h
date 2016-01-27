#pragma once

#include <xaudio2.h>

#include <functional>
#include <vector>
#include <map>
#include <cstdint>
#include "ChannelMatrix.h"


class XAudio2VoiceProxy
{
public:
	typedef std::map<IXAudio2Voice *, ChannelMatrix> matrices_map;
	typedef std::map<IXAudio2Voice*, XAUDIO2_FILTER_PARAMETERS> filters_map;

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
	typedef std::function<void(XAudio2VoiceProxy *, matrices_map &, filters_map &)> sends_voice_callback;


	XAudio2VoiceProxy(UINT32 inputChannels, UINT32 inputSampleRate, UINT32 flags, UINT32 processingStage, const std::vector<XAUDIO2_SEND_DESCRIPTOR> & sends, const std::vector<XAUDIO2_EFFECT_DESCRIPTOR> & effectChain);
	XAudio2VoiceProxy(const XAudio2VoiceProxy &) = delete;
	virtual ~XAudio2VoiceProxy();

	UINT32 getInputChannels() const;
	UINT32 getInputSampleRate() const;
	UINT32 getFlags() const;
	void setOutputVoices(const std::vector<XAUDIO2_SEND_DESCRIPTOR> & sends);
	void setEffectChain(const std::vector<XAUDIO2_EFFECT_DESCRIPTOR> & chain);
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
	std::vector<XAUDIO2_SEND_DESCRIPTOR> m_sends;
	std::vector<XAUDIO2_EFFECT_DESCRIPTOR> m_effectChain;
	std::vector<bool> m_effectActivity;
	std::vector<std::vector<int8_t>> m_effectParameters;
	XAUDIO2_FILTER_PARAMETERS m_filterParameters;
	filters_map m_outputFilterParameters;
	float m_volume;
	std::vector<float> m_channelVolumes;
	matrices_map m_outputMatrices;
};
