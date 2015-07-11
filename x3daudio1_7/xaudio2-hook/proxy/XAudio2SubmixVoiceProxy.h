#pragma once

#include "XAudio2VoiceProxy.h"
#include <xaudio2.h>
#include <memory>
#include <functional>

#include "interop/ISound3dRegistry.h"
#include "IVoiceMapper.h"

class XAudio2SubmixVoiceProxy : public IXAudio2SubmixVoice
{
public:
	typedef std::function<void(XAudio2SubmixVoiceProxy*)> deleter;

public:
	XAudio2SubmixVoiceProxy(IXAudio2 * original_xaudio, ISound3DRegistry * sound3d_registry, IVoiceMapper * voice_mapper, const deleter & on_destroy,
		UINT32 InputChannels, UINT32 InputSampleRate, UINT32 Flags, UINT32 ProcessingStage, const XAUDIO2_VOICE_SENDS *pSendList, const XAUDIO2_EFFECT_CHAIN *pEffectChain);
	virtual ~XAudio2SubmixVoiceProxy();

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
	std::unique_ptr< XAudio2VoiceProxy> m_impl;
	IXAudio2SubmixVoice * m_original;
	ISound3DRegistry * m_sound3d_registry;
	IVoiceMapper * m_voice_mapper;
	deleter m_on_destroy;

	XAudio2SubmixVoiceProxy(const XAudio2SubmixVoiceProxy &) = delete;
	XAudio2SubmixVoiceProxy & operator=(const XAudio2SubmixVoiceProxy &) = delete;
};
