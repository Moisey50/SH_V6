// WavGadgets.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "wavgadgets.h"
#include "wavgadgetsimpl.h"
#include "wavoscillograph.h"
#include <gadgets\shkernel.h>
#include "wavspectrum.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DEFINE_SHPLUGIN_EXT;

char TVDB400_PLUGIN_NAME[APP_NAME_MAXLENGTH];

static AFX_EXTENSION_MODULE WaveGadgetDll = { NULL, NULL };
CDynLinkLibrary* pThisDll = NULL;

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("WaveGadget.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(WaveGadgetDll, hInstance))
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

		pThisDll = new CDynLinkLibrary(WaveGadgetDll);
        // Init plugin
        FXString fName=::FxGetModuleName(WaveGadgetDll.hModule);
        ASSERT(fName.GetLength()<APP_NAME_MAXLENGTH);
        strcpy(TVDB400_PLUGIN_NAME, fName);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("WaveGadget.DLL Terminating!\n");
		// Terminate the library before destructors are called
		AfxTermExtensionModule(WaveGadgetDll);
	}
	return 1;   // ok
}

_declspec(dllexport) void __stdcall PluginEntry(LPVOID pGraphBuilder)
{
	// Add filter collection to graph builder:
	IGraphBuilderEx* Builder = (IGraphBuilderEx*)pGraphBuilder;
	REGISTER_RUNTIME_GADGET(WaveCapture, Builder);
	REGISTER_RUNTIME_GADGET(WavFileCapture, Builder);
	REGISTER_RUNTIME_GADGET(WaveSpeaker, Builder);
	REGISTER_RUNTIME_GADGET(WavOscillograph, Builder);
    REGISTER_RUNTIME_GADGET(WavSpectrum, Builder);
	REGISTER_RUNTIME_GADGET(ReIm2AmpPh, Builder);
	REGISTER_RUNTIME_GADGET(Spectrum2Wave, Builder);
}

_declspec(dllexport) void __stdcall PluginExit(LPVOID pGraphBuilder)
{
	// Remove filter collection from graph builder:
	IGraphbuilder* Builder = (IGraphbuilder*)pGraphBuilder;
    UNREGISTER_RUNTIME_GADGET(WaveCapture, Builder);
	UNREGISTER_RUNTIME_GADGET(WavFileCapture, Builder);
	UNREGISTER_RUNTIME_GADGET(WaveSpeaker, Builder);
	UNREGISTER_RUNTIME_GADGET(WavOscillograph, Builder);
    UNREGISTER_RUNTIME_GADGET(WavSpectrum, Builder);
	UNREGISTER_RUNTIME_GADGET(ReIm2AmpPh, Builder);
	UNREGISTER_RUNTIME_GADGET(Spectrum2Wave, Builder);
}

_declspec(dllexport) LPCTSTR __stdcall GetPluginName()
{
	return TVDB400_PLUGIN_NAME;
}

/////////////////////////////////////////////////////////////////////////////
// Wave Gadgets help

helpitem helpitems[]=
{	
    {"WaveCapture",IDR_10001},
	{"WavFileCapture",IDR_10002},
    {"WaveSpeaker",IDR_10003},
    {"WavOscillograph",IDR_10004},
    {"WavSpectrum",IDR_10005},
    {"ReIm2AmpPh",IDR_10006},
    {"Spectrum2Wave",IDR_10007}
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
