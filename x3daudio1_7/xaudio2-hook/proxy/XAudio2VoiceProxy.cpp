#include "stdafx.h"
#include "XAudio2VoiceProxy.h"

#include "XAPO/HrtfEffect.h"
#include <XAPO.h>
#include <XAPOFX.h>

#include "util.h"
#include "logger.h"
#include <cmath>

XAudio2VoiceProxy::XAudio2VoiceProxy(const std::wstring & type_name, ISound3DRegistry * sound3d_registry, const IVoiceMapper & voice_mapper, IXAudio2Voice * original, const void * id)
	: m_type_name(type_name)
	, m_sound3d_registry(sound3d_registry)
	, m_voice_mapper(voice_mapper)
	, m_original(original)
	, m_input_channels(0)
	, m_id(id)
	, m_hrtf_effect_index(UINT_MAX)
{

}

XAudio2VoiceProxy::XAudio2VoiceProxy(const std::wstring & type_name, ISound3DRegistry * sound3d_registry, const IVoiceMapper & voice_mapper, IXAudio2Voice * original, UINT32 input_channels, const void * id, const XAUDIO2_EFFECT_CHAIN * original_chain)
	: m_type_name(type_name)
	, m_sound3d_registry(sound3d_registry)
	, m_voice_mapper(voice_mapper)
	, m_original(original)
	, m_input_channels(input_channels)
	, m_id(id)
	, m_hrtf_effect_index(UINT_MAX)
{
	AlterAndSetEffectChain(original_chain);
}

void XAudio2VoiceProxy::GetVoiceDetails(XAUDIO2_VOICE_DETAILS *pVoiceDetails)
{
	m_original->GetVoiceDetails(pVoiceDetails);
}

HRESULT XAudio2VoiceProxy::SetOutputVoices(const XAUDIO2_VOICE_SENDS *pSendList)
{
	std::wstringstream ss;
	ss << m_type_name << "::SetOutputVoices " << m_id << " ";
	print_sends(ss, pSendList);
	logger::log(ss.str());

	XAUDIO2_VOICE_SENDS originalSendList = { 0 };
	if (pSendList)
		m_voice_mapper.MapSendsToOriginal(*pSendList, originalSendList);

	auto result = m_original->SetOutputVoices(pSendList ? &originalSendList : 0);

	m_voice_mapper.CleanupSends(originalSendList);

	return result;
}

HRESULT XAudio2VoiceProxy::SetEffectChain(const XAUDIO2_EFFECT_CHAIN *pEffectChain)
{
	return AlterAndSetEffectChain(pEffectChain);
}

HRESULT XAudio2VoiceProxy::EnableEffect(UINT32 EffectIndex, UINT32 OperationSet)
{
	return m_original->EnableEffect(EffectIndex, OperationSet);
}

HRESULT XAudio2VoiceProxy::DisableEffect(UINT32 EffectIndex, UINT32 OperationSet)
{
	return m_original->DisableEffect(EffectIndex, OperationSet);
}

void XAudio2VoiceProxy::GetEffectState(UINT32 EffectIndex, BOOL *pEnabled)
{
	m_original->GetEffectState(EffectIndex, pEnabled);
}

HRESULT XAudio2VoiceProxy::SetEffectParameters(UINT32 EffectIndex, const void *pParameters, UINT32 ParametersByteSize, UINT32 OperationSet)
{
	return m_original->SetEffectParameters(EffectIndex, pParameters, ParametersByteSize, OperationSet);
}

HRESULT XAudio2VoiceProxy::GetEffectParameters(UINT32 EffectIndex, void *pParameters, UINT32 ParametersByteSize)
{
	return m_original->GetEffectParameters(EffectIndex, pParameters, ParametersByteSize);
}

HRESULT XAudio2VoiceProxy::SetFilterParameters(const XAUDIO2_FILTER_PARAMETERS *pParameters, UINT32 OperationSet)
{
	return m_original->SetFilterParameters(pParameters, OperationSet);
}

void XAudio2VoiceProxy::GetFilterParameters(XAUDIO2_FILTER_PARAMETERS *pParameters)
{
	m_original->GetFilterParameters(pParameters);
}

HRESULT XAudio2VoiceProxy::SetOutputFilterParameters(IXAudio2Voice *pDestinationVoice, const XAUDIO2_FILTER_PARAMETERS *pParameters, UINT32 OperationSet)
{
	return m_original->SetOutputFilterParameters(m_voice_mapper.MapVoiceToOriginal(pDestinationVoice), pParameters, OperationSet);
}

void XAudio2VoiceProxy::GetOutputFilterParameters(IXAudio2Voice *pDestinationVoice, XAUDIO2_FILTER_PARAMETERS *pParameters)
{
	m_original->GetOutputFilterParameters(m_voice_mapper.MapVoiceToOriginal(pDestinationVoice), pParameters);
}

HRESULT XAudio2VoiceProxy::SetVolume(float Volume, UINT32 OperationSet)
{
	return m_original->SetVolume(Volume, OperationSet);
}

void XAudio2VoiceProxy::GetVolume(float *pVolume)
{
	m_original->GetVolume(pVolume);
}

HRESULT XAudio2VoiceProxy::SetChannelVolumes(UINT32 Channels, const float *pVolumes, UINT32 OperationSet)
{
	return m_original->SetChannelVolumes(Channels, pVolumes, OperationSet);
}

void XAudio2VoiceProxy::GetChannelVolumes(UINT32 Channels, float *pVolumes)
{
	m_original->GetChannelVolumes(Channels, pVolumes);
}

HRESULT XAudio2VoiceProxy::SetOutputMatrix(IXAudio2Voice *pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, const float *pLevelMatrix, UINT32 OperationSet)
{
	logger::log("SetOutputMatrix pLevelMatrix[0]=", pLevelMatrix[0]);

	if (std::isnan(pLevelMatrix[0]))
	{
		//if (DestinationChannels != 2)
		{
			logger::log(m_type_name, "::SetOutputMatrix ", m_id, " Has NaN and DestinationChannels=", DestinationChannels, " m_output_channels=", m_output_channels, " SourceChannels=", SourceChannels);
		}
	}

	//logger::log(m_type_name, "::SetOutputMatrix ", m_id, " ", (std::isnan(pLevelMatrix[0]) ? "NaN" : "Not NaN"));
	if (m_output_channels != SourceChannels)
		logger::log(m_type_name, "::SetOutputMatrix channels count mismatch: m_output_channels=", m_output_channels, " SourceChannels=", SourceChannels);

	if (DestinationChannels == 2 && std::isnan(pLevelMatrix[0]))
	{
		if (m_input_channels == 2)
			m_original->EnableEffect(m_hrtf_effect_index, OperationSet);

		auto & id = *reinterpret_cast<const sound_id*>(&pLevelMatrix[1]);
		auto sound3d = m_sound3d_registry->GetEntry(id);

		float matrix[] = { 1, 0, 0, 1 };
		return m_original->SetOutputMatrix(m_voice_mapper.MapVoiceToOriginal(pDestinationVoice), 2, 2, matrix, OperationSet);
	}
	else
	{
		if (m_hrtf_effect_index != UINT_MAX)
			m_original->DisableEffect(m_hrtf_effect_index, OperationSet);

		return m_original->SetOutputMatrix(m_voice_mapper.MapVoiceToOriginal(pDestinationVoice), SourceChannels, DestinationChannels, pLevelMatrix, OperationSet);
	}
}

void XAudio2VoiceProxy::GetOutputMatrix(IXAudio2Voice *pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, float *pLevelMatrix)
{
	m_original->GetOutputMatrix(m_voice_mapper.MapVoiceToOriginal(pDestinationVoice), SourceChannels, DestinationChannels, pLevelMatrix);
}

HRESULT XAudio2VoiceProxy::AlterAndSetEffectChain(const XAUDIO2_EFFECT_CHAIN * original_chain)
{
	std::wstringstream ss;
	if (original_chain)
	{
		ss << "[";
		for (UINT32 i = 0; i < original_chain->EffectCount; i++)
		{
			IXAPO * effect;
			original_chain->pEffectDescriptors[i].pEffect->QueryInterface(&effect);
			XAPO_REGISTRATION_PROPERTIES * props;
			effect->GetRegistrationProperties(&props);
			ss << props->FriendlyName;
			if (i < original_chain->EffectCount - 1)
				ss << ", ";
			XAPOFree(props);
		}
		ss << "]";
	}
	logger::log(m_type_name, "::AlterEffectChain", " ", m_id, " original_chain=", original_chain, (original_chain != nullptr ? L" count=" + std::to_wstring(original_chain->EffectCount) : L" none"), " ", ss.str());


	UINT32 effect_count = (original_chain != nullptr) ? original_chain->EffectCount : 0;
	_ASSERT(effect_count < 64);

	m_output_channels = (effect_count > 0) ? original_chain->pEffectDescriptors[effect_count - 1].OutputChannels : m_input_channels;

	if (true || m_output_channels == 2)
	{
		logger::log("AlterEffectChain actually altering");

		XAUDIO2_EFFECT_DESCRIPTOR apoDesc[64];
		for (UINT32 i = 0; i < effect_count; i++)
		{
			apoDesc[i] = original_chain->pEffectDescriptors[i];
		}

		IUnknown * pAPO;
		//CreateFX(__uuidof(FXEcho), &pAPO);
		HrtfXapoEffect::CreateInstance()->QueryInterface(__uuidof(IUnknown), reinterpret_cast<void**>(&pAPO));

		apoDesc[effect_count].InitialState = false;
		apoDesc[effect_count].OutputChannels = 2;
		apoDesc[effect_count].pEffect = pAPO;

		XAUDIO2_EFFECT_CHAIN chain;
		chain.EffectCount = effect_count + 1;
		chain.pEffectDescriptors = apoDesc;

		HRESULT result;
		if (!SUCCEEDED(result = m_original->SetEffectChain(&chain)))
		{
			logger::log("SetEffectChain failed with code ", result);
		}
		else
		{
			m_hrtf_effect_index = effect_count;
		}
		return result;
	}
	else
	{
		m_hrtf_effect_index = UINT_MAX;
		return m_original->SetEffectChain(original_chain);
	}
}
