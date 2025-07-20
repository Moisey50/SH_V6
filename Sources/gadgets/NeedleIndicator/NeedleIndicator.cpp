// NeedleIndicator.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "needleindicator.h"
#include <gadgets\shkernel.h>
#include "locator.h"
#include "needledetector.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DEFINE_SHPLUGIN_EXT;

static AFX_EXTENSION_MODULE NeedleIndicatorDll = { NULL, NULL };
CDynLinkLibrary* pThisDll = NULL;

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("NeedleIndicator.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(NeedleIndicatorDll, hInstance))
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

		pThisDll = new CDynLinkLibrary(NeedleIndicatorDll);
        // Init plugin
        FXString fName=::FxGetModuleName(NeedleIndicatorDll.hModule);
        ASSERT(fName.GetLength()<APP_NAME_MAXLENGTH);
        strcpy(TVDB400_PLUGIN_NAME, fName);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("NeedleIndicator.DLL Terminating!\n");
		// Terminate the library before destructors are called
		AfxTermExtensionModule(NeedleIndicatorDll);
	}
	return 1;   // ok
}


char TVDB400_PLUGIN_NAME[APP_NAME_MAXLENGTH];


_declspec(dllexport) void __stdcall PluginEntry(LPVOID pGraphBuilder)
{
// Add filter collection to graph builder:
	IGraphBuilderEx* Builder = (IGraphBuilderEx*)pGraphBuilder;
	REGISTER_RUNTIME_GADGET(Locator, Builder);
	REGISTER_RUNTIME_GADGET(NeedleDetector, Builder);
}

_declspec(dllexport) void __stdcall PluginExit(LPVOID pGraphBuilder)
{
	// Remove filter collection from graph builder:
	IGraphbuilder* Builder = (IGraphbuilder*)pGraphBuilder;
    UNREGISTER_RUNTIME_GADGET(Locator, Builder);
	UNREGISTER_RUNTIME_GADGET(NeedleDetector, Builder);
}

_declspec(dllexport) LPCTSTR __stdcall GetPluginName()
{
	return TVDB400_PLUGIN_NAME;
}
helpitem helpitems[]={
    {"Locator",         IDR_HTML_10000},
    {"NeedleDetector",  IDR_HTML_10001}
};

_declspec(dllexport) unsigned __stdcall GetHelpItem(LPCTSTR classname)
{
    int itemsnmb=(sizeof(helpitems)/sizeof(helpitem));
    for (int i=0; i<itemsnmb; i++)
    {
        if (strcmp(helpitems[i].classname,classname)==0)
            return helpitems[i].helpID;
    }
    return -1;
}
