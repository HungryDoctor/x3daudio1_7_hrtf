#pragma once

#include <xaudio2.h>
#include <memory>
#include <string>

#include "interop/ISound3dRegistry.h"
#include "IVoiceMapper.h"

class XAudio2VoiceProxy
{
public:
	XAudio2VoiceProxy(const std::wstring & type_name, ISound3DRegistry * sound3d_registry, const IVoiceMapper & voice_mapper, IXAudio2Voice * original, const void * id);
	XAudio2VoiceProxy(const std::wstring & type_name, ISound3DRegistry * sound3d_registry, const IVoiceMapper & voice_mapper, IXAudio2Voice * original, UINT32 input_channels, const void * id, const XAUDIO2_EFFECT_CHAIN * original_chain);

public:
	// Inherited via IXAudio2SubmixVoice
	void GetVoiceDetails(XAUDIO2_VOICE_DETAILS *pVoiceDetails);
	HRESULT SetOutputVoices(const XAUDIO2_VOICE_SENDS *pSendList);
	HRESULT SetEffectChain(const XAUDIO2_EFFECT_CHAIN *pEffectChain);
	HRESULT EnableEffect(UINT32 EffectIndex, UINT32 OperationSet = 0U);
	HRESULT DisableEffect(UINT32 EffectIndex, UINT32 OperationSet = 0U);
	void GetEffectState(UINT32 EffectIndex, BOOL *pEnabled);
	HRESULT SetEffectParameters(UINT32 EffectIndex, const void *pParameters, UINT32 ParametersByteSize, UINT32 OperationSet = 0U);
	HRESULT GetEffectParameters(UINT32 EffectIndex, void *pParameters, UINT32 ParametersByteSize);
	HRESULT SetFilterParameters(const XAUDIO2_FILTER_PARAMETERS *pParameters, UINT32 OperationSet = 0U);
	void GetFilterParameters(XAUDIO2_FILTER_PARAMETERS *pParameters);
	HRESULT SetOutputFilterParameters(IXAudio2Voice *pDestinationVoice, const XAUDIO2_FILTER_PARAMETERS *pParameters, UINT32 OperationSet = 0U);
	void GetOutputFilterParameters(IXAudio2Voice *pDestinationVoice, XAUDIO2_FILTER_PARAMETERS *pParameters);
	HRESULT SetVolume(float Volume, UINT32 OperationSet = 0U);
	void GetVolume(float *pVolume);
	HRESULT SetChannelVolumes(UINT32 Channels, const float *pVolumes, UINT32 OperationSet = 0U);
	void GetChannelVolumes(UINT32 Channels, float *pVolumes);
	HRESULT SetOutputMatrix(IXAudio2Voice *pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, const float *pLevelMatrix, UINT32 OperationSet = 0U);
	void GetOutputMatrix(IXAudio2Voice *pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, float *pLevelMatrix);

protected:
	HRESULT AlterAndSetEffectChain(const XAUDIO2_EFFECT_CHAIN * original_chain);

private:
	IXAudio2Voice * m_original;
	const void * m_id;
	const std::wstring m_type_name;
	ISound3DRegistry const * m_sound3d_registry;
	const IVoiceMapper & m_voice_mapper;
	const UINT32 m_input_channels;
	int m_output_channels;
	UINT32 m_hrtf_effect_index;
	const bool m_is_master;


	XAudio2VoiceProxy(const XAudio2VoiceProxy &) = delete;
	XAudio2VoiceProxy & operator=(const XAudio2VoiceProxy &) = delete;
};
