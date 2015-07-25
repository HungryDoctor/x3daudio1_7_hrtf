#pragma once
#include "interop/ISound3DRegistry.h"
#include <X3DAudio.h>

typedef void (STDAPIVCALLTYPE *X3DAudioInitializeFunc)(UINT32 SpeakerChannelMask, FLOAT32 SpeedOfSound, _Out_writes_bytes_(X3DAUDIO_HANDLE_BYTESIZE) X3DAUDIO_HANDLE Instance);
typedef void (STDAPIVCALLTYPE *X3DAudioCalculateFunc)(_In_reads_bytes_(X3DAUDIO_HANDLE_BYTESIZE) const X3DAUDIO_HANDLE Instance, _In_ const X3DAUDIO_LISTENER * pListener, _In_ const X3DAUDIO_EMITTER * pEmitter, UINT32 Flags, _Inout_ X3DAUDIO_DSP_SETTINGS * pDSPSettings);

struct x3daudio1_7_dll
{
	HMODULE module;
	X3DAudioInitializeFunc X3DAudioInitialize;
	X3DAudioCalculateFunc X3DAudioCalculate;
};

class X3DAudioProxy
{
public:
	X3DAudioProxy(const x3daudio1_7_dll & original, ISound3DRegistry * registry, UINT32 SpeakerChannelMask, FLOAT32 SpeedOfSound);
	~X3DAudioProxy() = default;
	void X3DAudioCalculate(const X3DAUDIO_LISTENER * pListener, const X3DAUDIO_EMITTER * pEmitter, UINT32 Flags, X3DAUDIO_DSP_SETTINGS * pDSPSettings);

	const X3DAUDIO_HANDLE& GetHandle() const;

private:
	x3daudio1_7_dll m_original;
	X3DAUDIO_HANDLE m_handle;
	ISound3DRegistry * m_registry;

private:
	X3DAudioProxy(const X3DAudioProxy &) = delete;
	X3DAudioProxy& operator=(const X3DAudioProxy &) = delete;
	float sample_volume_curve(const X3DAUDIO_EMITTER* pEmitter, const float distance);
};
