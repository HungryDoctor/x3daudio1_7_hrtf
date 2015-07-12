#include "stdafx.h"
#include "x3daudio_hook.h"
#include "X3DAudioProxy.h"
#include "interop/Sound3DRegistry.h"
#include "logger.h"
#include <string>
#include <vector>
#include <memory>

x3daudio1_7_dll x3daudio1_7;
std::vector<std::unique_ptr<X3DAudioProxy>> _proxies; // there's no [simple] way to destroy X3DAudioProxy because there's nop such procedure in the API. So the best we can do is simply destruct when dll is unloaded.

struct X3DAUDIO_CUSTOM
{
	union
	{
		X3DAudioProxy * proxy;
		X3DAUDIO_HANDLE handle; // we won't use it anyway...
	};
};

typedef X3DAUDIO_CUSTOM * X3DAUDIO_CUSTOM_HANDLE;

namespace Hook
{
	extern "C" __declspec(dllexport) void STDAPIVCALLTYPE X3DAudioInitialize(UINT32 SpeakerChannelMask, FLOAT32 SpeedOfSound, X3DAUDIO_CUSTOM_HANDLE Instance)
	{
		logger::log("X3DAudioInitialize");
		auto ptr = std::unique_ptr<X3DAudioProxy>(new X3DAudioProxy(x3daudio1_7, &Sound3DRegistry::GetInstance(), SpeakerChannelMask, SpeedOfSound));
		Instance->proxy = ptr.get();

		_proxies.push_back(std::move(ptr)); // be careful. std::move invalidates ptr.
	}

	extern "C" __declspec(dllexport) void STDAPIVCALLTYPE X3DAudioCalculate(const X3DAUDIO_CUSTOM_HANDLE Instance, _In_ const X3DAUDIO_LISTENER * pListener, _In_ const X3DAUDIO_EMITTER * pEmitter, UINT32 Flags, _Inout_ X3DAUDIO_DSP_SETTINGS * pDSPSettings)
	{
		Instance->proxy->X3DAudioCalculate(pListener, pEmitter, Flags, pDSPSettings);
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
		MessageBoxW(0, L"Unuable to load x3daudio1_7.dll!", L"X3DAudio1_7 DLL Proxy", MB_ICONERROR);
		ExitProcess(0);
	}
	x3daudio1_7.X3DAudioCalculate = (X3DAudioCalculateFunc)GetProcAddress(x3daudio1_7.module, "X3DAudioCalculate");
	x3daudio1_7.X3DAudioInitialize = (X3DAudioInitializeFunc)GetProcAddress(x3daudio1_7.module, "X3DAudioInitialize");
}

void destroy_x3daudio_hook()
{
	FreeLibrary(x3daudio1_7.module);
}
