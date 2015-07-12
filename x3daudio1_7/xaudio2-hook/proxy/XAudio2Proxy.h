#pragma once

#include <atlbase.h>
#include <atlcom.h>

#include "interop/ISound3DRegistry.h"
#include "IVoiceMapper.h"
#include <xaudio2.h>
#include <map>
#include <memory>


class
	ATL_NO_VTABLE XAudio2Proxy :
	public ATL::CComObjectRootEx<ATL::CComMultiThreadModel>,
	public ATL::CComCoClass<XAudio2Proxy, &__uuidof(XAudio2)>,
	public ATL::IDispatchImpl<IXAudio2>
{
public:
	XAudio2Proxy();

	DECLARE_NO_REGISTRY()

	BEGIN_COM_MAP(XAudio2Proxy)
		COM_INTERFACE_ENTRY(IXAudio2)
		COM_INTERFACE_ENTRY(IUnknown)
		END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

public:
	HRESULT FinalConstruct();
	void FinalRelease();

public:
	HRESULT static CreateInstance(IUnknown * original, REFIID riid, void ** ppvObject);

public:

	// NAME: IXAudio2::GetDeviceCount
	// DESCRIPTION: Returns the number of audio output devices available.
	//
	// ARGUMENTS:
	//  pCount - Returns the device count.
	//
	STDMETHOD(GetDeviceCount)(__out UINT32 * pCount);

	// NAME: IXAudio2::GetDeviceDetails
	// DESCRIPTION: Returns information about the device with the given index.
	//
	// ARGUMENTS:
	//  Index - Index of the device to be queried.
	//  pDeviceDetails - Returns the device details.
	//
	STDMETHOD(GetDeviceDetails)(UINT32 Index, __out XAUDIO2_DEVICE_DETAILS * pDeviceDetails);

	// NAME: IXAudio2::Initialize
	// DESCRIPTION: Sets global XAudio2 parameters and prepares it for use.
	//
	// ARGUMENTS:
	//  Flags - Flags specifying the XAudio2 object's behavior.  Currently unused.
	//  XAudio2Processor - An XAUDIO2_PROCESSOR enumeration value that specifies
	//  the hardware thread (Xbox) or processor (Windows) that XAudio2 will use.
	//  The enumeration values are platform-specific; platform-independent code
	//  can use XAUDIO2_DEFAULT_PROCESSOR to use the default on each platform.
	//
	STDMETHOD(Initialize)(UINT32 Flags X2DEFAULT(0),
	                      XAUDIO2_PROCESSOR XAudio2Processor X2DEFAULT(XAUDIO2_DEFAULT_PROCESSOR));

	// NAME: IXAudio2::RegisterForCallbacks
	// DESCRIPTION: Adds a new client to receive XAudio2's engine callbacks.
	//
	// ARGUMENTS:
	//  pCallback - Callback interface to be called during each processing pass.
	//
	STDMETHOD(RegisterForCallbacks)(__in IXAudio2EngineCallback * pCallback);

	// NAME: IXAudio2::UnregisterForCallbacks
	// DESCRIPTION: Removes an existing receiver of XAudio2 engine callbacks.
	//
	// ARGUMENTS:
	//  pCallback - Previously registered callback interface to be removed.
	//
	STDMETHOD_(void, UnregisterForCallbacks)(__in IXAudio2EngineCallback * pCallback);

	// NAME: IXAudio2::CreateSourceVoice
	// DESCRIPTION: Creates and configures a source voice.
	//
	// ARGUMENTS:
	//  ppSourceVoice - Returns the new object's IXAudio2SourceVoice interface.
	//  pSourceFormat - Format of the audio that will be fed to the voice.
	//  Flags - XAUDIO2_VOICE flags specifying the source voice's behavior.
	//  MaxFrequencyRatio - Maximum SetFrequencyRatio argument to be allowed.
	//  pCallback - Optional pointer to a client-provided callback interface.
	//  pSendList - Optional list of voices this voice should send audio to.
	//  pEffectChain - Optional list of effects to apply to the audio data.
	//
	STDMETHOD(CreateSourceVoice)(__deref_out IXAudio2SourceVoice ** ppSourceVoice,
	                                         __in const WAVEFORMATEX * pSourceFormat,
	                                         UINT32 Flags X2DEFAULT(0),
	                                         float MaxFrequencyRatio X2DEFAULT(XAUDIO2_DEFAULT_FREQ_RATIO),
	                                         __in_opt IXAudio2VoiceCallback * pCallback X2DEFAULT(NULL),
	                                         __in_opt const XAUDIO2_VOICE_SENDS * pSendList X2DEFAULT(NULL),
	                                         __in_opt const XAUDIO2_EFFECT_CHAIN * pEffectChain X2DEFAULT(NULL));

	// NAME: IXAudio2::CreateSubmixVoice
	// DESCRIPTION: Creates and configures a submix voice.
	//
	// ARGUMENTS:
	//  ppSubmixVoice - Returns the new object's IXAudio2SubmixVoice interface.
	//  InputChannels - Number of channels in this voice's input audio data.
	//  InputSampleRate - Sample rate of this voice's input audio data.
	//  Flags - XAUDIO2_VOICE flags specifying the submix voice's behavior.
	//  ProcessingStage - Arbitrary number that determines the processing order.
	//  pSendList - Optional list of voices this voice should send audio to.
	//  pEffectChain - Optional list of effects to apply to the audio data.
	//
	STDMETHOD(CreateSubmixVoice)(__deref_out IXAudio2SubmixVoice ** ppSubmixVoice,
	                                         UINT32 InputChannels, UINT32 InputSampleRate,
	                                         UINT32 Flags X2DEFAULT(0), UINT32 ProcessingStage X2DEFAULT(0),
	                                         __in_opt const XAUDIO2_VOICE_SENDS * pSendList X2DEFAULT(NULL),
	                                         __in_opt const XAUDIO2_EFFECT_CHAIN * pEffectChain X2DEFAULT(NULL));


	// NAME: IXAudio2::CreateMasteringVoice
	// DESCRIPTION: Creates and configures a mastering voice.
	//
	// ARGUMENTS:
	//  ppMasteringVoice - Returns the new object's IXAudio2MasteringVoice interface.
	//  InputChannels - Number of channels in this voice's input audio data.
	//  InputSampleRate - Sample rate of this voice's input audio data.
	//  Flags - XAUDIO2_VOICE flags specifying the mastering voice's behavior.
	//  DeviceIndex - Identifier of the device to receive the output audio.
	//  pEffectChain - Optional list of effects to apply to the audio data.
	//
	STDMETHOD(CreateMasteringVoice)(__deref_out IXAudio2MasteringVoice ** ppMasteringVoice,
	                                            UINT32 InputChannels X2DEFAULT(XAUDIO2_DEFAULT_CHANNELS),
	                                            UINT32 InputSampleRate X2DEFAULT(XAUDIO2_DEFAULT_SAMPLERATE),
	                                            UINT32 Flags X2DEFAULT(0), UINT32 DeviceIndex X2DEFAULT(0),
	                                            __in_opt const XAUDIO2_EFFECT_CHAIN * pEffectChain X2DEFAULT(NULL));

	// NAME: IXAudio2::StartEngine
	// DESCRIPTION: Creates and starts the audio processing thread.
	//
	STDMETHOD(StartEngine)();

	// NAME: IXAudio2::StopEngine
	// DESCRIPTION: Stops and destroys the audio processing thread.
	//
	STDMETHOD_(void, StopEngine)();

	// NAME: IXAudio2::CommitChanges
	// DESCRIPTION: Atomically applies a set of operations previously tagged
	//              with a given identifier.
	//
	// ARGUMENTS:
	//  OperationSet - Identifier of the set of operations to be applied.
	//
	STDMETHOD(CommitChanges)(UINT32 OperationSet);

	// NAME: IXAudio2::GetPerformanceData
	// DESCRIPTION: Returns current resource usage details: memory, CPU, etc.
	//
	// ARGUMENTS:
	//  pPerfData - Returns the performance data structure.
	//
	STDMETHOD_(void, GetPerformanceData)(__out XAUDIO2_PERFORMANCE_DATA * pPerfData);

	// NAME: IXAudio2::SetDebugConfiguration
	// DESCRIPTION: Configures XAudio2's debug output (in debug builds only).
	//
	// ARGUMENTS:
	//  pDebugConfiguration - Structure describing the debug output behavior.
	//  pReserved - Optional parameter; must be NULL.
	//
	STDMETHOD_(void, SetDebugConfiguration)(__in_opt const XAUDIO2_DEBUG_CONFIGURATION * pDebugConfiguration,
	                                                 __in_opt __reserved void * pReserved X2DEFAULT(NULL));

public:
	void set_voice_mapper(IVoiceMapper * voice_mapper);
	void set_sound3d_registry(ISound3DRegistry * sound3d_registry);

private:
	void DestroyVoice(IXAudio2Voice * voice);

private:
	ATL::CComPtr<IXAudio2> m_original;
	std::unique_ptr<IVoiceMapper> m_voice_mapper;
	ISound3DRegistry * m_sound3d_registry;
};
