#include "stdafx.h"
#include "XAudio2Proxy.h"
#include "graph/AudioGraphMapper.h"
#include "interop/Sound3DRegistry.h"

#include "logger.h"
#include "util.h"


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

	self->SetVoid(nullptr);
	self->set_graph_factory([](auto xaudio) { return new AudioGraphMapper(xaudio, &Sound3DRegistry::GetInstance()); });

	self->InternalFinalConstructAddRef();
	HRESULT hr = self->_AtlInitialConstruct();
	if (SUCCEEDED(hr))
		hr = self->FinalConstruct();
	if (SUCCEEDED(hr))
		hr = self->_AtlFinalConstruct();
	self->InternalFinalConstructRelease();

	if (SUCCEEDED(hr))
	{
		hr = original->QueryInterface(__uuidof(IXAudio2), reinterpret_cast<void**>(&self->m_original));
	}

	if (SUCCEEDED(hr))
		hr = self->QueryInterface(riid, ppvObject);

	if (hr != S_OK)
		delete self;

#if defined(_DEBUG)
	XAUDIO2_DEBUG_CONFIGURATION config = { 0 };
	config.LogThreadID = true;
	config.TraceMask = XAUDIO2_LOG_WARNINGS;
	//config.TraceMask = config.TraceMask | XAUDIO2_LOG_FUNC_CALLS | XAUDIO2_LOG_DETAIL;
	config.BreakMask = XAUDIO2_LOG_ERRORS;

	self->m_original->SetDebugConfiguration(&config);
#endif

	return hr;
}

HRESULT XAudio2Proxy::CreateActualDebugInstance(IUnknown* original, const IID& riid, void** ppvObject)
{
	IXAudio2 * xaudio;
	original->QueryInterface(__uuidof(IXAudio2), reinterpret_cast<void**>(&xaudio));
	*ppvObject = xaudio;
	XAUDIO2_DEBUG_CONFIGURATION config;
	config.LogThreadID = true;
	config.TraceMask = XAUDIO2_LOG_WARNINGS;
	//config.TraceMask = config.TraceMask | XAUDIO2_LOG_FUNC_CALLS | XAUDIO2_LOG_DETAIL;
	config.BreakMask = XAUDIO2_LOG_ERRORS;

	xaudio->SetDebugConfiguration(&config);
	return S_OK;
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

	m_graph.reset(m_graph_factory(m_original));

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
		*ppSourceVoice = m_graph->CreateSourceVoice(pSourceFormat, Flags, MaxFrequencyRatio, pCallback, pSendList, pEffectChain);
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
		*ppSubmixVoice = m_graph->CreateSubmixVoice(InputChannels, InputSampleRate, Flags, ProcessingStage, pSendList, pEffectChain);
		return S_OK;
	}
	catch (std::bad_alloc &)
	{
		return E_OUTOFMEMORY;
	}
}

STDMETHODIMP XAudio2Proxy::CreateMasteringVoice(IXAudio2MasteringVoice ** ppMasteringVoice, UINT32 InputChannels, UINT32 InputSampleRate, UINT32 Flags, UINT32 DeviceIndex, const XAUDIO2_EFFECT_CHAIN * pEffectChain)
{
	try
	{
		*ppMasteringVoice = m_graph->CreateMasteringVoice(InputChannels, InputSampleRate, Flags, DeviceIndex, pEffectChain);
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

void XAudio2Proxy::set_graph_factory(const AudioGraphFactory & factory)
{
	m_graph_factory = factory;
}

STDMETHODIMP_(void) XAudio2Proxy::SetDebugConfiguration(const XAUDIO2_DEBUG_CONFIGURATION * pDebugConfiguration, void * pReserved)
{
	m_original->SetDebugConfiguration(pDebugConfiguration, pReserved);
}
