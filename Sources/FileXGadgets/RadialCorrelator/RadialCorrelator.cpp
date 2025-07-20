// RadialCorrelator.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <gadgets\shkernel.h>
#include "RadialCorrelator.h"
#include "RadialCorrGadget.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DEFINE_SHPLUGIN_EXT;

char TVDB400_PLUGIN_NAME[APP_NAME_MAXLENGTH];

//helpitem helpitems[]={
//    {"CFramesCounter",IDR_CFRAMECOUNTER},
//};

static AFX_EXTENSION_MODULE RadialCorrelatorDll = { NULL, NULL };
CDynLinkLibrary* pThisDll = NULL;

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("RadialCorrelator.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(RadialCorrelatorDll, hInstance))
			return 0;

		// Insert this DLL into the resource chain
		// NOTE: If this Extension DLL is being implicitly linked to by
		//  an MFC Regular DLL (such as an ActiveX Control)
		//  instead of an MFC application, then you will want to
		//  remove this line from DllMain and put it in a separate
		//  function exported from this Extension DLL.  The Regular DLL
		//  that uses this Extension DLL should then explicitly call that
		//  function to initialize this Extension DLL.  Otherwise,
		//  the CDynLinkLibrary object will not be attached to the
		//  Regular DLL's resource chain, and serious problems will
		//  result.

		pThisDll = new CDynLinkLibrary(RadialCorrelatorDll);
        // Init plugin
        FXString fName=::FxGetModuleName(RadialCorrelatorDll.hModule);
        ASSERT(fName.GetLength()<APP_NAME_MAXLENGTH);
        strcpy(TVDB400_PLUGIN_NAME, fName);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("RadialCorrelator.DLL Terminating!\n");
		// Terminate the library before destructors are called
		AfxTermExtensionModule(RadialCorrelatorDll);
	}
	return 1;   // ok
}

///////////\\\\\\\\\\/////////\\\\\\\\\////////\\\\\\\\\

_declspec(dllexport) void __stdcall PluginEntry(LPVOID pGraphBuilder)
{
	// Add filter collection to graph builder:
	IGraphbuilder* Builder = (IGraphbuilder*)pGraphBuilder;
    REGISTER_RUNTIME_GADGET(RadialCorr, Builder);
}

_declspec(dllexport) void __stdcall PluginExit(LPVOID pGraphBuilder)
{
	// Remove filter collection from graph builder:
	IGraphbuilder* Builder = (IGraphbuilder*)pGraphBuilder;
    UNREGISTER_RUNTIME_GADGET(RadialCorr, Builder);
}

_declspec(dllexport) LPCTSTR __stdcall GetPluginName()
{
	return TVDB400_PLUGIN_NAME;
}
