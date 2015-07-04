#include "stdafx.h"
#include "x3daudio_hook.h"
#include <string>

#define APP_NAME    L"X3DAudio1_7 DLL Proxy"
#define APP_VERSION L"1.0 [28.06.2015]"

x3daudio1_7_dll x3daudio1_7;

namespace Hook
{
	extern "C" __declspec(dllexport) void STDAPIVCALLTYPE X3DAudioCalculate(_In_reads_bytes_(X3DAUDIO_HANDLE_BYTESIZE) const X3DAUDIO_HANDLE Instance, _In_ const X3DAUDIO_LISTENER* pListener, _In_ const X3DAUDIO_EMITTER* pEmitter, UINT32 Flags, _Inout_ X3DAUDIO_DSP_SETTINGS* pDSPSettings)
	{
		x3daudio1_7.X3DAudioCalculate(Instance, pListener, pEmitter, Flags, pDSPSettings);
		pDSPSettings->DopplerFactor = 0.9f;
		OutputDebugStringW(L"X3DAudioCalculate\r\n");
	}

	extern "C" __declspec(dllexport) HRESULT STDAPIVCALLTYPE X3DAudioInitialize(UINT32 SpeakerChannelMask, FLOAT32 SpeedOfSound, _Out_writes_bytes_(X3DAUDIO_HANDLE_BYTESIZE) X3DAUDIO_HANDLE Instance)
	{
		return x3daudio1_7.X3DAudioInitialize(SpeakerChannelMask, SpeedOfSound, Instance);
	}
}

void init_x3daudio_hook()
{
	wchar_t path[MAX_PATH];
	GetSystemDirectory(path, MAX_PATH);
	std::wstring original_library_path = std::wstring(path) + L"\\x3daudio1_7.dll";

	x3daudio1_7.module = LoadLibrary(original_library_path.c_str());
	if (x3daudio1_7.module == nullptr)
	{
		MessageBoxW(0, L"Unuable to load x3daudio1_7.dll!", APP_NAME, MB_ICONERROR);
		ExitProcess(0);
	}
	x3daudio1_7.X3DAudioCalculate = (X3DAudioCalculateFunc)GetProcAddress(x3daudio1_7.module, "X3DAudioCalculate");
	x3daudio1_7.X3DAudioInitialize = (X3DAudioInitializeFunc)GetProcAddress(x3daudio1_7.module, "X3DAudioInitialize");
}

void destroy_x3daudio_hook()
{
	FreeLibrary(x3daudio1_7.module);
}