
#include "stdafx.h"
#include "XAudio2MasteringVoiceProxy.h"

#include "XAudio2Proxy.h"

#include "util.h"
#include "logger.h"

XAudio2MasteringVoiceProxy::XAudio2MasteringVoiceProxy(IXAudio2* original_xaudio, IVoiceMapper * voice_mapper, const deleter& on_destroy,
	UINT32 InputChannels, UINT32 InputSampleRate, UINT32 Flags, UINT32 DeviceIndex, const XAUDIO2_EFFECT_CHAIN* pEffectChain)
	: m_voice_mapper(voice_mapper)
	, m_on_destroy(on_destroy)
{
	std::wstringstream ss;
	ss << "XAudio2MasteringVoiceProxy::ctor InputChannels=" << InputChannels << " InputSampleRate=" << InputSampleRate << " DeviceIndex=" << DeviceIndex << " Effects=" << (pEffectChain ? std::to_wstring(pEffectChain->EffectCount) : L"nullptr") << " ";
	logger::log(ss.str());

	IXAudio2MasteringVoice * original_voice = nullptr;
	HRESULT result;
	if (SUCCEEDED(result = original_xaudio->CreateMasteringVoice(&original_voice, InputChannels, InputSampleRate, Flags, DeviceIndex, pEffectChain)))
	{
		m_original = original_voice;
		m_impl.reset(new XAudio2VoiceProxy(L"XAudio2MasteringVoiceProxy", m_voice_mapper, m_original, this));
		m_voice_mapper->RememberMap(original_voice, this);
		logger::log("IXAudio2::CreateSubmixVoice succeeded ", this);
	}
}

XAudio2MasteringVoiceProxy::~XAudio2MasteringVoiceProxy()
{
	m_original->DestroyVoice();
}

void XAudio2MasteringVoiceProxy::GetVoiceDetails(XAUDIO2_VOICE_DETAILS *pVoiceDetails)
{
	m_impl->GetVoiceDetails(pVoiceDetails);
}

HRESULT XAudio2MasteringVoiceProxy::SetOutputVoices(const XAUDIO2_VOICE_SENDS *pSendList)
{
	// That shouldn't happen, actually. Mastering voices cannot have sends.
	return m_impl->SetOutputVoices(pSendList);
}

HRESULT XAudio2MasteringVoiceProxy::SetEffectChain(const XAUDIO2_EFFECT_CHAIN *pEffectChain)
{
	return m_impl->SetEffectChain(pEffectChain);
}

HRESULT XAudio2MasteringVoiceProxy::EnableEffect(UINT32 EffectIndex, UINT32 OperationSet)
{
	return m_impl->EnableEffect(EffectIndex, OperationSet);
}

HRESULT XAudio2MasteringVoiceProxy::DisableEffect(UINT32 EffectIndex, UINT32 OperationSet)
{
	return m_impl->DisableEffect(EffectIndex, OperationSet);
}

void XAudio2MasteringVoiceProxy::GetEffectState(UINT32 EffectIndex, BOOL *pEnabled)
{
	m_impl->GetEffectState(EffectIndex, pEnabled);
}

HRESULT XAudio2MasteringVoiceProxy::SetEffectParameters(UINT32 EffectIndex, const void *pParameters, UINT32 ParametersByteSize, UINT32 OperationSet)
{
	return m_impl->SetEffectParameters(EffectIndex, pParameters, ParametersByteSize, OperationSet);
}

HRESULT XAudio2MasteringVoiceProxy::GetEffectParameters(UINT32 EffectIndex, void *pParameters, UINT32 ParametersByteSize)
{
	return m_impl->GetEffectParameters(EffectIndex, pParameters, ParametersByteSize);
}

HRESULT XAudio2MasteringVoiceProxy::SetFilterParameters(const XAUDIO2_FILTER_PARAMETERS *pParameters, UINT32 OperationSet)
{
	return m_impl->SetFilterParameters(pParameters, OperationSet);
}

void XAudio2MasteringVoiceProxy::GetFilterParameters(XAUDIO2_FILTER_PARAMETERS *pParameters)
{
	m_impl->GetFilterParameters(pParameters);
}

HRESULT XAudio2MasteringVoiceProxy::SetOutputFilterParameters(IXAudio2Voice *pDestinationVoice, const XAUDIO2_FILTER_PARAMETERS *pParameters, UINT32 OperationSet)
{
	return m_impl->SetOutputFilterParameters(m_voice_mapper->MapVoiceToOriginal(pDestinationVoice), pParameters, OperationSet);
}

void XAudio2MasteringVoiceProxy::GetOutputFilterParameters(IXAudio2Voice *pDestinationVoice, XAUDIO2_FILTER_PARAMETERS *pParameters)
{
	m_impl->GetOutputFilterParameters(m_voice_mapper->MapVoiceToOriginal(pDestinationVoice), pParameters);
}

HRESULT XAudio2MasteringVoiceProxy::SetVolume(float Volume, UINT32 OperationSet)
{
	return m_impl->SetVolume(Volume, OperationSet);
}

void XAudio2MasteringVoiceProxy::GetVolume(float *pVolume)
{
	m_impl->GetVolume(pVolume);
}

HRESULT XAudio2MasteringVoiceProxy::SetChannelVolumes(UINT32 Channels, const float *pVolumes, UINT32 OperationSet)
{
	return m_impl->SetChannelVolumes(Channels, pVolumes, OperationSet);
}

void XAudio2MasteringVoiceProxy::GetChannelVolumes(UINT32 Channels, float *pVolumes)
{
	m_impl->GetChannelVolumes(Channels, pVolumes);
}

HRESULT XAudio2MasteringVoiceProxy::SetOutputMatrix(IXAudio2Voice *pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, const float *pLevelMatrix, UINT32 OperationSet)
{
	return m_original->SetOutputMatrix(m_voice_mapper->MapVoiceToOriginal(pDestinationVoice), SourceChannels, DestinationChannels, pLevelMatrix, OperationSet);
}

void XAudio2MasteringVoiceProxy::GetOutputMatrix(IXAudio2Voice *pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, float *pLevelMatrix)
{
	m_original->GetOutputMatrix(m_voice_mapper->MapVoiceToOriginal(pDestinationVoice), SourceChannels, DestinationChannels, pLevelMatrix);
}

void XAudio2MasteringVoiceProxy::DestroyVoice()
{
	std::wstringstream ss;
	ss << "XAudio2MasteringVoiceProxy::DestroyVoice " << this;
	logger::log(ss.str());
	m_on_destroy(this);
}