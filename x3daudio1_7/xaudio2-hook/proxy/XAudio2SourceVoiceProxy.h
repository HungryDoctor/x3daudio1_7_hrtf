#pragma once

#include "XAudio2VoiceProxy.h"

#include <xaudio2.h>
#include <functional>
#include <vector>

class XAudio2SourceVoiceProxy : public IXAudio2SourceVoice, public XAudio2VoiceProxy
{
public:
	// void (eventSource, operationSet)
	typedef std::function<void(XAudio2SourceVoiceProxy *, UINT32)> source_voice_callback;

	// void (eventSource)
	typedef std::function<void(XAudio2SourceVoiceProxy *)> immediate_source_voice_callback;

	// pVoice, pBuffer, pBufferWMA
	typedef std::function<void(XAudio2SourceVoiceProxy *, const XAUDIO2_BUFFER *, const XAUDIO2_BUFFER_WMA *)> submit_buffer_voice_callback;

	// pVoice, flags, operationSet
	typedef std::function<void(XAudio2SourceVoiceProxy *, UINT32, UINT32)> start_stop_voice_callback;

	// pVoice, out_state
	typedef std::function<void(XAudio2SourceVoiceProxy *, XAUDIO2_VOICE_STATE &)> state_getter;


public:
	XAudio2SourceVoiceProxy(UINT32 inputChannels, UINT32 inputSampleRate, UINT32 flags, float maxFrequencyRatio, IXAudio2VoiceCallback * pCallback, UINT32 processingStage, const VoiceSends & sends, const effect_chain & effect);
	virtual ~XAudio2SourceVoiceProxy();

public:
	// Inherited from IXAudio2SourceVoice
	STDMETHOD(Start)(UINT32 Flags X2DEFAULT(0), UINT32 OperationSet X2DEFAULT(XAUDIO2_COMMIT_NOW)) override;
	STDMETHOD(Stop)(UINT32 Flags X2DEFAULT(0), UINT32 OperationSet X2DEFAULT(XAUDIO2_COMMIT_NOW)) override;
	STDMETHOD(SubmitSourceBuffer)(const XAUDIO2_BUFFER * pBuffer, const XAUDIO2_BUFFER_WMA * pBufferWMA X2DEFAULT(NULL)) override;
	STDMETHOD(FlushSourceBuffers)(void) override;
	STDMETHOD(Discontinuity)(void) override;
	STDMETHOD(ExitLoop)(UINT32 OperationSet X2DEFAULT(XAUDIO2_COMMIT_NOW)) override;
	STDMETHOD_(void, GetState)(XAUDIO2_VOICE_STATE * pVoiceState) override;
	STDMETHOD(SetFrequencyRatio)(float Ratio, UINT32 OperationSet X2DEFAULT(XAUDIO2_COMMIT_NOW)) override;
	STDMETHOD_(void, GetFrequencyRatio)(float * pRatio) override;
	STDMETHOD(SetSourceSampleRate)(UINT32 NewSourceSampleRate) override;

	// Inherited from IXAudio2Voice
	STDMETHOD_(void, GetVoiceDetails)(XAUDIO2_VOICE_DETAILS * pVoiceDetails) override;
	STDMETHOD(SetOutputVoices)(const XAUDIO2_VOICE_SENDS * pSendList) override;
	STDMETHOD(SetEffectChain)(const XAUDIO2_EFFECT_CHAIN * pEffectChain) override;
	STDMETHOD(EnableEffect)(UINT32 EffectIndex, UINT32 OperationSet = 0U) override;
	STDMETHOD(DisableEffect)(UINT32 EffectIndex, UINT32 OperationSet = 0U) override;
	STDMETHOD_(void, GetEffectState)(UINT32 EffectIndex, BOOL * pEnabled) override;
	STDMETHOD(SetEffectParameters)(UINT32 EffectIndex, const void * pParameters, UINT32 ParametersByteSize, UINT32 OperationSet = 0U) override;
	STDMETHOD(GetEffectParameters)(UINT32 EffectIndex, void * pParameters, UINT32 ParametersByteSize) override;
	STDMETHOD(SetFilterParameters)(const XAUDIO2_FILTER_PARAMETERS * pParameters, UINT32 OperationSet = 0U) override;
	STDMETHOD_(void, GetFilterParameters)(XAUDIO2_FILTER_PARAMETERS * pParameters) override;
	STDMETHOD(SetOutputFilterParameters)(IXAudio2Voice * pDestinationVoice, const XAUDIO2_FILTER_PARAMETERS * pParameters, UINT32 OperationSet = 0U) override;
	STDMETHOD_(void, GetOutputFilterParameters)(IXAudio2Voice * pDestinationVoice, XAUDIO2_FILTER_PARAMETERS * pParameters) override;
	STDMETHOD(SetVolume)(float Volume, UINT32 OperationSet = 0U) override;
	STDMETHOD_(void, GetVolume)(float * pVolume) override;
	STDMETHOD(SetChannelVolumes)(UINT32 Channels, const float * pVolumes, UINT32 OperationSet = 0U) override;
	STDMETHOD_(void, GetChannelVolumes)(UINT32 Channels, float * pVolumes) override;
	STDMETHOD(SetOutputMatrix)(IXAudio2Voice * pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, const float * pLevelMatrix, UINT32 OperationSet = 0U) override;
	STDMETHOD_(void, GetOutputMatrix)(IXAudio2Voice * pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, float * pLevelMatrix) override;
	STDMETHOD_(void, DestroyVoice)() override;

	float getMaxFrequencyRatio() const
	{
		return m_maxFrequencyRatio;
	}

	float getFrequencyRatio() const
	{
		return m_frequencyRatio;
	}

	IXAudio2VoiceCallback* getVoiceCallback() const
	{
		return m_voiceCallback;
	}

	const IXAudio2Voice* asXAudio2Voice() const override { return this; }
	IXAudio2Voice* asXAudio2Voice() override { return this; }

	state_getter stateGetter;
	start_stop_voice_callback onStart;
	start_stop_voice_callback onStop;
	submit_buffer_voice_callback onSubmitSourceBuffer;
	immediate_source_voice_callback onFlushSourceBuffers;
	immediate_source_voice_callback onDiscontinuity;
	source_voice_callback onExitLoop;
	source_voice_callback onSetFrequencyRatio;
	immediate_source_voice_callback onSetSourceSampleRate;


private:
	const float m_maxFrequencyRatio;
	float m_frequencyRatio;
	IXAudio2VoiceCallback * m_voiceCallback;
};
