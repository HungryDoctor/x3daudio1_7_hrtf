#include "stdafx.h"
#include "creation_hook.h"

#include <atlbase.h>
#include <atlcom.h>
#include <stdexcept>
#include "mhook/mhook.h"

#include "proxy\XAudio2ProxyFactory.h"
#include "proxy\XAudio2Proxy.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

//#define CREATE_AUTHENTIC_DEBUG_XAUDIO

DEFINE_CLSID(XAudio2_0, fac23f48, 31f5, 45a8, b4, 9b, 52, 25, d6, 14, 01, aa);
DEFINE_CLSID(XAudio2_1, e21a7345, eb21, 468e, be, 50, 80, 4d, b9, 7c, f7, 08);
DEFINE_CLSID(XAudio2_2, b802058a, 464a, 42db, bc, 10, b6, 50, d6, f2, 58, 6a);
DEFINE_CLSID(XAudio2_3, 4c5e637a, 16c7, 4de3, 9c, 46, 5e, d2, 21, 81, 96, 2d);
DEFINE_CLSID(XAudio2_4, 03219e78, 5bc3, 44d1, b9, 2e, f6, 3d, 89, cc, 65, 26);
DEFINE_CLSID(XAudio2_5, 4c9b6dde, 6809, 46e6, a2, 78, 9b, 6a, 97, 58, 86, 70);
DEFINE_CLSID(XAudio2_6, 3eda9b49, 2085, 498b, 9b, b2, 39, a6, 77, 84, 93, de);

//////////////////////////////////////////////////////////////////////////

namespace Hook
{
	HRESULT WINAPI CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID * ppv);
	HRESULT WINAPI CoGetClassObject(REFCLSID rclsid, DWORD dwClsContext, COSERVERINFO * pServerInfo, REFIID riid, LPVOID * ppv);
};

typedef HRESULT (WINAPI *CoCreateInstance_T)(REFCLSID, LPUNKNOWN, DWORD, REFIID, LPVOID *);
typedef HRESULT (WINAPI *CoGetClassObject_T)(REFCLSID rclsid, DWORD dwClsContext, COSERVERINFO * pServerInfo, REFIID riid, LPVOID * ppv);

namespace Original
{
	CoCreateInstance_T CoCreateInstance = nullptr;
	CoGetClassObject_T CoGetClassObject = nullptr;
}

//////////////////////////////////////////////////////////////////////////

struct FunctionInfo
{
	char * FunctionModule;
	char * FunctionName;
	void ** OriginalFunction;
	void * HookFunction;
};

FunctionInfo g_Functions[] =
{
	{"ole32.dll", "CoCreateInstance", reinterpret_cast<void**>(&Original::CoCreateInstance), static_cast<void*>(Hook::CoCreateInstance)},
	{"ole32.dll", "CoGetClassObject", reinterpret_cast<void**>(&Original::CoGetClassObject), static_cast<void*>(Hook::CoGetClassObject)}
};

const size_t g_FunctionsCount = sizeof(g_Functions) / sizeof(FunctionInfo);

//////////////////////////////////////////////////////////////////////////

HRESULT WINAPI Hook::CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID * ppv)
{
#if !defined(BYPASS_COM_HOOK)
	if (
		rclsid == __uuidof(XAudio2) || rclsid == __uuidof(XAudio2_6) || rclsid == __uuidof(XAudio2_5) || rclsid == __uuidof(XAudio2_4) || rclsid == __uuidof(XAudio2_3) || rclsid == __uuidof(XAudio2_2) || rclsid == __uuidof(XAudio2_1) || rclsid == __uuidof(XAudio2_0)
		|| rclsid == __uuidof(XAudio2_Debug)
		)
	{
		if (pUnkOuter)
			return CLASS_E_NOAGGREGATION;

		ATL::CComPtr<IUnknown> originalObject;
#if defined(_DEBUG)
		HRESULT hr = Original::CoCreateInstance(__uuidof(XAudio2_Debug), pUnkOuter, dwClsContext, riid, reinterpret_cast<void**>(&originalObject));
#else
		HRESULT hr = Original::CoCreateInstance(__uuidof(XAudio2), pUnkOuter, dwClsContext, riid, reinterpret_cast<void**>(&originalObject));
#endif
		if (FAILED(hr))
			return hr;
		
#if defined(CREATE_AUTHENTIC_DEBUG_XAUDIO)
		return XAudio2Proxy::CreateActualDebugInstance(originalObject.Detach(), riid, ppv);
#else
		return XAudio2Proxy::CreateInstance(originalObject.Detach(), riid, ppv);
#endif
	}
#endif

	return Original::CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
}

HRESULT WINAPI Hook::CoGetClassObject(REFCLSID rclsid, DWORD dwClsContext, COSERVERINFO * pServerInfo, REFIID riid, LPVOID * ppv)
{
#if !defined(BYPASS_COM_HOOK)
	if (riid == IID_IClassFactory)
	{
		ATL::CComPtr<IClassFactory> originalFactory;
#if defined(_DEBUG)
		HRESULT hr = Original::CoGetClassObject(__uuidof(XAudio2_Debug), dwClsContext, pServerInfo, riid, reinterpret_cast<void**>(&originalFactory));
#else
		HRESULT hr = Original::CoGetClassObject(__uuidof(XAudio2), dwClsContext, pServerInfo, riid, reinterpret_cast<void**>(&originalFactory));
#endif
		if (FAILED(hr))
			return hr;

		return XAudio2ProxyFactory::CreateFactory(originalFactory, ppv);
	}
#endif

	return Original::CoGetClassObject(rclsid, dwClsContext, pServerInfo, riid, ppv);
}

//////////////////////////////////////////////////////////////////////////

void install_com_hooks()
{
	for (size_t i = 0; i < g_FunctionsCount; ++i)
	{
		if (!GetModuleHandleA(g_Functions[i].FunctionModule))
			throw std::runtime_error("Cannot find the module ");

		*g_Functions[i].OriginalFunction = GetProcAddress(GetModuleHandleA(g_Functions[i].FunctionModule), g_Functions[i].FunctionName);

		if (*g_Functions[i].OriginalFunction == nullptr)
			throw std::runtime_error("Cannot find the function");

		if (!Mhook_SetHook(g_Functions[i].OriginalFunction, g_Functions[i].HookFunction))
			throw std::runtime_error("Cannot set hook on the function");
	}
}

void uninstall_com_hooks()
{
	BOOL failed = false;

	for (size_t i = 0; i < g_FunctionsCount; ++i)
	{
		if (g_Functions[i].OriginalFunction)
			failed = failed || !Mhook_Unhook(g_Functions[i].OriginalFunction);
	}

	if (failed)
		throw std::runtime_error("uninstall_com_hooks was failed to remove one or more hooks");
}
