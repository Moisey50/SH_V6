// FXFC.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <fxfc\fxfc.h>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
        TRACE("FXFC::DllMain(DLL_PROCESS_ATTACH)\n");
        break;
	case DLL_THREAD_ATTACH:
        //TRACE("FXFC::DllMain(DLL_THREAD_ATTACH)\n");
        break;
	case DLL_THREAD_DETACH:
        //TRACE("FXFC::DllMain(DLL_THREAD_DETACH)\n");
        break;
	case DLL_PROCESS_DETACH:
        TRACE("FXFC::DllMain(DLL_PROCESS_DETACH)\n");
        break;
	}
	return TRUE;
}

