// json_reader.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "json_reader.h"
#include "JsonGadget.h"

#include <gadgets\gadbase.h>
#include <gadgets\shkernel.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DEFINE_SHPLUGIN_EXT;

char TVDB400_PLUGIN_NAME[APP_NAME_MAXLENGTH];

static AFX_EXTENSION_MODULE json_readerDll = { NULL, NULL };
CDynLinkLibrary* pThisDll = NULL;

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("json_reader.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(json_readerDll, hInstance))
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

		pThisDll = new CDynLinkLibrary(json_readerDll);
        // Init plugin
        FXString fName=::FxGetModuleName(json_readerDll.hModule);
        ASSERT(fName.GetLength()<APP_NAME_MAXLENGTH);
        strcpy(TVDB400_PLUGIN_NAME, fName);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("json_reader.DLL Terminating!\n");
		// Terminate the library before destructors are called
		AfxTermExtensionModule(json_readerDll);
	}
	return 1;   // OK
}

_declspec(dllexport) void __stdcall PluginEntry(LPVOID pGraphBuilder)
{
	// Add filter collection to graph builder:
	IGraphbuilder* pBuilder = (IGraphbuilder*)pGraphBuilder;
    REGISTER_RUNTIME_GADGET(JsonGadget, pBuilder);
}

_declspec(dllexport) void __stdcall PluginExit(LPVOID pGraphBuilder)
{
	// Remove filter collection from graph builder:
	IGraphbuilder* pBuilder = (IGraphbuilder*)pGraphBuilder;
	UNREGISTER_RUNTIME_GADGET(JsonGadget, pBuilder);
}

_declspec(dllexport) LPCTSTR __stdcall GetPluginName()
{
	return TVDB400_PLUGIN_NAME;
}

/////////////////////////////////////////////////////////////////////////////
// PhidgetKit_0_16_16 help

helpitem helpitems[]=
{	
  {"json_reader_basics",IDR_MHT_HELP}
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
