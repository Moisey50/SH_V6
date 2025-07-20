// TVBMPGadget.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "TVBMPGadget.h"
#include <gadgets\shkernel.h>
#include "BMPRender.h"
#include "BMPCapture.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DEFINE_SHPLUGIN_EXT;

char TVDB400_PLUGIN_NAME[APP_NAME_MAXLENGTH];

static AFX_EXTENSION_MODULE TVBMPGadgetDll = { NULL, NULL };
CDynLinkLibrary* pThisDll = NULL;

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("TVBMPGadget.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(TVBMPGadgetDll, hInstance))
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

		pThisDll = new CDynLinkLibrary(TVBMPGadgetDll);
        // Init plugin
        FXString fName=::FxGetModuleName(TVBMPGadgetDll.hModule);
        ASSERT(fName.GetLength()<APP_NAME_MAXLENGTH);
        strcpy(TVDB400_PLUGIN_NAME, fName);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("TVBMPGadget.DLL Terminating!\n");
		// Terminate the library before destructors are called
		AfxTermExtensionModule(TVBMPGadgetDll);
	}
	return 1;   // ok
}

_declspec(dllexport) void __stdcall PluginEntry(LPVOID pGraphBuilder)
{
    // Add filter collection to graph builder:
	IGraphBuilderEx* Builder = (IGraphBuilderEx*)pGraphBuilder;
    REGISTER_RUNTIME_GADGET(BMPRender, Builder);
	REGISTER_RUNTIME_GADGET(BMPCapture, Builder);
}

_declspec(dllexport) void __stdcall PluginExit(LPVOID pGraphBuilder)
{
	// Remove filter collection from graph builder:
	IGraphbuilder* Builder = (IGraphbuilder*)pGraphBuilder;
    UNREGISTER_RUNTIME_GADGET(BMPRender, Builder);
	UNREGISTER_RUNTIME_GADGET(BMPCapture, Builder);
}

_declspec(dllexport) LPCTSTR __stdcall GetPluginName()
{
	return TVDB400_PLUGIN_NAME;
}

/////////////////////////////////////////////////////////////////////////////
// BMP Capture help

helpitem helpitems[]=
{	
    {"BMPCapture",  IDR_HTML_10000},
    {"BMPRender",   IDR_HTML_10001}
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
