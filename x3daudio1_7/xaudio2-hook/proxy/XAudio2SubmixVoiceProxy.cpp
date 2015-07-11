#include "stdafx.h"
#include "XAudio2SubmixVoiceProxy.h"

#include "XAudio2Proxy.h"

#include <XAPO.h>

#include "util.h"
#include "logger.h"
#include <cmath>


XAudio2SubmixVoiceProxy::XAudio2SubmixVoiceProxy(IXAudio2 * original_xaudio, ISound3DRegistry* sound3d_registry, IVoiceMapper* voice_mapper, const deleter& on_destroy,
	UINT32 InputChannels, UINT32 InputSampleRate, UINT32 Flags, UINT32 ProcessingStage, const XAUDIO2_VOICE_SENDS* pSendList, const XAUDIO2_EFFECT_CHAIN* pEffectChain)
	: m_sound3d_registry(sound3d_registry)
	, m_voice_mapper(voice_mapper)
	, m_on_destroy(on_destroy)
{

	std::wstringstream ss;
	ss << "XAudio2SubmixVoiceProxy::ctor InputChannels=" << InputChannels << " InputSampleRate=" << InputSampleRate << " ProcessingStage=" << ProcessingStage << " Effects=" << (pEffectChain ? std::to_wstring(pEffectChain->EffectCount) : L"nullptr") << " ";
	print_sends(ss, pSendList);
	logger::log(ss.str());

	XAUDIO2_VOICE_SENDS originalSendList = { 0 };
	if (pSendList)
		m_voice_mapper->MapSendsToOriginal(*pSendList, originalSendList);

	IXAudio2SubmixVoice * original_voice = nullptr;
	HRESULT result;
	if (SUCCEEDED(result = original_xaudio->CreateSubmixVoice(&original_voice, InputChannels, InputSampleRate, Flags, ProcessingStage, pSendList ? &originalSendList : 0, nullptr)))
	{
		m_original = original_voice;
		m_impl.reset(new XAudio2VoiceProxy(L"XAudio2SubmixVoiceProxy", m_sound3d_registry, m_voice_mapper, m_original, InputChannels, this));
		m_voice_mapper->RememberMap(original_voice, this);
		logger::log("IXAudio2::CreateSubmixVoice succeeded ", this);
	}

	m_voice_mapper->CleanupSends(originalSendList);
}

XAudio2SubmixVoiceProxy::~XAudio2SubmixVoiceProxy()
{
	m_original->DestroyVoice();
}

void XAudio2SubmixVoiceProxy::GetVoiceDetails(XAUDIO2_VOICE_DETAILS *pVoiceDetails)
{
	m_impl->GetVoiceDetails(pVoiceDetails);
}

HRESULT XAudio2SubmixVoiceProxy::SetOutputVoices(const XAUDIO2_VOICE_SENDS *pSendList)
{
	return m_impl->SetOutputVoices(pSendList);
}

HRESULT XAudio2SubmixVoiceProxy::SetEffectChain(const XAUDIO2_EFFECT_CHAIN *pEffectChain)
{
	return m_impl->SetEffectChain(pEffectChain);
}

HRESULT XAudio2SubmixVoiceProxy::EnableEffect(UINT32 EffectIndex, UINT32 OperationSet)
{
	return m_impl->EnableEffect(EffectIndex, OperationSet);
}

HRESULT XAudio2SubmixVoiceProxy::DisableEffect(UINT32 EffectIndex, UINT32 OperationSet)
{
	return m_impl->DisableEffect(EffectIndex, OperationSet);
}

void XAudio2SubmixVoiceProxy::GetEffectState(UINT32 EffectIndex, BOOL *pEnabled)
{
	m_impl->GetEffectState(EffectIndex, pEnabled);
}

HRESULT XAudio2SubmixVoiceProxy::SetEffectParameters(UINT32 EffectIndex, const void *pParameters, UINT32 ParametersByteSize, UINT32 OperationSet)
{
	return m_impl->SetEffectParameters(EffectIndex, pParameters, ParametersByteSize, OperationSet);
}

HRESULT XAudio2SubmixVoiceProxy::GetEffectParameters(UINT32 EffectIndex, void *pParameters, UINT32 ParametersByteSize)
{
	return m_impl->GetEffectParameters(EffectIndex, pParameters, ParametersByteSize);
}

HRESULT XAudio2SubmixVoiceProxy::SetFilterParameters(const XAUDIO2_FILTER_PARAMETERS *pParameters, UINT32 OperationSet)
{
	return m_impl->SetFilterParameters(pParameters, OperationSet);
}

void XAudio2SubmixVoiceProxy::GetFilterParameters(XAUDIO2_FILTER_PARAMETERS *pParameters)
{
	m_impl->GetFilterParameters(pParameters);
}

HRESULT XAudio2SubmixVoiceProxy::SetOutputFilterParameters(IXAudio2Voice *pDestinationVoice, const XAUDIO2_FILTER_PARAMETERS *pParameters, UINT32 OperationSet)
{
	return m_impl->SetOutputFilterParameters(m_voice_mapper->MapVoiceToOriginal(pDestinationVoice), pParameters, OperationSet);
}

void XAudio2SubmixVoiceProxy::GetOutputFilterParameters(IXAudio2Voice *pDestinationVoice, XAUDIO2_FILTER_PARAMETERS *pParameters)
{
	m_impl->GetOutputFilterParameters(m_voice_mapper->MapVoiceToOriginal(pDestinationVoice), pParameters);
}

HRESULT XAudio2SubmixVoiceProxy::SetVolume(float Volume, UINT32 OperationSet)
{
	return m_impl->SetVolume(Volume, OperationSet);
}

void XAudio2SubmixVoiceProxy::GetVolume(float *pVolume)
{
	m_impl->GetVolume(pVolume);
}

HRESULT XAudio2SubmixVoiceProxy::SetChannelVolumes(UINT32 Channels, const float *pVolumes, UINT32 OperationSet)
{
	return m_impl->SetChannelVolumes(Channels, pVolumes, OperationSet);
}

void XAudio2SubmixVoiceProxy::GetChannelVolumes(UINT32 Channels, float *pVolumes)
{
	m_impl->GetChannelVolumes(Channels, pVolumes);
}

HRESULT XAudio2SubmixVoiceProxy::SetOutputMatrix(IXAudio2Voice *pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, const float *pLevelMatrix, UINT32 OperationSet)
{
	if (std::isnan(pLevelMatrix[0]))
	{
		//logger::log("XAudio2SubmixVoiceProxy::SetOutputMatrix NaN ", this);
	}
	return m_impl->SetOutputMatrix(m_voice_mapper->MapVoiceToOriginal(pDestinationVoice), SourceChannels, DestinationChannels, pLevelMatrix, OperationSet);
}

void XAudio2SubmixVoiceProxy::GetOutputMatrix(IXAudio2Voice *pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, float *pLevelMatrix)
{
	m_impl->GetOutputMatrix(m_voice_mapper->MapVoiceToOriginal(pDestinationVoice), SourceChannels, DestinationChannels, pLevelMatrix);
}

void XAudio2SubmixVoiceProxy::DestroyVoice()
{
	std::wstringstream ss;
	ss << "XAudio2SubmixVoiceProxy::DestroyVoice " << this;
	logger::log(ss.str());
	m_on_destroy(this);
}