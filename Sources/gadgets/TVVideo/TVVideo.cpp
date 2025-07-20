// TVVideo.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "TVVideo.h"
#include "GadgetsLibrary.h"
#include "VideoStereoSplitter.h"
#include "VideoStereoRenderer.h"
#include "ScanLine.h"
#include "VideoCutRect.h"
#include "ColorMapGadget.h"
#include "MergeLayers.h"
#include "CustomFilter.h"
#include "SquareFilter.h"
#include "TVRotate.h"
#include <gadgets\shkernel.h>
#include "SlideView.h"
#include "DXRender.h"
#include "Pseudocolor.h"
#include "GMarking.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DEFINE_SHPLUGIN_EXT;

char TVDB400_PLUGIN_NAME[APP_NAME_MAXLENGTH];
static AFX_EXTENSION_MODULE TVVideoDll = { NULL, NULL };

CDynLinkLibrary* pThisDll = NULL;

extern "C" int APIENTRY
  DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
  // Remove this if you use lpReserved
  UNREFERENCED_PARAMETER(lpReserved);

  if (dwReason == DLL_PROCESS_ATTACH)
  {
    TRACE0("TVVideo.DLL Initializing!\n");

    // Extension DLL one-time initialization
    if (!AfxInitExtensionModule(TVVideoDll, hInstance))
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

    pThisDll = new CDynLinkLibrary(TVVideoDll);
    // Init plugin
    FXString fName=::FxGetModuleName(TVVideoDll.hModule);
    ASSERT(fName.GetLength()<APP_NAME_MAXLENGTH);
    strcpy(TVDB400_PLUGIN_NAME, fName);
  }
  else if (dwReason == DLL_PROCESS_DETACH)
  {
    TRACE0("TVVideo.DLL Terminating!\n");
    // Terminate the library before destructors are called
    AfxTermExtensionModule(TVVideoDll);
  }
  return 1;   // ok
}



_declspec(dllexport) void __stdcall PluginEntry(LPVOID pGraphBuilder)
{
  // Add filter collection to graph builder:
  IGraphbuilder* Builder = (IGraphbuilder*)pGraphBuilder;
  REGISTER_RUNTIME_GADGET(VideoRender, Builder);
  REGISTER_RUNTIME_GADGET(ColorBalance,Builder);
  REGISTER_RUNTIME_GADGET(DXVideoRender, Builder);
  REGISTER_RUNTIME_GADGET( VideoY16toY8 , Builder );
  REGISTER_RUNTIME_GADGET( VideoY8toY16 , Builder );
  REGISTER_RUNTIME_GADGET( VideoY8toYUV9 , Builder );
  REGISTER_RUNTIME_GADGET(VideoNegative, Builder);
  REGISTER_RUNTIME_GADGET(VideoNormalize, Builder);
  REGISTER_RUNTIME_GADGET(VideoFlip, Builder);
  REGISTER_RUNTIME_GADGET(VideoNormalizeEx, Builder);
  REGISTER_RUNTIME_GADGET(VideoMassNormalize, Builder);
  REGISTER_RUNTIME_GADGET(VideoClearColor, Builder);
  REGISTER_RUNTIME_GADGET(GammaCorrection, Builder);
  REGISTER_RUNTIME_GADGET(VideoSimpleBinarize, Builder);
  REGISTER_RUNTIME_GADGET(VideoPercentBinarize, Builder);
  REGISTER_RUNTIME_GADGET(MassBinarize, Builder);
  REGISTER_RUNTIME_GADGET(HTriggerBinarize, Builder);
  REGISTER_RUNTIME_GADGET(PseudoColor,Builder);
  REGISTER_RUNTIME_GADGET(RangeContrast,Builder);
  REGISTER_RUNTIME_GADGET(ColorBalanceM,Builder);
  REGISTER_RUNTIME_GADGET(Equalize,Builder);
  REGISTER_RUNTIME_GADGET(VideoClearFrames, Builder);
  REGISTER_RUNTIME_GADGET(VideoLowPass, Builder);
  REGISTER_RUNTIME_GADGET(VideoLowPassM, Builder);
  REGISTER_RUNTIME_GADGET(VideoHighPass, Builder);
  REGISTER_RUNTIME_GADGET(Video81PassFilter, Builder);
  REGISTER_RUNTIME_GADGET(VideoHighPass1DV, Builder);
  REGISTER_RUNTIME_GADGET(VideoLowPass1DH, Builder);
  REGISTER_RUNTIME_GADGET(VideoHighPass1DH, Builder);
  REGISTER_RUNTIME_GADGET(VideoLowPass1DV, Builder);
  REGISTER_RUNTIME_GADGET(VideoEdgeDetector, Builder);
  REGISTER_RUNTIME_GADGET(VideoFeatureDetector, Builder);
  REGISTER_RUNTIME_GADGET(VideoFlatten, Builder);
  REGISTER_RUNTIME_GADGET(VideoFlatten2, Builder);
  REGISTER_RUNTIME_GADGET(VideoSharpen, Builder);
  REGISTER_RUNTIME_GADGET(VideoEnlarge, Builder);
  REGISTER_RUNTIME_GADGET(VideoDiminish, Builder);
  REGISTER_RUNTIME_GADGET(VideoDiminishX, Builder);
  REGISTER_RUNTIME_GADGET(VideoEnlargeY, Builder);
  REGISTER_RUNTIME_GADGET(VideoResample, Builder);
  REGISTER_RUNTIME_GADGET(VideoErode, Builder);
  REGISTER_RUNTIME_GADGET(VideoDilate, Builder);
  REGISTER_RUNTIME_GADGET(VideoErodeHorizontal, Builder);
  REGISTER_RUNTIME_GADGET(VideoDilateHorizontal, Builder);
  REGISTER_RUNTIME_GADGET(VideoErodeVertical, Builder);
  REGISTER_RUNTIME_GADGET(VideoDilateVertical, Builder);
  REGISTER_RUNTIME_GADGET(VideoCutRect, Builder);
  REGISTER_RUNTIME_GADGET(Block8, Builder);
  REGISTER_RUNTIME_GADGET(Render3DPlot, Builder);
  REGISTER_RUNTIME_GADGET(VideoHMeander, Builder);
  REGISTER_RUNTIME_GADGET(VideoAverage, Builder);
  REGISTER_RUNTIME_GADGET(VideoStereoSplitter, Builder);
  REGISTER_RUNTIME_GADGET(VideoStereoRender, Builder);
  REGISTER_RUNTIME_GADGET(ScanLine, Builder);
  REGISTER_RUNTIME_GADGET(Deinterlace, Builder);
  REGISTER_RUNTIME_GADGET( ColorMap , Builder ) ;
  REGISTER_RUNTIME_GADGET(Any2Yuv9, Builder);
  REGISTER_RUNTIME_GADGET( MergeLayers , Builder ) ;
  REGISTER_RUNTIME_GADGET(CustomFilter, Builder);
  REGISTER_RUNTIME_GADGET(SquareFilter, Builder);
  REGISTER_RUNTIME_GADGET(Rotate, Builder);
  REGISTER_RUNTIME_GADGET(SlideView, Builder);
  REGISTER_RUNTIME_GADGET(Pseudocolor2, Builder);
  REGISTER_RUNTIME_GADGET( GMarking , Builder);
  // Add filter register string just above.	
}

_declspec(dllexport) void __stdcall PluginExit(LPVOID pGraphBuilder)
{
  // Remove filter collection from graph builder:
  IGraphbuilder* Builder = (IGraphbuilder*)pGraphBuilder;

  UNREGISTER_RUNTIME_GADGET(VideoRender, Builder);
  UNREGISTER_RUNTIME_GADGET(ColorBalance,Builder);
  UNREGISTER_RUNTIME_GADGET(DXVideoRender, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoY16toY8, Builder);
  UNREGISTER_RUNTIME_GADGET( VideoY8toY16 , Builder );
  UNREGISTER_RUNTIME_GADGET( VideoY8toYUV9 , Builder );
  UNREGISTER_RUNTIME_GADGET(VideoNegative, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoNormalize, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoFlip, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoNormalizeEx, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoMassNormalize, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoClearColor, Builder);
  UNREGISTER_RUNTIME_GADGET(GammaCorrection, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoSimpleBinarize, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoPercentBinarize, Builder);
  UNREGISTER_RUNTIME_GADGET(MassBinarize, Builder);
  UNREGISTER_RUNTIME_GADGET(HTriggerBinarize, Builder);
  UNREGISTER_RUNTIME_GADGET(PseudoColor,Builder);
  UNREGISTER_RUNTIME_GADGET(RangeContrast,Builder);
  UNREGISTER_RUNTIME_GADGET(ColorBalanceM,Builder);
  UNREGISTER_RUNTIME_GADGET(Equalize,Builder);
  UNREGISTER_RUNTIME_GADGET(VideoClearFrames, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoLowPass, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoHighPass, Builder);
  UNREGISTER_RUNTIME_GADGET(Video81PassFilter, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoHighPass1DV, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoLowPass1DH, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoHighPass1DH, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoLowPass1DV, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoEdgeDetector, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoFeatureDetector, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoFlatten, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoFlatten2, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoSharpen, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoEnlarge, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoDiminish, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoDiminishX, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoEnlargeY, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoResample, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoErode, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoDilate, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoErodeHorizontal, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoDilateHorizontal, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoErodeVertical, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoDilateVertical, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoCutRect, Builder);
  UNREGISTER_RUNTIME_GADGET(Block8, Builder);
  UNREGISTER_RUNTIME_GADGET(Render3DPlot, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoHMeander, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoAverage, Builder);
  UNREGISTER_RUNTIME_GADGET(VideoStereoSplitter, Builder);
  //
  UNREGISTER_RUNTIME_GADGET(VideoStereoRender, Builder);
  UNREGISTER_RUNTIME_GADGET(ScanLine, Builder);
  UNREGISTER_RUNTIME_GADGET(Deinterlace, Builder);
  UNREGISTER_RUNTIME_GADGET( ColorMap , Builder ) ;
  UNREGISTER_RUNTIME_GADGET(Any2Yuv9, Builder);
  UNREGISTER_RUNTIME_GADGET(MergeLayers, Builder);
  UNREGISTER_RUNTIME_GADGET(CustomFilter, Builder);
  UNREGISTER_RUNTIME_GADGET(SquareFilter, Builder);
  UNREGISTER_RUNTIME_GADGET(Rotate, Builder);
  UNREGISTER_RUNTIME_GADGET(SlideView, Builder);
  UNREGISTER_RUNTIME_GADGET(Pseudocolor2, Builder);
  UNREGISTER_RUNTIME_GADGET( GMarking , Builder );
  // Add filter unregister string just above.
}

_declspec(dllexport) LPCTSTR __stdcall GetPluginName()
{
  return TVDB400_PLUGIN_NAME;
}

helpitem helpitems[]=
{
  {"VideoRender",     IDR_10001},
  {"ColorBalance",    IDR_10002},
  {"DXVideoRender",   IDR_10003},
  {"VideoY16toY8",    IDR_10004},
  {"VideoY8toYUV9",   IDR_10005},
  {"VideoNegative",   IDR_10006},
  {"VideoNormalize",  IDR_10007},
  {"VideoFlip",       IDR_10008},
  {"VideoNormalizeEx",IDR_10009},
  {"VideoMassNormalize",IDR_10010},
  {"VideoClearColor", IDR_10011},
  {"GammaCorrection", IDR_10012},
  {"VideoSimpleBinarize",IDR_10013},
  {"VideoPercentBinarize",IDR_10014},
  {"MassBinarize",    IDR_10015},
  {"HTriggerBinarize",IDR_10016},
  {"PseudoColor",     IDR_10017},
  {"ColorBalanceM",   IDR_10018},
  {"Equalize",        IDR_10019},
  {"VideoClearFrames",IDR_10020},
  {"VideoLowPass",    IDR_10021},
  {"VideoHighPass",   IDR_10022},
  {"Video81PassFilter",IDR_10023},
  {"VideoHighPass1DV",IDR_10024},
  {"VideoLowPass1DH", IDR_10025},
  {"VideoHighPass1DH",IDR_10026},
  {"VideoLowPass1DV",IDR_10027},
  {"VideoEdgeDetector",IDR_10028},
  {"VideoFeatureDetector",IDR_10029},
  {"VideoFlatten",    IDR_10030},
  {"VideoFlatten2",   IDR_10031},
  {"VideoSharpen",    IDR_10032},
  {"VideoEnlarge",    IDR_10033},
  {"VideoDiminish",   IDR_10034},
  {"VideoDiminishX",  IDR_10035},
  {"VideoEnlargeY",   IDR_10036},
  {"VideoResample",   IDR_10037},
  {"VideoErode",      IDR_10038},
  {"VideoDilate",     IDR_10039},
  {"VideoErodeHorizontal", IDR_10040},
  {"VideoDilateHorizontal",IDR_10041},
  {"VideoErodeVertical",   IDR_10042},
  {"VideoDilateVertical",  IDR_10043},
  {"VideoCutRect",    IDR_10044},
  {"Block8",          IDR_10045},
  {"Render3DPlot",    IDR_10046},
  {"VideoHMeander",   IDR_10047},
  {"VideoAverage",    IDR_10048},
  {"VideoStereoSplitter",  IDR_10049},
  {"VideoStereoRender",IDR_10050},
  {"ScanLine",        IDR_10051},
  {"Deinterlace",     IDR_10052},
  {"ColorMap",        IDR_10053},
  {"Any2Yuv9",       IDR_10054},
  {"MergeLayers",     IDR_10055},
  {"CustomFilter",    IDR_10056},
  {"SquareFilter",    IDR_10057},
  {"Rotate",          IDR_10058},
  {"SlideView",       IDR_10059}
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
