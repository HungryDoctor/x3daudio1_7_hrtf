#pragma once

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <x3daudio.h>

typedef void  (STDAPIVCALLTYPE *X3DAudioCalculateFunc)(_In_reads_bytes_(X3DAUDIO_HANDLE_BYTESIZE) const X3DAUDIO_HANDLE Instance, _In_ const X3DAUDIO_LISTENER* pListener, _In_ const X3DAUDIO_EMITTER* pEmitter, UINT32 Flags, _Inout_ X3DAUDIO_DSP_SETTINGS* pDSPSettings);
typedef HRESULT(STDAPIVCALLTYPE *X3DAudioInitializeFunc)(UINT32 SpeakerChannelMask, FLOAT32 SpeedOfSound, _Out_writes_bytes_(X3DAUDIO_HANDLE_BYTESIZE) X3DAUDIO_HANDLE Instance);

struct x3daudio1_7_dll
{
	HMODULE module;
	X3DAudioCalculateFunc X3DAudioCalculate;
	X3DAudioInitializeFunc X3DAudioInitialize;
};

extern x3daudio1_7_dll x3daudio1_7;

void init_x3daudio_hook();
void destroy_x3daudio_hook();