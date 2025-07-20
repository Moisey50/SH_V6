// TVGeneric.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "TVGeneric.h"
#include "GadgetsLibrary.h"
#include "LPTBitRenderer.h"
#include "SwitchFilter.h"
#include "AggregatorGadget.h"
#include "DisassembleGadget.h"
#include "Gadgets.h"
#include "Uniselector.h"
#include "NullGadget.h"
#include "ControlPinTester.h"
#include "Sink.h"
#include "BufferGadget.h"
#include "GenericCapture.h"
#include "LabelFilter.h"
#include "GenericRenderer.h"
#include <gadgets\shkernel.h>
#include "ArrayToFigure.h"
#include "SynthRGB.h"
#include "Calculator.h"
#include "Spool.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DEFINE_SHPLUGIN_EXT;

char TVDB400_PLUGIN_NAME[APP_NAME_MAXLENGTH];

static AFX_EXTENSION_MODULE TVGenericDll = { NULL, NULL };
CDynLinkLibrary* pThisDll = NULL;

extern "C" int APIENTRY
  DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
  // Remove this if you use lpReserved
  UNREFERENCED_PARAMETER(lpReserved);

  if (dwReason == DLL_PROCESS_ATTACH)
  {
    TRACE0("TVGeneric.DLL Initializing!\n");

    // Extension DLL one-time initialization
    if (!AfxInitExtensionModule(TVGenericDll, hInstance))
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

    pThisDll = new CDynLinkLibrary(TVGenericDll);
    // Init plugin
    FXString fName=::FxGetModuleName(TVGenericDll.hModule);
    ASSERT(fName.GetLength()<APP_NAME_MAXLENGTH);
    strcpy(TVDB400_PLUGIN_NAME, fName);
  }
  else if (dwReason == DLL_PROCESS_DETACH)
  {
    TRACE0("TVGeneric.DLL Terminating!\n");
    // Terminate the library before destructors are called
    AfxTermExtensionModule(TVGenericDll);
  }
  return 1;   // ok
}

_declspec(dllexport) void __stdcall PluginEntry(LPVOID pGraphBuilder)
{
  // Add filter collection to graph builder:
  IGraphbuilder* Builder = (IGraphbuilder*)pGraphBuilder;

  REGISTER_RUNTIME_GADGET(GenericCapture, Builder);
  REGISTER_RUNTIME_GADGET(ControlPinTester, Builder);
  REGISTER_RUNTIME_GADGET(CaptureTimer, Builder);
  REGISTER_RUNTIME_GADGET(FramesCounter, Builder);
  REGISTER_RUNTIME_GADGET(CompareFilter, Builder);
  REGISTER_RUNTIME_GADGET(Switch, Builder);
  REGISTER_RUNTIME_GADGET(Aggregator, Builder);
  REGISTER_RUNTIME_GADGET(Disassemble, Builder);
  REGISTER_RUNTIME_GADGET(RenderID, Builder);
  REGISTER_RUNTIME_GADGET(Delay, Builder);
  REGISTER_RUNTIME_GADGET(ChangeID, Builder);
  REGISTER_RUNTIME_GADGET(RenderFrameRate, Builder);
  REGISTER_RUNTIME_GADGET( RenderContainer , Builder );
  REGISTER_RUNTIME_GADGET( Meas_dT , Builder );
  REGISTER_RUNTIME_GADGET( Uniselector , Builder );
  REGISTER_RUNTIME_GADGET(Null, Builder);
  REGISTER_RUNTIME_GADGET(NullIzo, Builder);
  REGISTER_RUNTIME_GADGET(Sink, Builder);
  REGISTER_RUNTIME_GADGET(Buffer, Builder);
  REGISTER_RUNTIME_GADGET(LPTBitRender, Builder);
  REGISTER_RUNTIME_GADGET(LabelFilter, Builder);
  REGISTER_RUNTIME_GADGET(GenericRender, Builder);
  //	REGISTER_RUNTIME_GADGET(SerializeTest, Builder);
  REGISTER_RUNTIME_GADGET(Cap, Builder);
  REGISTER_RUNTIME_GADGET(ArrayToFigure, Builder);
  REGISTER_RUNTIME_GADGET(SynthRGB, Builder);
  REGISTER_RUNTIME_GADGET(Calculator, Builder);
  REGISTER_RUNTIME_GADGET(Spool, Builder);
  REGISTER_RUNTIME_GADGET(SplitConts, Builder);
  REGISTER_RUNTIME_GADGET( Decimator , Builder );
  REGISTER_RUNTIME_GADGET( GenRange , Builder );
  REGISTER_RUNTIME_GADGET( Port , Builder );
  // Add filter register string just above.
}

_declspec(dllexport) void __stdcall PluginExit(LPVOID pGraphBuilder)
{
  // Remove filter collection from graph builder:
  IGraphbuilder* Builder = (IGraphbuilder*)pGraphBuilder;

  UNREGISTER_RUNTIME_GADGET(GenericCapture, Builder);
  UNREGISTER_RUNTIME_GADGET(ControlPinTester, Builder);
  UNREGISTER_RUNTIME_GADGET(CaptureTimer, Builder);
  UNREGISTER_RUNTIME_GADGET(FramesCounter, Builder); //+
  UNREGISTER_RUNTIME_GADGET(CompareFilter, Builder);
  UNREGISTER_RUNTIME_GADGET(Switch, Builder);
  UNREGISTER_RUNTIME_GADGET(Aggregator, Builder);
  UNREGISTER_RUNTIME_GADGET(Disassemble, Builder);
  UNREGISTER_RUNTIME_GADGET(LPTBitRender, Builder);
  UNREGISTER_RUNTIME_GADGET(RenderID, Builder);
  UNREGISTER_RUNTIME_GADGET(Delay, Builder);
  UNREGISTER_RUNTIME_GADGET(ChangeID, Builder);
  UNREGISTER_RUNTIME_GADGET(RenderFrameRate, Builder);
  UNREGISTER_RUNTIME_GADGET(RenderContainer, Builder);
  UNREGISTER_RUNTIME_GADGET( Meas_dT , Builder );
  UNREGISTER_RUNTIME_GADGET( Uniselector , Builder );
  UNREGISTER_RUNTIME_GADGET(Null, Builder);
  UNREGISTER_RUNTIME_GADGET(NullIzo, Builder);
  UNREGISTER_RUNTIME_GADGET(Sink, Builder);
  UNREGISTER_RUNTIME_GADGET(Buffer, Builder);
  UNREGISTER_RUNTIME_GADGET(LabelFilter, Builder);
  UNREGISTER_RUNTIME_GADGET(GenericRender, Builder);
  //	UNREGISTER_RUNTIME_GADGET(SerializeTest, Builder);
  UNREGISTER_RUNTIME_GADGET(Cap, Builder);
  UNREGISTER_RUNTIME_GADGET(ArrayToFigure, Builder);
  UNREGISTER_RUNTIME_GADGET(SynthRGB, Builder);
  UNREGISTER_RUNTIME_GADGET(Calculator, Builder);
  UNREGISTER_RUNTIME_GADGET(Spool, Builder);
  UNREGISTER_RUNTIME_GADGET(SplitConts, Builder);
  UNREGISTER_RUNTIME_GADGET(Decimator, Builder);
  UNREGISTER_RUNTIME_GADGET( GenRange , Builder );
  UNREGISTER_RUNTIME_GADGET( Port , Builder );
  // Add filter unregister string just above.
}

_declspec(dllexport) LPCTSTR __stdcall GetPluginName()
{
  return TVDB400_PLUGIN_NAME;
}

helpitem helpitems[]={
  {"GenericCapture",  IDR_1000},
  {"ControlPinTester",IDR_1010},
  {"CaptureTimer",    IDR_1020},
  {"FramesCounter",   IDR_1030},
  {"CompareFilter",   IDR_1040},
  {"Switch",          IDR_1050},
  {"Aggregator",      IDR_1060},
  {"Disassemble",     IDR_1070},
  {"LPTBitRender",    IDR_1080},
  {"RenderID",        IDR_1090},
  {"Delay",           IDR_1100},
  {"ChangeID",        IDR_1110},
  {"RenderFrameRate", IDR_1120},
  {"RenderContainer", IDR_1130},
  {"Uniselector",     IDR_1140},
  {"Null",            IDR_1150},
  {"Sink",            IDR_1160},
  {"Buffer",          IDR_1170},
  {"LabelFilter",     /*IDR_1180*/  IDR_LabelFilter },
  {"GenericRender",   IDR_1190},
  {"Cap",             IDR_1200},
  {"ArrayToFigure",   IDR_1210},
  {"SynthRGB",        IDR_1220},
  {"Calculator",      IDR_1230}
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
