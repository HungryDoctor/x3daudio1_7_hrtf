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

	self->SetVoid(NULL);

	self->InternalFinalConstructAddRef();
	HRESULT hr = self->_AtlInitialConstruct();
	if (SUCCEEDED(hr))
		hr = self->FinalConstruct();
	if (SUCCEEDED(hr))
		hr = self->_AtlFinalConstruct();
	self->InternalFinalConstructRelease();

	if (SUCCEEDED(hr))
		hr = original->QueryInterface(__uuidof(IXAudio2), (void**)&self->m_original);

	if (SUCCEEDED(hr))
		hr = self->QueryInterface(riid, ppvObject);

	if (hr != S_OK)
		delete self;

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
	HRESULT result = m_original->Initialize(Flags, XAudio2Processor);
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
	IXAudio2VoiceCallback *pCallback, const XAUDIO2_VOICE_SENDS *pSendList, const XAUDIO2_EFFECT_CHAIN *pEffectChain)
{
	std::wstringstream ss;
	ss << "XAudio2Proxy::CreateSourceVoice FormatChannels=" << pSourceFormat->nChannels << " FormatSampleRate=" << pSourceFormat->nSamplesPerSec << " MaxFrequencyRatio=" << MaxFrequencyRatio << " Effects=" << (pEffectChain ? std::to_wstring(pEffectChain->EffectCount) : L"nullptr") << " ";
	print_sends(ss, pSendList);
	logger::log(ss.str());

	XAUDIO2_VOICE_SENDS originalSendList = { 0 };
	if (pSendList)
		m_voice_mapper->MapSendsToOriginal(*pSendList, originalSendList);

	IXAudio2SourceVoice * original_voice = NULL;
	HRESULT result;
	if (SUCCEEDED(result = m_original->CreateSourceVoice(&original_voice, pSourceFormat, Flags, MaxFrequencyRatio, pCallback, pSendList ? &originalSendList : 0, pEffectChain)))
	{
		try
		{
			*ppSourceVoice = new XAudio2SourceVoiceProxy(m_sound3d_registry, *m_voice_mapper, original_voice, [&](auto voice)
				{
					m_voice_mapper->ForgetMapByProxy(voice);
					delete voice;
				});
			m_voice_mapper->RememberMap(original_voice, *ppSourceVoice);
			std::wstringstream ss;
			ss << "XAudio2Proxy::CreateSourceVoice succeeded " << *ppSourceVoice;
			logger::log(ss.str());
		}
		catch (std::bad_alloc&)
		{
			result = E_OUTOFMEMORY;
		}
	}

	m_voice_mapper->CleanupSends(originalSendList);

	return result;
}

STDMETHODIMP XAudio2Proxy::CreateSubmixVoice(IXAudio2SubmixVoice ** ppSubmixVoice, UINT32 InputChannels, UINT32 InputSampleRate, UINT32 Flags, UINT32 ProcessingStage, const XAUDIO2_VOICE_SENDS *pSendList, const XAUDIO2_EFFECT_CHAIN *pEffectChain)
{
	std::wstringstream ss;
	ss << "XAudio2Proxy::CreateSubmixVoice InputChannels=" << InputChannels << " InputSampleRate=" << InputSampleRate << " ProcessingStage=" << ProcessingStage << " Effects=" << (pEffectChain ? std::to_wstring(pEffectChain->EffectCount) : L"nullptr") << " ";
	print_sends(ss, pSendList);
	logger::log(ss.str());

	XAUDIO2_VOICE_SENDS originalSendList = { 0 };
	if (pSendList)
		m_voice_mapper->MapSendsToOriginal(*pSendList, originalSendList);

	IXAudio2SubmixVoice * original_voice = NULL;
	HRESULT result;
	if (SUCCEEDED(result = m_original->CreateSubmixVoice(&original_voice, InputChannels, InputSampleRate, Flags, ProcessingStage, pSendList ? &originalSendList : 0, pEffectChain)))
	{
		try
		{
			*ppSubmixVoice = new XAudio2SubmixVoiceProxy(m_sound3d_registry, *m_voice_mapper, original_voice, [&](auto voice)
				{
					m_voice_mapper->ForgetMapByProxy(voice);
					delete voice;
				});
			m_voice_mapper->RememberMap(original_voice, *ppSubmixVoice);
			std::wstringstream ss;
			ss << "XAudio2Proxy::CreateSubmixVoice succeeded " << *ppSubmixVoice;
			logger::log(ss.str());
		}
		catch (std::bad_alloc&)
		{
			result = E_OUTOFMEMORY;
		}
	}

	m_voice_mapper->CleanupSends(originalSendList);

	return result;
}

STDMETHODIMP XAudio2Proxy::CreateMasteringVoice(IXAudio2MasteringVoice ** ppMasteringVoice, UINT32 InputChannels, UINT32 InputSampleRate, UINT32 Flags, UINT32 DeviceIndex, const XAUDIO2_EFFECT_CHAIN *pEffectChain)
{
	//return m_original->CreateMasteringVoice(ppMasteringVoice, InputChannels, InputSampleRate, Flags, DeviceIndex, pEffectChain);

	std::wstringstream ss;
	ss << "XAudio2Proxy::CreateMasteringVoice InputChannels=" << InputChannels << " InputSampleRate=" << InputSampleRate << " DeviceIndex=" << DeviceIndex << " Effects=" << (pEffectChain ? std::to_wstring(pEffectChain->EffectCount) : L"nullptr") << " ";
	logger::log(ss.str());

	IXAudio2MasteringVoice * original_voice = NULL;
	HRESULT result;
	if (SUCCEEDED(result = m_original->CreateMasteringVoice(&original_voice, InputChannels, InputSampleRate, Flags, DeviceIndex, pEffectChain)))
	{
		try
		{
			*ppMasteringVoice = new XAudio2MasteringVoiceProxy(*m_voice_mapper, original_voice, [&](auto voice)
				{
					m_voice_mapper->ForgetMapByProxy(voice);
					delete voice;
				});
			m_voice_mapper->RememberMap(original_voice, *ppMasteringVoice);
			std::wstringstream ss;
			ss << "XAudio2Proxy::CreateMasteringVoice succeeded " << *ppMasteringVoice;
			logger::log(ss.str());
		}
		catch (std::bad_alloc&)
		{
			result = E_OUTOFMEMORY;
		}
	}

	return result;
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


STDMETHODIMP_(void) XAudio2Proxy::SetDebugConfiguration(const XAUDIO2_DEBUG_CONFIGURATION * pDebugConfiguration, void *pReserved)
{
	m_original->SetDebugConfiguration(pDebugConfiguration, pReserved);
}
