#include "stdafx.h"
#include "XAudio2SourceVoiceProxy.h"

#include "XAudio2Proxy.h"
#include "wave/WaveFile.h"
#include "XAPO/HrtfEffect.h"
#include <XAPO.h>

#include "util.h"
#include "logger.h"

//#define DUMP_SOUND_WAV

std::wstring GetName(const XAudio2SourceVoiceProxy * ptr)
{
	std::wstringstream ss;
	ss << ptr;
	return ss.str();
}

XAudio2SourceVoiceProxy::XAudio2SourceVoiceProxy(ISound3DRegistry * sound3d_registry, const IVoiceMapper & voice_mapper, IXAudio2SourceVoice * original, UINT32 input_channels, const XAUDIO2_EFFECT_CHAIN * original_chain, const deleter & on_destroy)
	: m_impl(L"XAudio2SourceVoiceProxy", sound3d_registry, voice_mapper, original, input_channels, this, original_chain)
	, m_original(original)
	, m_sound3d_registry(sound3d_registry)
	, m_voice_mapper(voice_mapper)
	, m_on_destroy(on_destroy)
{
#ifdef DUMP_SOUND_WAV
	XAUDIO2_VOICE_DETAILS details;
	original->GetVoiceDetails(&details);
	m_wave_file.reset(new WaveFile(std::wstring(L"wavs\\") + std::to_wstring(input_channels) + L"_"  + GetName(this) + L".wav", details.InputChannels, details.InputSampleRate, 16));
#endif
}

XAudio2SourceVoiceProxy::~XAudio2SourceVoiceProxy()
{
	m_original->DestroyVoice();
}

HRESULT XAudio2SourceVoiceProxy::Start(UINT32 Flags, UINT32 OperationSet)
{
	return m_original->Start(Flags, OperationSet);
}

HRESULT XAudio2SourceVoiceProxy::Stop(UINT32 Flags, UINT32 OperationSet)
{
	return m_original->Stop(Flags, OperationSet);
}

HRESULT XAudio2SourceVoiceProxy::SubmitSourceBuffer(const XAUDIO2_BUFFER * pBuffer, const XAUDIO2_BUFFER_WMA *pBufferWMA)
{
#ifdef DUMP_SOUND_WAV
	m_wave_file->AppendData(pBuffer->pAudioData, pBuffer->AudioBytes);
#endif
	return m_original->SubmitSourceBuffer(pBuffer, pBufferWMA);
}

HRESULT XAudio2SourceVoiceProxy::FlushSourceBuffers()
{
	return m_original->FlushSourceBuffers();
}

HRESULT XAudio2SourceVoiceProxy::Discontinuity()
{
	return m_original->Discontinuity();
}

HRESULT XAudio2SourceVoiceProxy::ExitLoop(UINT32 OperationSet)
{
	return m_original->ExitLoop(OperationSet);
}

void XAudio2SourceVoiceProxy::GetState(XAUDIO2_VOICE_STATE * pVoiceState)
{
	m_original->GetState(pVoiceState);
}

HRESULT XAudio2SourceVoiceProxy::SetFrequencyRatio(float Ratio, UINT32 OperationSet)
{
	return m_original->SetFrequencyRatio(Ratio, OperationSet);
}

void XAudio2SourceVoiceProxy::GetFrequencyRatio(float * pRatio)
{
	m_original->GetFrequencyRatio(pRatio);
}

HRESULT XAudio2SourceVoiceProxy::SetSourceSampleRate(UINT32 NewSourceSampleRate)
{
	return m_original->SetSourceSampleRate(NewSourceSampleRate);
}

// End Source-specific

// Common part

void XAudio2SourceVoiceProxy::GetVoiceDetails(XAUDIO2_VOICE_DETAILS *pVoiceDetails)
{
	m_impl.GetVoiceDetails(pVoiceDetails);
}

HRESULT XAudio2SourceVoiceProxy::SetOutputVoices(const XAUDIO2_VOICE_SENDS *pSendList)
{
	return m_impl.SetOutputVoices(pSendList );
}

HRESULT XAudio2SourceVoiceProxy::SetEffectChain(const XAUDIO2_EFFECT_CHAIN *pEffectChain)
{
	return m_impl.SetEffectChain(pEffectChain);
}

HRESULT XAudio2SourceVoiceProxy::EnableEffect(UINT32 EffectIndex, UINT32 OperationSet)
{
	return m_impl.EnableEffect(EffectIndex, OperationSet);
}

HRESULT XAudio2SourceVoiceProxy::DisableEffect(UINT32 EffectIndex, UINT32 OperationSet)
{
	return m_impl.DisableEffect(EffectIndex, OperationSet);
}

void XAudio2SourceVoiceProxy::GetEffectState(UINT32 EffectIndex, BOOL *pEnabled)
{
	m_impl.GetEffectState(EffectIndex, pEnabled);
}

HRESULT XAudio2SourceVoiceProxy::SetEffectParameters(UINT32 EffectIndex, const void *pParameters, UINT32 ParametersByteSize, UINT32 OperationSet)
{
	return m_impl.SetEffectParameters(EffectIndex, pParameters, ParametersByteSize, OperationSet);
}

HRESULT XAudio2SourceVoiceProxy::GetEffectParameters(UINT32 EffectIndex, void *pParameters, UINT32 ParametersByteSize)
{
	return m_impl.GetEffectParameters(EffectIndex, pParameters, ParametersByteSize);
}

HRESULT XAudio2SourceVoiceProxy::SetFilterParameters(const XAUDIO2_FILTER_PARAMETERS *pParameters, UINT32 OperationSet)
{
	return m_impl.SetFilterParameters(pParameters, OperationSet);
}

void XAudio2SourceVoiceProxy::GetFilterParameters(XAUDIO2_FILTER_PARAMETERS *pParameters)
{
	m_impl.GetFilterParameters(pParameters);
}

HRESULT XAudio2SourceVoiceProxy::SetOutputFilterParameters(IXAudio2Voice *pDestinationVoice, const XAUDIO2_FILTER_PARAMETERS *pParameters, UINT32 OperationSet)
{
	return m_impl.SetOutputFilterParameters(m_voice_mapper.MapVoiceToOriginal(pDestinationVoice), pParameters, OperationSet);
}

void XAudio2SourceVoiceProxy::GetOutputFilterParameters(IXAudio2Voice *pDestinationVoice, XAUDIO2_FILTER_PARAMETERS *pParameters)
{
	m_impl.GetOutputFilterParameters(m_voice_mapper.MapVoiceToOriginal(pDestinationVoice), pParameters);
}

HRESULT XAudio2SourceVoiceProxy::SetVolume(float Volume, UINT32 OperationSet)
{
	return m_impl.SetVolume(Volume, OperationSet);
}

void XAudio2SourceVoiceProxy::GetVolume(float *pVolume)
{
	m_impl.GetVolume(pVolume);
}

HRESULT XAudio2SourceVoiceProxy::SetChannelVolumes(UINT32 Channels, const float *pVolumes, UINT32 OperationSet)
{
	return m_impl.SetChannelVolumes(Channels, pVolumes, OperationSet);
}

void XAudio2SourceVoiceProxy::GetChannelVolumes(UINT32 Channels, float *pVolumes)
{
	m_impl.GetChannelVolumes(Channels, pVolumes);
}

HRESULT XAudio2SourceVoiceProxy::SetOutputMatrix(IXAudio2Voice *pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, const float *pLevelMatrix, UINT32 OperationSet)
{
	return m_impl.SetOutputMatrix(m_voice_mapper.MapVoiceToOriginal(pDestinationVoice), SourceChannels, DestinationChannels, pLevelMatrix, OperationSet);
}

void XAudio2SourceVoiceProxy::GetOutputMatrix(IXAudio2Voice *pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, float *pLevelMatrix)
{
	m_impl.GetOutputMatrix(m_voice_mapper.MapVoiceToOriginal(pDestinationVoice), SourceChannels, DestinationChannels, pLevelMatrix);
}

void XAudio2SourceVoiceProxy::DestroyVoice()
{
	std::wstringstream ss;
	ss << "XAudio2SourceVoiceProxy::DestroyVoice " << this;
	logger::log(ss.str());
	m_on_destroy(this);
}