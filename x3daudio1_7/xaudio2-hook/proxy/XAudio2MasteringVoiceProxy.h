#pragma once

#include <xaudio2.h>
#include "IVoiceMapper.h"
#include <memory>
#include <functional>

class XAudio2MasteringVoiceProxy : public IXAudio2MasteringVoice
{
public:
	typedef std::function<void(XAudio2MasteringVoiceProxy*)> deleter;

public:
	XAudio2MasteringVoiceProxy(const IVoiceMapper & voice_mapper, IXAudio2MasteringVoice * original, const deleter & on_destroy);
	virtual ~XAudio2MasteringVoiceProxy();

public:
	// Inherited via IXAudio2SubmixVoice
	STDMETHOD_(void, GetVoiceDetails)(XAUDIO2_VOICE_DETAILS *pVoiceDetails);
	STDMETHOD(SetOutputVoices)(const XAUDIO2_VOICE_SENDS *pSendList);
	STDMETHOD(SetEffectChain)(const XAUDIO2_EFFECT_CHAIN *pEffectChain);
	STDMETHOD(EnableEffect)(UINT32 EffectIndex, UINT32 OperationSet = 0U);
	STDMETHOD(DisableEffect)(UINT32 EffectIndex, UINT32 OperationSet = 0U);
	STDMETHOD_(void, GetEffectState)(UINT32 EffectIndex, BOOL *pEnabled);
	STDMETHOD(SetEffectParameters)(UINT32 EffectIndex, const void *pParameters, UINT32 ParametersByteSize, UINT32 OperationSet = 0U);
	STDMETHOD(GetEffectParameters)(UINT32 EffectIndex, void *pParameters, UINT32 ParametersByteSize);
	STDMETHOD(SetFilterParameters)(const XAUDIO2_FILTER_PARAMETERS *pParameters, UINT32 OperationSet = 0U);
	STDMETHOD_(void, GetFilterParameters)(XAUDIO2_FILTER_PARAMETERS *pParameters);
	STDMETHOD(SetOutputFilterParameters)(IXAudio2Voice *pDestinationVoice, const XAUDIO2_FILTER_PARAMETERS *pParameters, UINT32 OperationSet = 0U);
	STDMETHOD_(void, GetOutputFilterParameters)(IXAudio2Voice *pDestinationVoice, XAUDIO2_FILTER_PARAMETERS *pParameters);
	STDMETHOD(SetVolume)(float Volume, UINT32 OperationSet = 0U);
	STDMETHOD_(void, GetVolume)(float *pVolume);
	STDMETHOD(SetChannelVolumes)(UINT32 Channels, const float *pVolumes, UINT32 OperationSet = 0U);
	STDMETHOD_(void, GetChannelVolumes)(UINT32 Channels, float *pVolumes);
	STDMETHOD(SetOutputMatrix)(IXAudio2Voice *pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, const float *pLevelMatrix, UINT32 OperationSet = 0U);
	STDMETHOD_(void, GetOutputMatrix)(IXAudio2Voice *pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, float *pLevelMatrix);
	STDMETHOD_(void, DestroyVoice)();

private:
	IXAudio2MasteringVoice * m_original;
	const IVoiceMapper & m_voice_mapper;
	deleter m_on_destroy;

	XAudio2MasteringVoiceProxy(const XAudio2MasteringVoiceProxy &) = delete;
	XAudio2MasteringVoiceProxy & operator=(const XAudio2MasteringVoiceProxy &) = delete;
};