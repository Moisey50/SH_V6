// TxtGadgets.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "TxtGadgets.h"
#include "TextGadgetsImpl.h"
#include "COMCapture.h"
#include "NMEAParser.h"
#include "MultiPinProtoGadget.h"
#include "TextFormat.h"
#include "HtmlRequest.h"
#include "MatchTemplate.h"
#include <gadgets\shkernel.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DEFINE_SHPLUGIN_EXT;

char TVDB400_PLUGIN_NAME[APP_NAME_MAXLENGTH];

static AFX_EXTENSION_MODULE TxtGadgetDll = { NULL, NULL };
CDynLinkLibrary* pThisDll = NULL;

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("TxtGadget.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(TxtGadgetDll, hInstance))
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

		pThisDll = new CDynLinkLibrary(TxtGadgetDll);
        // Init plugin
        FXString fName=::FxGetModuleName(TxtGadgetDll.hModule);
        ASSERT(fName.GetLength()<APP_NAME_MAXLENGTH);
        strcpy(TVDB400_PLUGIN_NAME, fName);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("TxtGadget.DLL Terminating!\n");
		// Terminate the library before destructors are called
		AfxTermExtensionModule(TxtGadgetDll);
	}
	return 1;   // ok
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CTxtGadgetsApp object


_declspec(dllexport) void __stdcall PluginEntry(LPVOID pGraphBuilder)
{
	// Add filter collection to graph builder:
	IGraphbuilder* Builder = (IGraphbuilder*)pGraphBuilder;
    REGISTER_RUNTIME_GADGET(TextWriter   , Builder);
    REGISTER_RUNTIME_GADGET(TextFormat   , Builder);
    REGISTER_RUNTIME_GADGET(COMCapture   , Builder);
    REGISTER_RUNTIME_GADGET(SimpleTextRender, Builder);
    REGISTER_RUNTIME_GADGET(TextRender   , Builder);
    REGISTER_RUNTIME_GADGET(NMEAParser   , Builder);
    REGISTER_RUNTIME_GADGET(HtmlRequest  , Builder);
    REGISTER_RUNTIME_GADGET(MatchTemplate, Builder);
}

_declspec(dllexport) void __stdcall PluginExit(LPVOID pGraphBuilder)
{
	// Remove filter collection from graph builder:
	IGraphbuilder* Builder = (IGraphbuilder*)pGraphBuilder;

    UNREGISTER_RUNTIME_GADGET(TextWriter   , Builder);
    UNREGISTER_RUNTIME_GADGET(TextFormat   , Builder);
    UNREGISTER_RUNTIME_GADGET(COMCapture   , Builder);
	UNREGISTER_RUNTIME_GADGET(SimpleTextRender, Builder);
    UNREGISTER_RUNTIME_GADGET(TextRender   , Builder);
    UNREGISTER_RUNTIME_GADGET(NMEAParser   , Builder);
    UNREGISTER_RUNTIME_GADGET(HtmlRequest  , Builder);
    UNREGISTER_RUNTIME_GADGET(MatchTemplate, Builder);
}
_declspec(dllexport) LPCTSTR __stdcall GetPluginName()
{
	return TVDB400_PLUGIN_NAME;
}
/////////////////////////////////////////////////////////////////////////////
// Text Gadgets help

helpitem helpitems[]=
{
	{"TextWriter",IDR_10000},
    {"TextFormat",IDR_10001},
    {"COMCapture",IDR_ComCaptureHelp},
	  {"SimpleTextRender",IDR_10003},
    {"TextRender",IDR_10004},
    {"NMEAParser",IDR_10005},
    {"HtmlRequest",IDR_10006},
    {"MatchTemplate",IDR_10007}

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



