#pragma once

#include <xaudio2.h>
#include <vector>

#include "XAudio2VoiceProxy.h"

class XAudio2SubmixVoiceProxy : public IXAudio2SubmixVoice, public XAudio2VoiceProxy
{
public:
	XAudio2SubmixVoiceProxy(UINT32 inputChannels, UINT32 inputSampleRate, UINT32 flags, UINT32 processingStage, const VoiceSends & sends, const effect_chain & effectChain);
	virtual ~XAudio2SubmixVoiceProxy();


	// Inherited via IXAudio2SubmixVoice
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

	const IXAudio2Voice* asXAudio2Voice() const override { return this; }
	IXAudio2Voice* asXAudio2Voice() override { return this; }
};
