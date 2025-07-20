// FileX.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <gadgets\shkernel.h>
#include "FileX.h"
// #include "GridCalbrator.h"
#include "PolyTransform.h"
#include "MapperGadget.h"
#include "AverageIntense.h"
#include "CalibBySpots.h"
#include "Holoor.h"
#include "ConturAsm.h"
#include "ColorExtractor.h"
#include "TColorMatch.h"
#include "FindStraights.h"
#include "Noga.h"
#include "VideoCrossFFT.h"
#include "MPPT.h"
#include "ScreenFrag.h"
#include "SpotSymmetry.h"
#include "TwoCamsGadget.h"
#include "MPPT_Dispens.h"
#include "Tecto.h"
#include "SkewMeter.h"
#include "RulTracker.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char * THIS_FILE = __FILE__;
#endif

DEFINE_SHPLUGIN_EXT;

char TVDB400_PLUGIN_NAME[APP_NAME_MAXLENGTH];

static AFX_EXTENSION_MODULE FileXDll = { NULL, NULL };
CDynLinkLibrary* pThisDll = NULL;

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("FileX.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(FileXDll, hInstance))
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

		pThisDll = new CDynLinkLibrary(FileXDll);
        // Init plugin
        FXString fName=::FxGetModuleName(FileXDll.hModule);
        ASSERT(fName.GetLength()<APP_NAME_MAXLENGTH);
        strcpy(TVDB400_PLUGIN_NAME, fName);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("FileX.DLL Terminating!\n");
		// Terminate the library before destructors are called
		AfxTermExtensionModule(FileXDll);
	}
	return 1;   // ok
}

helpitem helpitems[]=
{
  {"ColorExtractor",IDR_ColorExtractHelp},
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
_declspec(dllexport) void __stdcall PluginEntry(LPVOID pGraphBuilder)
{
	// Add filter collection to graph builder:
	IGraphbuilder* Builder = (IGraphbuilder*)pGraphBuilder;
// 	REGISTER_RUNTIME_GADGET(GridCalibrator, Builder);
	REGISTER_RUNTIME_GADGET(PolyTransform, Builder);
	REGISTER_RUNTIME_GADGET(Mapper, Builder);
  REGISTER_RUNTIME_GADGET(AverageIntensity, Builder);
  REGISTER_RUNTIME_GADGET(Holoor, Builder);
  REGISTER_RUNTIME_GADGET(ConturAsm, Builder);
	REGISTER_RUNTIME_GADGET(ColorExtractor, Builder);
  REGISTER_RUNTIME_GADGET( TColorMatch , Builder );
  REGISTER_RUNTIME_GADGET( FindStraights , Builder );
  REGISTER_RUNTIME_GADGET( FindExtrem , Builder );
  REGISTER_RUNTIME_GADGET( Noga , Builder );
  REGISTER_RUNTIME_GADGET( CrossFFT , Builder );
  REGISTER_RUNTIME_GADGET( MPPT , Builder );
  REGISTER_RUNTIME_GADGET( ScreenFrag , Builder );
  REGISTER_RUNTIME_GADGET( SpotSymmetry , Builder );
  REGISTER_RUNTIME_GADGET( TwoChans , Builder );
  REGISTER_RUNTIME_GADGET( MPPT_Dispens , Builder );
  REGISTER_RUNTIME_GADGET( Tecto , Builder );
  REGISTER_RUNTIME_GADGET( SkewMeter , Builder );
  REGISTER_RUNTIME_GADGET( RulTracker , Builder );
}

_declspec(dllexport) void __stdcall PluginExit(LPVOID pGraphBuilder)
{
	// Remove filter collection from graph builder:
	IGraphbuilder* Builder = (IGraphbuilder*)pGraphBuilder;
// 	UNREGISTER_RUNTIME_GADGET(GridCalibrator, Builder);
	UNREGISTER_RUNTIME_GADGET(PolyTransform, Builder);
	UNREGISTER_RUNTIME_GADGET(Mapper, Builder);
	UNREGISTER_RUNTIME_GADGET(AverageIntensity, Builder);
  UNREGISTER_RUNTIME_GADGET( Holoor , Builder);
  UNREGISTER_RUNTIME_GADGET( ConturAsm , Builder);
  UNREGISTER_RUNTIME_GADGET(ColorExtractor, Builder);
  UNREGISTER_RUNTIME_GADGET( TColorMatch , Builder );
  UNREGISTER_RUNTIME_GADGET( FindStraights , Builder );
  UNREGISTER_RUNTIME_GADGET( FindExtrem , Builder );
  UNREGISTER_RUNTIME_GADGET( CrossFFT , Builder );
  UNREGISTER_RUNTIME_GADGET( MPPT , Builder );
  UNREGISTER_RUNTIME_GADGET( ScreenFrag , Builder );
  UNREGISTER_RUNTIME_GADGET( SpotSymmetry , Builder );
  UNREGISTER_RUNTIME_GADGET( TwoChans , Builder );
  UNREGISTER_RUNTIME_GADGET( MPPT_Dispens , Builder );
  UNREGISTER_RUNTIME_GADGET( Tecto , Builder );
  UNREGISTER_RUNTIME_GADGET( SkewMeter , Builder );
  UNREGISTER_RUNTIME_GADGET( RulTracker , Builder );
}

_declspec(dllexport) LPCTSTR __stdcall GetPluginName()
{
	return TVDB400_PLUGIN_NAME;
}

