#include "stdafx.h"
#include "XAudio2Proxy.h"

#include "XAudio2SourceVoiceProxy.h"
#include "XAudio2SubmixVoiceProxy.h"
#include "XAudio2MasteringVoiceProxy.h"

#include "interop/Sound3DRegistry.h"
#include "VoiceMapper.h"

#include "logger.h"
#include "util.h"

#include <sstream>
#include <vector>

XAudio2Proxy::XAudio2Proxy()
{
	logger::log("Constructing XAudio2Proxy");
}

HRESULT XAudio2Proxy::FinalConstruct()
{
	return S_OK;
}

void XAudio2Proxy::FinalRelease()
{
}

HRESULT XAudio2Proxy::CreateInstance(IUnknown * original, REFIID riid, void ** ppvObject)
{
	auto self = new ATL::CComObjectNoLock<XAudio2Proxy>;

	self->set_sound3d_registry(&Sound3DRegistry::GetInstance());
	self->set_voice_mapper(new VoiceMapper);

	self->SetVoid(nullptr);

	self->InternalFinalConstructAddRef();
	HRESULT hr = self->_AtlInitialConstruct();
	if (SUCCEEDED(hr))
		hr = self->FinalConstruct();
	if (SUCCEEDED(hr))
		hr = self->_AtlFinalConstruct();
	self->InternalFinalConstructRelease();

	if (SUCCEEDED(hr))
		hr = original->QueryInterface(__uuidof(IXAudio2), reinterpret_cast<void**>(&self->m_original));

	if (SUCCEEDED(hr))
		hr = self->QueryInterface(riid, ppvObject);

	if (hr != S_OK)
		delete self;

	XAUDIO2_DEBUG_CONFIGURATION config = { 0 };
	config.LogThreadID = true;
	config.TraceMask = XAUDIO2_LOG_FUNC_CALLS | XAUDIO2_LOG_DETAIL | XAUDIO2_LOG_WARNINGS;
	config.BreakMask = XAUDIO2_LOG_ERRORS;

	//self->m_original->SetDebugConfiguration(&config);

	return hr;
}

STDMETHODIMP XAudio2Proxy::GetDeviceCount(UINT32 * pCount)
{
	return m_original->GetDeviceCount(pCount);
}

STDMETHODIMP XAudio2Proxy::GetDeviceDetails(UINT32 Index, XAUDIO2_DEVICE_DETAILS * pDeviceDetails)
{
	return m_original->GetDeviceDetails(Index, pDeviceDetails);
}

STDMETHODIMP XAudio2Proxy::Initialize(UINT32 Flags, XAUDIO2_PROCESSOR XAudio2Processor)
{
	logger::log("Initializing XAudio2Proxy");
	HRESULT result = m_original->Initialize(Flags | XAUDIO2_DEBUG_ENGINE, XAudio2Processor);
	logger::log(L"(Not)constructed XAudio2Proxy with result " + std::to_wstring(result));
	return result;
}

STDMETHODIMP XAudio2Proxy::RegisterForCallbacks(IXAudio2EngineCallback * pCallback)
{
	return m_original->RegisterForCallbacks(pCallback);
}

STDMETHODIMP_(void) XAudio2Proxy::UnregisterForCallbacks(IXAudio2EngineCallback * pCallback)
{
	m_original->UnregisterForCallbacks(pCallback);
}

STDMETHODIMP XAudio2Proxy::CreateSourceVoice(IXAudio2SourceVoice ** ppSourceVoice, const WAVEFORMATEX * pSourceFormat, UINT32 Flags, float MaxFrequencyRatio,
                                             IXAudio2VoiceCallback * pCallback, const XAUDIO2_VOICE_SENDS * pSendList, const XAUDIO2_EFFECT_CHAIN * pEffectChain)
{
	try
	{
		*ppSourceVoice = new XAudio2SourceVoiceProxy(
			m_original.p,
			m_sound3d_registry,
			m_voice_mapper.get(),
			[&](auto voice)
			{
				m_voice_mapper->ForgetMapByProxy(voice);
				delete voice;
			},
			pSourceFormat, Flags, MaxFrequencyRatio, pCallback, pSendList, pEffectChain);
		return S_OK;
	}
	catch (std::bad_alloc &)
	{
		return E_OUTOFMEMORY;
	}
}

STDMETHODIMP XAudio2Proxy::CreateSubmixVoice(IXAudio2SubmixVoice ** ppSubmixVoice, UINT32 InputChannels, UINT32 InputSampleRate, UINT32 Flags, UINT32 ProcessingStage, const XAUDIO2_VOICE_SENDS * pSendList, const XAUDIO2_EFFECT_CHAIN * pEffectChain)
{
	try
	{
		*ppSubmixVoice = new XAudio2SubmixVoiceProxy(
			m_original.p,
			m_sound3d_registry,
			m_voice_mapper.get(),
			[&](auto voice)
			{
				m_voice_mapper->ForgetMapByProxy(voice);
				delete voice;
			},
			InputChannels, InputSampleRate, Flags, ProcessingStage, pSendList, pEffectChain);
		return S_OK;
	}
	catch (std::bad_alloc &)
	{
		return E_OUTOFMEMORY;
	}
}

STDMETHODIMP XAudio2Proxy::CreateMasteringVoice(IXAudio2MasteringVoice ** ppMasteringVoice, UINT32 InputChannels, UINT32 InputSampleRate, UINT32 Flags, UINT32 DeviceIndex, const XAUDIO2_EFFECT_CHAIN * pEffectChain)
{
	//return m_original->CreateMasteringVoice(ppMasteringVoice, InputChannels, InputSampleRate, Flags, DeviceIndex, pEffectChain);

	try
	{
		*ppMasteringVoice = new XAudio2MasteringVoiceProxy(
			m_original.p,
			m_voice_mapper.get(),
			[&](auto voice)
			{
				m_voice_mapper->ForgetMapByProxy(voice);
				delete voice;
			},
			InputChannels, InputSampleRate, Flags, DeviceIndex, pEffectChain);
		return S_OK;
	}
	catch (std::bad_alloc &)
	{
		return E_OUTOFMEMORY;
	}
}

STDMETHODIMP XAudio2Proxy::StartEngine()
{
	return m_original->StartEngine();
}

STDMETHODIMP_(void) XAudio2Proxy::StopEngine()
{
	m_original->StopEngine();
}

STDMETHODIMP XAudio2Proxy::CommitChanges(UINT32 OperationSet)
{
	return m_original->CommitChanges(OperationSet);
}

STDMETHODIMP_(void) XAudio2Proxy::GetPerformanceData(XAUDIO2_PERFORMANCE_DATA * pPerfData)
{
	return m_original->GetPerformanceData(pPerfData);
}

void XAudio2Proxy::set_voice_mapper(IVoiceMapper * voice_mapper)
{
	m_voice_mapper = std::unique_ptr<IVoiceMapper>(voice_mapper);
}

void XAudio2Proxy::set_sound3d_registry(ISound3DRegistry * sound3d_registry)
{
	m_sound3d_registry = sound3d_registry;
}

void XAudio2Proxy::DestroyVoice(IXAudio2Voice * voice)
{
	m_voice_mapper->ForgetMapByProxy(voice);
	delete voice;
}


STDMETHODIMP_(void) XAudio2Proxy::SetDebugConfiguration(const XAUDIO2_DEBUG_CONFIGURATION * pDebugConfiguration, void * pReserved)
{
	m_original->SetDebugConfiguration(pDebugConfiguration, pReserved);
}
