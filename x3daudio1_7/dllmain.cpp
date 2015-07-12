// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#include "x3daudio-hook/x3daudio_hook.h"
#include "xaudio2-hook\creation_hook.h"

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD ul_reason_for_call,
                      LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		init_x3daudio_hook();
		install_com_hooks();
		break;
	case DLL_PROCESS_DETACH:
		uninstall_com_hooks();
		destroy_x3daudio_hook();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}
