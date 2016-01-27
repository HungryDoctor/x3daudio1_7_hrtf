#pragma once

#include "XAudio2VoiceProxy.h"

#include <xaudio2.h>

class XAudio2MasteringVoiceProxy : public IXAudio2MasteringVoice, public XAudio2VoiceProxy
{
public:
	

	XAudio2MasteringVoiceProxy(UINT32 inputChannels, UINT32 inputSampleRate, UINT32 Flags, UINT32 deviceIndex, const std::vector<XAUDIO2_EFFECT_DESCRIPTOR> & effectChain);
	virtual ~XAudio2MasteringVoiceProxy();

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

	UINT32 getDeviceIndex() const
	{
		return m_deviceIndex;
	}

private:
	const UINT32 m_deviceIndex;
};
