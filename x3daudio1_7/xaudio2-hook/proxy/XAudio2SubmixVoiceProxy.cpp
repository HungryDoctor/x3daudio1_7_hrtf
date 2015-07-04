#include "stdafx.h"
#include "XAudio2SubmixVoiceProxy.h"

#include "XAudio2Proxy.h"

#include "util.h"
#include "logger.h"

XAudio2SubmixVoiceProxy::XAudio2SubmixVoiceProxy(const IVoiceMapper & voice_mapper, IXAudio2SubmixVoice * original, const deleter & on_destroy)
	: m_voice_mapper(voice_mapper)
	, m_original(original)
	, m_on_destroy(on_destroy)
{

}

XAudio2SubmixVoiceProxy::~XAudio2SubmixVoiceProxy()
{
	m_original->DestroyVoice();
}

void XAudio2SubmixVoiceProxy::GetVoiceDetails(XAUDIO2_VOICE_DETAILS *pVoiceDetails)
{
	m_original->GetVoiceDetails(pVoiceDetails);
}

HRESULT XAudio2SubmixVoiceProxy::SetOutputVoices(const XAUDIO2_VOICE_SENDS *pSendList)
{
	std::stringstream ss;
	ss << "XAudio2SubmixVoiceProxy::SetOutputVoices " << this << " ";
	print_sends(ss, pSendList);
	logger::log(ss.str());

	XAUDIO2_VOICE_SENDS originalSendList = { 0 };
	if (pSendList)
		m_voice_mapper.MapSendsToOriginal(*pSendList, originalSendList);

	auto result = m_original->SetOutputVoices(pSendList ? &originalSendList : 0);

	m_voice_mapper.CleanupSends(originalSendList);

	return result;
}

HRESULT XAudio2SubmixVoiceProxy::SetEffectChain(const XAUDIO2_EFFECT_CHAIN *pEffectChain)
{
	return m_original->SetEffectChain(pEffectChain);
}

HRESULT XAudio2SubmixVoiceProxy::EnableEffect(UINT32 EffectIndex, UINT32 OperationSet)
{
	return m_original->EnableEffect(EffectIndex, OperationSet);
}

HRESULT XAudio2SubmixVoiceProxy::DisableEffect(UINT32 EffectIndex, UINT32 OperationSet)
{
	return m_original->DisableEffect(EffectIndex, OperationSet);
}

void XAudio2SubmixVoiceProxy::GetEffectState(UINT32 EffectIndex, BOOL *pEnabled)
{
	m_original->GetEffectState(EffectIndex, pEnabled);
}

HRESULT XAudio2SubmixVoiceProxy::SetEffectParameters(UINT32 EffectIndex, const void *pParameters, UINT32 ParametersByteSize, UINT32 OperationSet)
{
	return m_original->SetEffectParameters(EffectIndex, pParameters, ParametersByteSize, OperationSet);
}

HRESULT XAudio2SubmixVoiceProxy::GetEffectParameters(UINT32 EffectIndex, void *pParameters, UINT32 ParametersByteSize)
{
	return m_original->GetEffectParameters(EffectIndex, pParameters, ParametersByteSize);
}

HRESULT XAudio2SubmixVoiceProxy::SetFilterParameters(const XAUDIO2_FILTER_PARAMETERS *pParameters, UINT32 OperationSet)
{
	return m_original->SetFilterParameters(pParameters, OperationSet);
}

void XAudio2SubmixVoiceProxy::GetFilterParameters(XAUDIO2_FILTER_PARAMETERS *pParameters)
{
	m_original->GetFilterParameters(pParameters);
}

HRESULT XAudio2SubmixVoiceProxy::SetOutputFilterParameters(IXAudio2Voice *pDestinationVoice, const XAUDIO2_FILTER_PARAMETERS *pParameters, UINT32 OperationSet)
{
	return m_original->SetOutputFilterParameters(m_voice_mapper.MapVoiceToOriginal(pDestinationVoice), pParameters, OperationSet);
}

void XAudio2SubmixVoiceProxy::GetOutputFilterParameters(IXAudio2Voice *pDestinationVoice, XAUDIO2_FILTER_PARAMETERS *pParameters)
{
	m_original->GetOutputFilterParameters(m_voice_mapper.MapVoiceToOriginal(pDestinationVoice), pParameters);
}

HRESULT XAudio2SubmixVoiceProxy::SetVolume(float Volume, UINT32 OperationSet)
{
	return m_original->SetVolume(Volume, OperationSet);
}

void XAudio2SubmixVoiceProxy::GetVolume(float *pVolume)
{
	m_original->GetVolume(pVolume);
}

HRESULT XAudio2SubmixVoiceProxy::SetChannelVolumes(UINT32 Channels, const float *pVolumes, UINT32 OperationSet)
{
	return m_original->SetChannelVolumes(Channels, pVolumes, OperationSet);
}

void XAudio2SubmixVoiceProxy::GetChannelVolumes(UINT32 Channels, float *pVolumes)
{
	m_original->GetChannelVolumes(Channels, pVolumes);
}

HRESULT XAudio2SubmixVoiceProxy::SetOutputMatrix(IXAudio2Voice *pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, const float *pLevelMatrix, UINT32 OperationSet)
{
	return m_original->SetOutputMatrix(m_voice_mapper.MapVoiceToOriginal(pDestinationVoice), SourceChannels, DestinationChannels, pLevelMatrix, OperationSet);
}

void XAudio2SubmixVoiceProxy::GetOutputMatrix(IXAudio2Voice *pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, float *pLevelMatrix)
{
	m_original->GetOutputMatrix(m_voice_mapper.MapVoiceToOriginal(pDestinationVoice), SourceChannels, DestinationChannels, pLevelMatrix);
}

void XAudio2SubmixVoiceProxy::DestroyVoice()
{
	std::stringstream ss;
	ss << "XAudio2SubmixVoiceProxy::DestroyVoice " << this;
	logger::log(ss.str());
	m_on_destroy(this);
}