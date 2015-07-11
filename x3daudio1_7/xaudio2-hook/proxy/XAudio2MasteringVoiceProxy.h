#pragma once

#include "XAudio2VoiceProxy.h"
#include <xaudio2.h>
#include "IVoiceMapper.h"
#include <memory>
#include <functional>

class XAudio2MasteringVoiceProxy : public IXAudio2MasteringVoice
{
public:
	typedef std::function<void(XAudio2MasteringVoiceProxy*)> deleter;

public:
	XAudio2MasteringVoiceProxy(IXAudio2 * original_xaudio, IVoiceMapper * voice_mapper, const deleter & on_destroy,
		UINT32 InputChannels, UINT32 InputSampleRate, UINT32 Flags, UINT32 DeviceIndex, const XAUDIO2_EFFECT_CHAIN *pEffectChain);
	virtual ~XAudio2MasteringVoiceProxy();

public:
	// Inherited via IXAudio2SubmixVoice
	STDMETHOD_(void, GetVoiceDetails)(XAUDIO2_VOICE_DETAILS *pVoiceDetails) override;
	STDMETHOD(SetOutputVoices)(const XAUDIO2_VOICE_SENDS *pSendList) override;
	STDMETHOD(SetEffectChain)(const XAUDIO2_EFFECT_CHAIN *pEffectChain) override;
	STDMETHOD(EnableEffect)(UINT32 EffectIndex, UINT32 OperationSet = 0U) override;
	STDMETHOD(DisableEffect)(UINT32 EffectIndex, UINT32 OperationSet = 0U) override;
	STDMETHOD_(void, GetEffectState)(UINT32 EffectIndex, BOOL *pEnabled) override;
	STDMETHOD(SetEffectParameters)(UINT32 EffectIndex, const void *pParameters, UINT32 ParametersByteSize, UINT32 OperationSet = 0U) override;
	STDMETHOD(GetEffectParameters)(UINT32 EffectIndex, void *pParameters, UINT32 ParametersByteSize) override;
	STDMETHOD(SetFilterParameters)(const XAUDIO2_FILTER_PARAMETERS *pParameters, UINT32 OperationSet = 0U) override;
	STDMETHOD_(void, GetFilterParameters)(XAUDIO2_FILTER_PARAMETERS *pParameters) override;
	STDMETHOD(SetOutputFilterParameters)(IXAudio2Voice *pDestinationVoice, const XAUDIO2_FILTER_PARAMETERS *pParameters, UINT32 OperationSet = 0U) override;
	STDMETHOD_(void, GetOutputFilterParameters)(IXAudio2Voice *pDestinationVoice, XAUDIO2_FILTER_PARAMETERS *pParameters) override;
	STDMETHOD(SetVolume)(float Volume, UINT32 OperationSet = 0U) override;
	STDMETHOD_(void, GetVolume)(float *pVolume) override;
	STDMETHOD(SetChannelVolumes)(UINT32 Channels, const float *pVolumes, UINT32 OperationSet = 0U) override;
	STDMETHOD_(void, GetChannelVolumes)(UINT32 Channels, float *pVolumes) override;
	STDMETHOD(SetOutputMatrix)(IXAudio2Voice *pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, const float *pLevelMatrix, UINT32 OperationSet = 0U) override;
	STDMETHOD_(void, GetOutputMatrix)(IXAudio2Voice *pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, float *pLevelMatrix) override;
	STDMETHOD_(void, DestroyVoice)() override;

private:
	std::unique_ptr<XAudio2VoiceProxy> m_impl;
	IXAudio2MasteringVoice * m_original;
	IVoiceMapper * m_voice_mapper;
	deleter m_on_destroy;

	XAudio2MasteringVoiceProxy(const XAudio2MasteringVoiceProxy &) = delete;
	XAudio2MasteringVoiceProxy & operator=(const XAudio2MasteringVoiceProxy &) = delete;
};