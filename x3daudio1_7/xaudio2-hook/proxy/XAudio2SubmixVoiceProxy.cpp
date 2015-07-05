#include "stdafx.h"
#include "XAudio2SubmixVoiceProxy.h"

#include "XAudio2Proxy.h"

#include <XAPO.h>

#include "util.h"
#include "logger.h"
#include <cmath>

XAudio2SubmixVoiceProxy::XAudio2SubmixVoiceProxy(ISound3DRegistry * sound3d_registry, const IVoiceMapper & voice_mapper, IXAudio2SubmixVoice * original, const deleter & on_destroy)
	: m_sound3d_registry(sound3d_registry)
	, m_voice_mapper(voice_mapper)
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
	std::wstringstream ss;
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
	std::wstringstream ss;
	if (pEffectChain)
	{
		ss << "[";
		for (INT32 i = 0; i < pEffectChain->EffectCount; i++)
		{
			IXAPO * effect;
			pEffectChain->pEffectDescriptors[i].pEffect->QueryInterface(&effect);
			XAPO_REGISTRATION_PROPERTIES * props;
			effect->GetRegistrationProperties(&props);
			ss << props->FriendlyName;
			if (i < pEffectChain->EffectCount - 1)
				ss << ", ";
			XAPOFree(props);
		}
		ss << "]";
	}
	logger::log("XAudio2SubmixVoiceProxy::SetEffectChain", " ", this, " pEffectChain=", pEffectChain, (pEffectChain != nullptr ? L" count=" + std::to_wstring(pEffectChain->EffectCount) : L" none"), " ", ss.str());
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
	if (DestinationChannels == 2 && std::isnan(pLevelMatrix[0]))
	{
		auto & id = *reinterpret_cast<const sound_id*>(&pLevelMatrix[1]);
		auto sound3d = m_sound3d_registry->GetEntry(id);

		float matrix[XAUDIO2_MAX_AUDIO_CHANNELS * 2]; // we only support two output channels
		for (UINT32 i = 0; i < SourceChannels; i++)
		{
			matrix[i * 2 + 0] = sound3d.matrix_coefficients[0];
			matrix[i * 2 + 1] = -sound3d.matrix_coefficients[1];
		}
		return m_original->SetOutputMatrix(m_voice_mapper.MapVoiceToOriginal(pDestinationVoice), SourceChannels, DestinationChannels, matrix, OperationSet);
	}
	else
	{
		return m_original->SetOutputMatrix(m_voice_mapper.MapVoiceToOriginal(pDestinationVoice), SourceChannels, DestinationChannels, pLevelMatrix, OperationSet);
	}
}

void XAudio2SubmixVoiceProxy::GetOutputMatrix(IXAudio2Voice *pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, float *pLevelMatrix)
{
	m_original->GetOutputMatrix(m_voice_mapper.MapVoiceToOriginal(pDestinationVoice), SourceChannels, DestinationChannels, pLevelMatrix);
}

void XAudio2SubmixVoiceProxy::DestroyVoice()
{
	std::wstringstream ss;
	ss << "XAudio2SubmixVoiceProxy::DestroyVoice " << this;
	logger::log(ss.str());
	m_on_destroy(this);
}