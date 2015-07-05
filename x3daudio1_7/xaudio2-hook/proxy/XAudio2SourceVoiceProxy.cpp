#include "stdafx.h"
#include "XAudio2SourceVoiceProxy.h"

#include "XAudio2Proxy.h"
#include "wave/WaveFile.h"
#include "XAPO/HrtfEffect.h"
#include <XAPO.h>

#include "util.h"
#include "logger.h"

std::wstring GetName(const XAudio2SourceVoiceProxy * ptr)
{
	std::wstringstream ss;
	ss << ptr;
	return ss.str();
}

XAudio2SourceVoiceProxy::XAudio2SourceVoiceProxy(ISound3DRegistry * sound3d_registry, const IVoiceMapper & voice_mapper, IXAudio2SourceVoice * original, const deleter & on_destroy)
	: m_sound3d_registry(sound3d_registry)
	, m_voice_mapper(voice_mapper)
	, m_original(original)
	, m_on_destroy(on_destroy)
	, m_has_hrtf_effect(false)
	, m_has_effect(false)
{
	XAUDIO2_VOICE_DETAILS details;
	original->GetVoiceDetails(&details);
	m_wave_file.reset(new WaveFile(std::wstring(L"wavs\\") + GetName(this) + L".wav", details.InputChannels, details.InputSampleRate, 16));
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
	m_wave_file->AppendData(pBuffer->pAudioData, pBuffer->AudioBytes);
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
	std::wstringstream ss;
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
	m_has_effect = pEffectChain != nullptr;
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
	logger::log("XAudio2SourceVoiceProxy::SetEffectChain", " ", this, " pEffectChain=", pEffectChain, (pEffectChain != nullptr ? L" count=" + std::to_wstring(pEffectChain->EffectCount) : L" none"), " ", ss.str());
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
	if (m_has_effect)
	{
		logger::log("XAudio2SourceVoiceProxy::SetOutputMatrix ", this, " has effect");
	}

	if (DestinationChannels == 2 && std::isnan(pLevelMatrix[0]))
	{
		/*if (!m_has_hrtf_effect)
		{
			m_has_hrtf_effect = true;
			auto hrtfEffect = HrtfXapoEffect::CreateInstance();

			XAUDIO2_EFFECT_DESCRIPTOR apoDesc[1] = { 0 };
			apoDesc[0].InitialState = true;
			apoDesc[0].OutputChannels = 2;
			apoDesc[0].pEffect = static_cast<IXAPO*>(hrtfEffect);

			XAUDIO2_EFFECT_CHAIN chain = { 0 };
			chain.EffectCount = sizeof(apoDesc) / sizeof(apoDesc[0]);
			chain.pEffectDescriptors = apoDesc;
			m_original->SetEffectChain(&chain);
		}*/

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

void XAudio2SourceVoiceProxy::GetOutputMatrix(IXAudio2Voice *pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, float *pLevelMatrix)
{
	m_original->GetOutputMatrix(m_voice_mapper.MapVoiceToOriginal(pDestinationVoice), SourceChannels, DestinationChannels, pLevelMatrix);
}

void XAudio2SourceVoiceProxy::DestroyVoice()
{
	std::wstringstream ss;
	ss << "XAudio2SourceVoiceProxy::DestroyVoice " << this;
	logger::log(ss.str());
	m_on_destroy(this);
}