
#include "stdafx.h"
#include "XAudio2MasteringVoiceProxy.h"

#include "XAudio2Proxy.h"

#include "util.h"
#include "logger.h"

XAudio2MasteringVoiceProxy::XAudio2MasteringVoiceProxy(const IVoiceMapper & voice_mapper, IXAudio2MasteringVoice * original, const deleter & on_destroy)
	: m_voice_mapper(voice_mapper)
	, m_original(original)
	, m_on_destroy(on_destroy)
{

}

XAudio2MasteringVoiceProxy::~XAudio2MasteringVoiceProxy()
{
	m_original->DestroyVoice();
}

void XAudio2MasteringVoiceProxy::GetVoiceDetails(XAUDIO2_VOICE_DETAILS *pVoiceDetails)
{
	m_original->GetVoiceDetails(pVoiceDetails);
}

HRESULT XAudio2MasteringVoiceProxy::SetOutputVoices(const XAUDIO2_VOICE_SENDS *pSendList)
{
	// That shouldn't happen, actually. Mastering voices cannot have sends.

	std::stringstream ss;
	ss << "XAudio2MasteringVoiceProxy::SetOutputVoices " << this << " ";
	print_sends(ss, pSendList);
	logger::log(ss.str());

	XAUDIO2_VOICE_SENDS originalSendList = { 0 };
	if (pSendList)
		m_voice_mapper.MapSendsToOriginal(*pSendList, originalSendList);

	auto result = m_original->SetOutputVoices(pSendList ? &originalSendList : 0);

	m_voice_mapper.CleanupSends(originalSendList);

	return result;
}

HRESULT XAudio2MasteringVoiceProxy::SetEffectChain(const XAUDIO2_EFFECT_CHAIN *pEffectChain)
{
	return m_original->SetEffectChain(pEffectChain);
}

HRESULT XAudio2MasteringVoiceProxy::EnableEffect(UINT32 EffectIndex, UINT32 OperationSet)
{
	return m_original->EnableEffect(EffectIndex, OperationSet);
}

HRESULT XAudio2MasteringVoiceProxy::DisableEffect(UINT32 EffectIndex, UINT32 OperationSet)
{
	return m_original->DisableEffect(EffectIndex, OperationSet);
}

void XAudio2MasteringVoiceProxy::GetEffectState(UINT32 EffectIndex, BOOL *pEnabled)
{
	m_original->GetEffectState(EffectIndex, pEnabled);
}

HRESULT XAudio2MasteringVoiceProxy::SetEffectParameters(UINT32 EffectIndex, const void *pParameters, UINT32 ParametersByteSize, UINT32 OperationSet)
{
	return m_original->SetEffectParameters(EffectIndex, pParameters, ParametersByteSize, OperationSet);
}

HRESULT XAudio2MasteringVoiceProxy::GetEffectParameters(UINT32 EffectIndex, void *pParameters, UINT32 ParametersByteSize)
{
	return m_original->GetEffectParameters(EffectIndex, pParameters, ParametersByteSize);
}

HRESULT XAudio2MasteringVoiceProxy::SetFilterParameters(const XAUDIO2_FILTER_PARAMETERS *pParameters, UINT32 OperationSet)
{
	return m_original->SetFilterParameters(pParameters, OperationSet);
}

void XAudio2MasteringVoiceProxy::GetFilterParameters(XAUDIO2_FILTER_PARAMETERS *pParameters)
{
	m_original->GetFilterParameters(pParameters);
}

HRESULT XAudio2MasteringVoiceProxy::SetOutputFilterParameters(IXAudio2Voice *pDestinationVoice, const XAUDIO2_FILTER_PARAMETERS *pParameters, UINT32 OperationSet)
{
	return m_original->SetOutputFilterParameters(m_voice_mapper.MapVoiceToOriginal(pDestinationVoice), pParameters, OperationSet);
}

void XAudio2MasteringVoiceProxy::GetOutputFilterParameters(IXAudio2Voice *pDestinationVoice, XAUDIO2_FILTER_PARAMETERS *pParameters)
{
	m_original->GetOutputFilterParameters(m_voice_mapper.MapVoiceToOriginal(pDestinationVoice), pParameters);
}

HRESULT XAudio2MasteringVoiceProxy::SetVolume(float Volume, UINT32 OperationSet)
{
	return m_original->SetVolume(Volume, OperationSet);
}

void XAudio2MasteringVoiceProxy::GetVolume(float *pVolume)
{
	m_original->GetVolume(pVolume);
}

HRESULT XAudio2MasteringVoiceProxy::SetChannelVolumes(UINT32 Channels, const float *pVolumes, UINT32 OperationSet)
{
	return m_original->SetChannelVolumes(Channels, pVolumes, OperationSet);
}

void XAudio2MasteringVoiceProxy::GetChannelVolumes(UINT32 Channels, float *pVolumes)
{
	m_original->GetChannelVolumes(Channels, pVolumes);
}

HRESULT XAudio2MasteringVoiceProxy::SetOutputMatrix(IXAudio2Voice *pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, const float *pLevelMatrix, UINT32 OperationSet)
{
	return m_original->SetOutputMatrix(m_voice_mapper.MapVoiceToOriginal(pDestinationVoice), SourceChannels, DestinationChannels, pLevelMatrix, OperationSet);
}

void XAudio2MasteringVoiceProxy::GetOutputMatrix(IXAudio2Voice *pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, float *pLevelMatrix)
{
	m_original->GetOutputMatrix(m_voice_mapper.MapVoiceToOriginal(pDestinationVoice), SourceChannels, DestinationChannels, pLevelMatrix);
}

void XAudio2MasteringVoiceProxy::DestroyVoice()
{
	std::stringstream ss;
	ss << "XAudio2MasteringVoiceProxy::DestroyVoice " << this;
	logger::log(ss.str());
	m_on_destroy(this);
}