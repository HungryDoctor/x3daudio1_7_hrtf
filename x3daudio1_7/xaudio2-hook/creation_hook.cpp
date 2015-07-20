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
	if (rclsid == __uuidof(XAudio2) || rclsid == __uuidof(XAudio2_6))
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

		return XAudio2Proxy::CreateInstance(originalObject, riid, ppv);
	}

	return Original::CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
}

HRESULT WINAPI Hook::CoGetClassObject(REFCLSID rclsid, DWORD dwClsContext, COSERVERINFO * pServerInfo, REFIID riid, LPVOID * ppv)
{
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
