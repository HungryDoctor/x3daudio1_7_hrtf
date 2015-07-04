#include "stdafx.h"
#include "XAudio2SourceVoiceProxy.h"

#include "XAudio2Proxy.h"

#include "util.h"
#include "logger.h"

XAudio2SourceVoiceProxy::XAudio2SourceVoiceProxy(const IVoiceMapper & voice_mapper, IXAudio2SourceVoice * original, const deleter & on_destroy)
	: m_voice_mapper(voice_mapper)
	, m_original(original)
	, m_on_destroy(on_destroy)
{

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

void XAudio2SourceVoiceProxy::GetVoiceDetails(XAUDIO2_VOICE_DETAILS *pVoiceDetails)
{
	m_original->GetVoiceDetails(pVoiceDetails);
}

HRESULT XAudio2SourceVoiceProxy::SetOutputVoices(const XAUDIO2_VOICE_SENDS *pSendList)
{
	std::stringstream ss;
	ss << "XAudio2SourceVoiceProxy::SetOutputVoices " << this << " ";
	print_sends(ss, pSendList);
	logger::log(ss.str());

	XAUDIO2_VOICE_SENDS originalSendList = { 0 };
	if (pSendList)
		m_voice_mapper.MapSendsToOriginal(*pSendList, originalSendList);

	auto result = m_original->SetOutputVoices(pSendList ? &originalSendList : 0);

	m_voice_mapper.CleanupSends(originalSendList);

	return result;
}

HRESULT XAudio2SourceVoiceProxy::SetEffectChain(const XAUDIO2_EFFECT_CHAIN *pEffectChain)
{
	return m_original->SetEffectChain(pEffectChain);
}

HRESULT XAudio2SourceVoiceProxy::EnableEffect(UINT32 EffectIndex, UINT32 OperationSet)
{
	return m_original->EnableEffect(EffectIndex, OperationSet);
}

HRESULT XAudio2SourceVoiceProxy::DisableEffect(UINT32 EffectIndex, UINT32 OperationSet)
{
	return m_original->DisableEffect(EffectIndex, OperationSet);
}

void XAudio2SourceVoiceProxy::GetEffectState(UINT32 EffectIndex, BOOL *pEnabled)
{
	m_original->GetEffectState(EffectIndex, pEnabled);
}

HRESULT XAudio2SourceVoiceProxy::SetEffectParameters(UINT32 EffectIndex, const void *pParameters, UINT32 ParametersByteSize, UINT32 OperationSet)
{
	return m_original->SetEffectParameters(EffectIndex, pParameters, ParametersByteSize, OperationSet);
}

HRESULT XAudio2SourceVoiceProxy::GetEffectParameters(UINT32 EffectIndex, void *pParameters, UINT32 ParametersByteSize)
{
	return m_original->GetEffectParameters(EffectIndex, pParameters, ParametersByteSize);
}

HRESULT XAudio2SourceVoiceProxy::SetFilterParameters(const XAUDIO2_FILTER_PARAMETERS *pParameters, UINT32 OperationSet)
{
	return m_original->SetFilterParameters(pParameters, OperationSet);
}

void XAudio2SourceVoiceProxy::GetFilterParameters(XAUDIO2_FILTER_PARAMETERS *pParameters)
{
	m_original->GetFilterParameters(pParameters);
}

HRESULT XAudio2SourceVoiceProxy::SetOutputFilterParameters(IXAudio2Voice *pDestinationVoice, const XAUDIO2_FILTER_PARAMETERS *pParameters, UINT32 OperationSet)
{
	return m_original->SetOutputFilterParameters(m_voice_mapper.MapVoiceToOriginal(pDestinationVoice), pParameters, OperationSet);
}

void XAudio2SourceVoiceProxy::GetOutputFilterParameters(IXAudio2Voice *pDestinationVoice, XAUDIO2_FILTER_PARAMETERS *pParameters)
{
	m_original->GetOutputFilterParameters(m_voice_mapper.MapVoiceToOriginal(pDestinationVoice), pParameters);
}

HRESULT XAudio2SourceVoiceProxy::SetVolume(float Volume, UINT32 OperationSet)
{
	return m_original->SetVolume(Volume, OperationSet);
}

void XAudio2SourceVoiceProxy::GetVolume(float *pVolume)
{
	m_original->GetVolume(pVolume);
}

HRESULT XAudio2SourceVoiceProxy::SetChannelVolumes(UINT32 Channels, const float *pVolumes, UINT32 OperationSet)
{
	return m_original->SetChannelVolumes(Channels, pVolumes, OperationSet);
}

void XAudio2SourceVoiceProxy::GetChannelVolumes(UINT32 Channels, float *pVolumes)
{
	m_original->GetChannelVolumes(Channels, pVolumes);
}

HRESULT XAudio2SourceVoiceProxy::SetOutputMatrix(IXAudio2Voice *pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, const float *pLevelMatrix, UINT32 OperationSet)
{
	return m_original->SetOutputMatrix(m_voice_mapper.MapVoiceToOriginal(pDestinationVoice), SourceChannels, DestinationChannels, pLevelMatrix, OperationSet);
}

void XAudio2SourceVoiceProxy::GetOutputMatrix(IXAudio2Voice *pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, float *pLevelMatrix)
{
	m_original->GetOutputMatrix(m_voice_mapper.MapVoiceToOriginal(pDestinationVoice), SourceChannels, DestinationChannels, pLevelMatrix);
}

void XAudio2SourceVoiceProxy::DestroyVoice()
{
	std::stringstream ss;
	ss << "XAudio2SourceVoiceProxy::DestroyVoice " << this;
	logger::log(ss.str());
	m_on_destroy(this);
}