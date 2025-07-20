// VectorFields.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "VectorFields.h"
#include <gadgets\gadbase.h>
#include "TestVectorField.h"
#include "ShowVectorField.h"
#include "MotionEstimate.h"
#include "CreateMask.h"
#include "VFEnlarge.h"
#include "MotionBridgeGadget.h"
#include <gadgets\shkernel.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DEFINE_SHPLUGIN_EXT;

char TVDB400_PLUGIN_NAME[APP_NAME_MAXLENGTH];

static AFX_EXTENSION_MODULE VectorFieldsDll = { NULL, NULL };
CDynLinkLibrary* pThisDll = NULL;

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("VectorFields.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(VectorFieldsDll, hInstance))
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

		pThisDll = new CDynLinkLibrary(VectorFieldsDll);
        // Init plugin
        FXString fName=::FxGetModuleName(VectorFieldsDll.hModule);
        ASSERT(fName.GetLength()<APP_NAME_MAXLENGTH);
        strcpy(TVDB400_PLUGIN_NAME, fName);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("VectorFields.DLL Terminating!\n");
		// Terminate the library before destructors are called
		AfxTermExtensionModule(VectorFieldsDll);
	}
	return 1;   // ok
}


_declspec(dllexport) void __stdcall PluginEntry(LPVOID pGraphBuilder)
{
    // Add filter collection to graph builder:
	IGraphBuilderEx* Builder = (IGraphBuilderEx*)pGraphBuilder;

    REGISTER_RUNTIME_GADGET(TestVectorField, pGraphBuilder);
    REGISTER_RUNTIME_GADGET(ShowVectorField, pGraphBuilder);
    REGISTER_RUNTIME_GADGET(MotionEstimate, pGraphBuilder);
    REGISTER_RUNTIME_GADGET(CreateMask, pGraphBuilder);
    REGISTER_RUNTIME_GADGET(VFEnlarge, pGraphBuilder);
	REGISTER_RUNTIME_GADGET(MotionBridge, pGraphBuilder);
}

_declspec(dllexport) void __stdcall PluginExit(LPVOID pGraphBuilder)
{
	// Remove filter collection from graph builder:
	IGraphbuilder* pBuilder = (IGraphbuilder*)pGraphBuilder;
    
    UNREGISTER_RUNTIME_GADGET(TestVectorField, pGraphBuilder);
    UNREGISTER_RUNTIME_GADGET(ShowVectorField, pGraphBuilder);
    UNREGISTER_RUNTIME_GADGET(MotionEstimate, pGraphBuilder);
    UNREGISTER_RUNTIME_GADGET(CreateMask, pGraphBuilder);
    UNREGISTER_RUNTIME_GADGET(VFEnlarge, pGraphBuilder);
	UNREGISTER_RUNTIME_GADGET(MotionBridge, pGraphBuilder);
}

_declspec(dllexport) LPCTSTR __stdcall GetPluginName()
{
	return TVDB400_PLUGIN_NAME;
}

/////////////////////////////////////////////////////////////////////////////
// Vector Fields Gadgets help

helpitem helpitems[]=
{	
    {"TestVectorField",IDR_10001},
	{"ShowVectorField",IDR_10002},
    {"MotionEstimate",IDR_10003},
    {"CreateMask",IDR_10004},
    {"VFEnlarge",IDR_10005},
    {"MotionBridge",IDR_10006}
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
