#include "stdafx.h"
#include "XAudio2SourceVoiceProxy.h"

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


XAudio2SourceVoiceProxy::XAudio2SourceVoiceProxy(IXAudio2 * original_xaudio, ISound3DRegistry * sound3d_registry, IVoiceMapper * voice_mapper, const deleter & on_destroy,
                                                 const WAVEFORMATEX * pSourceFormat, UINT32 Flags, float MaxFrequencyRatio, IXAudio2VoiceCallback * pCallback, const XAUDIO2_VOICE_SENDS * pSendList, const XAUDIO2_EFFECT_CHAIN * pEffectChain)
	: m_sound3d_registry(sound3d_registry)
	  , m_voice_mapper(voice_mapper)
	  , m_on_destroy(on_destroy)
{
	std::wstringstream ss;
	ss << "XAudio2SourceVoiceProxy::ctor FormatChannels=" << pSourceFormat->nChannels << " FormatSampleRate=" << pSourceFormat->nSamplesPerSec << " MaxFrequencyRatio=" << MaxFrequencyRatio << " Effects=" << (pEffectChain ? std::to_wstring(pEffectChain->EffectCount) : L"nullptr") << " ";
	print_sends(ss, pSendList);
	logger::log(ss.str());

	XAUDIO2_VOICE_SENDS originalSendList = {0};
	if (pSendList)
		m_voice_mapper->MapSendsToOriginal(*pSendList, originalSendList);

	UINT32 effect_count = (pEffectChain != nullptr) ? pEffectChain->EffectCount : 0;
	_ASSERT(effect_count < 64);

	XAUDIO2_EFFECT_DESCRIPTOR apoDesc[64];
	for (UINT32 i = 0; i < effect_count; i++)
	{
		apoDesc[i] = pEffectChain->pEffectDescriptors[i];
	}

	IUnknown * apo;
	//CreateFX(__uuidof(FXEQ), apo.p);
	apo = static_cast<IUnknown *>(static_cast<IXAPOParameters *>(HrtfXapoEffect::CreateInstance()));

	apoDesc[effect_count].InitialState = false;
	apoDesc[effect_count].OutputChannels = 2;
	apoDesc[effect_count].pEffect = apo;

	XAUDIO2_EFFECT_CHAIN chain;
	chain.EffectCount = effect_count + 1;
	chain.pEffectDescriptors = apoDesc;

	IXAudio2SourceVoice * original_voice = nullptr;
	HRESULT result;
	if (SUCCEEDED(result = original_xaudio->CreateSourceVoice(&original_voice, pSourceFormat, Flags, MaxFrequencyRatio, pCallback, pSendList ? &originalSendList : 0, &chain)))
	{
		m_original = original_voice;
		m_impl.reset(new XAudio2VoiceProxy(L"XAudio2SourceVoiceProxy", m_sound3d_registry, m_voice_mapper, m_original, pSourceFormat->nChannels, this, effect_count));
		m_voice_mapper->RememberMap(original_voice, this);
		logger::log("IXAudio2::CreateSourceVoice succeeded ", this);
	}
	apo->Release();
	m_voice_mapper->CleanupSends(originalSendList);

#ifdef DUMP_SOUND_WAV
	XAUDIO2_VOICE_DETAILS details;
	original->GetVoiceDetails(&details);
	m_wave_file.reset(new WaveFile(std::wstring(L"wavs\\") + std::to_wstring(input_channels) + L"_" + GetName(this) + L".wav", details.InputChannels, details.InputSampleRate, 16));
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

HRESULT XAudio2SourceVoiceProxy::SubmitSourceBuffer(const XAUDIO2_BUFFER * pBuffer, const XAUDIO2_BUFFER_WMA * pBufferWMA)
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

void XAudio2SourceVoiceProxy::GetVoiceDetails(XAUDIO2_VOICE_DETAILS * pVoiceDetails)
{
	m_impl->GetVoiceDetails(pVoiceDetails);
}

HRESULT XAudio2SourceVoiceProxy::SetOutputVoices(const XAUDIO2_VOICE_SENDS * pSendList)
{
	return m_impl->SetOutputVoices(pSendList);
}

HRESULT XAudio2SourceVoiceProxy::SetEffectChain(const XAUDIO2_EFFECT_CHAIN * pEffectChain)
{
	return m_impl->SetEffectChain(pEffectChain);
}

HRESULT XAudio2SourceVoiceProxy::EnableEffect(UINT32 EffectIndex, UINT32 OperationSet)
{
	return m_impl->EnableEffect(EffectIndex, OperationSet);
}

HRESULT XAudio2SourceVoiceProxy::DisableEffect(UINT32 EffectIndex, UINT32 OperationSet)
{
	return m_impl->DisableEffect(EffectIndex, OperationSet);
}

void XAudio2SourceVoiceProxy::GetEffectState(UINT32 EffectIndex, BOOL * pEnabled)
{
	m_impl->GetEffectState(EffectIndex, pEnabled);
}

HRESULT XAudio2SourceVoiceProxy::SetEffectParameters(UINT32 EffectIndex, const void * pParameters, UINT32 ParametersByteSize, UINT32 OperationSet)
{
	return m_impl->SetEffectParameters(EffectIndex, pParameters, ParametersByteSize, OperationSet);
}

HRESULT XAudio2SourceVoiceProxy::GetEffectParameters(UINT32 EffectIndex, void * pParameters, UINT32 ParametersByteSize)
{
	return m_impl->GetEffectParameters(EffectIndex, pParameters, ParametersByteSize);
}

HRESULT XAudio2SourceVoiceProxy::SetFilterParameters(const XAUDIO2_FILTER_PARAMETERS * pParameters, UINT32 OperationSet)
{
	return m_impl->SetFilterParameters(pParameters, OperationSet);
}

void XAudio2SourceVoiceProxy::GetFilterParameters(XAUDIO2_FILTER_PARAMETERS * pParameters)
{
	m_impl->GetFilterParameters(pParameters);
}

HRESULT XAudio2SourceVoiceProxy::SetOutputFilterParameters(IXAudio2Voice * pDestinationVoice, const XAUDIO2_FILTER_PARAMETERS * pParameters, UINT32 OperationSet)
{
	return m_impl->SetOutputFilterParameters(m_voice_mapper->MapVoiceToOriginal(pDestinationVoice), pParameters, OperationSet);
}

void XAudio2SourceVoiceProxy::GetOutputFilterParameters(IXAudio2Voice * pDestinationVoice, XAUDIO2_FILTER_PARAMETERS * pParameters)
{
	m_impl->GetOutputFilterParameters(m_voice_mapper->MapVoiceToOriginal(pDestinationVoice), pParameters);
}

HRESULT XAudio2SourceVoiceProxy::SetVolume(float Volume, UINT32 OperationSet)
{
	return m_impl->SetVolume(Volume, OperationSet);
}

void XAudio2SourceVoiceProxy::GetVolume(float * pVolume)
{
	m_impl->GetVolume(pVolume);
}

HRESULT XAudio2SourceVoiceProxy::SetChannelVolumes(UINT32 Channels, const float * pVolumes, UINT32 OperationSet)
{
	return m_impl->SetChannelVolumes(Channels, pVolumes, OperationSet);
}

void XAudio2SourceVoiceProxy::GetChannelVolumes(UINT32 Channels, float * pVolumes)
{
	m_impl->GetChannelVolumes(Channels, pVolumes);
}

HRESULT XAudio2SourceVoiceProxy::SetOutputMatrix(IXAudio2Voice * pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, const float * pLevelMatrix, UINT32 OperationSet)
{
	return m_impl->SetOutputMatrix(m_voice_mapper->MapVoiceToOriginal(pDestinationVoice), SourceChannels, DestinationChannels, pLevelMatrix, OperationSet);
}

void XAudio2SourceVoiceProxy::GetOutputMatrix(IXAudio2Voice * pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, float * pLevelMatrix)
{
	m_impl->GetOutputMatrix(m_voice_mapper->MapVoiceToOriginal(pDestinationVoice), SourceChannels, DestinationChannels, pLevelMatrix);
}

void XAudio2SourceVoiceProxy::DestroyVoice()
{
	std::wstringstream ss;
	ss << "XAudio2SourceVoiceProxy::DestroyVoice " << this;
	logger::log(ss.str());
	m_on_destroy(this);
}
