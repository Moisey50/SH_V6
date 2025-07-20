// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "fxfc/FXRegistry.h"

#include "USBCamera.h"
#include <gadgets\shkernel.h>
#include <gadgets/gadbase.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DEFINE_SHPLUGIN_EXT;

char TVDB400_PLUGIN_NAME[ APP_NAME_MAXLENGTH ];


//IMPLEMENT_RUNTIME_GADGET_EX( USBCamera , CCaptureGadget , "Video.capture" , TVDB400_PLUGIN_NAME );
//USER_CAPTURE_RUNTIME_GADGET( USBCamera , "Video.capture" );

#pragma data_seg(".USB_CAM_SHARED")

CameraInSystem g_USBCameras[MAX_NUMBER_OF_USB_CAMERAS_IN_SYSTEM] ;
// Index is camera location in USB tree as one __int64 number
// (look comment to m_Index member in USBLocation class
// Only available indexes will be in following array, number of indexes is
// in g_dwNumOfConnectedCameras
__int64        g_ActiveIndexes[MAX_NUMBER_OF_USB_CAMERAS_IN_SYSTEM]; 
// Mutex is global for all applications
HANDLE  g_hGlobalMutex = NULL;
DWORD   g_dwNumOfConnectedCameras = 0;

#pragma data_seg()

// section SHARED is for read, write and shared
#pragma comment(linker, "/SECTION:.USB_CAM_SHARED,RWS") 

int GetNAllocatedUSBCAmeras()
{
  int iNAllocated = 0;
  for ( int i = 0 ; i < MAX_NUMBER_OF_USB_CAMERAS_IN_SYSTEM ; i++ )
  {
    if (g_USBCameras[i].m_pLocalPtr)
      iNAllocated++;
  }
  return iNAllocated;
}

// Following program returns camera index in global cameras array
// If not found returns -1
int GetCameraForLocation( __int64 Location )
{
  for (int i = 0 ; i < ARRSZ( g_USBCameras ) ; i++)
  {
    if (Location == g_USBCameras[ i ].m_Location.m_Index)
    {
      if (g_USBCameras[ i ].m_pLocalPtr == NULL)
        return i ;
      else
        return -1 ; // camera is busy
    }
  }
  return -1 ;
}
static AFX_EXTENSION_MODULE USBCameraDll = { NULL , NULL };
CDynLinkLibrary* pThisDll = NULL;

extern "C" int APIENTRY
DllMain( HINSTANCE hInstance , DWORD dwReason , LPVOID lpReserved )
{
  // Remove this if you use lpReserved
  UNREFERENCED_PARAMETER( lpReserved );

  if ( dwReason == DLL_PROCESS_ATTACH )
  {
    TRACE0( "USBCameraDll.DLL Initializing!\n" );

    // Extension DLL one-time initialization
    if ( !AfxInitExtensionModule( USBCameraDll , hInstance ) )
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

    pThisDll = new CDynLinkLibrary( USBCameraDll );
    // Init plugin
    FXString fName = ::FxGetModuleName( USBCameraDll.hModule );
    ASSERT( fName.GetLength() < APP_NAME_MAXLENGTH );
    strcpy( TVDB400_PLUGIN_NAME , fName );
  }
  else if ( dwReason == DLL_PROCESS_DETACH )
  {
    TRACE0( "USBCameraDll.DLL Terminating!\n" );
    // Terminate the library before destructors are called
    AfxTermExtensionModule( USBCameraDll );
  }
  return 1;   // ok
}


_declspec(dllexport) void __stdcall PluginEntry( LPVOID pGraphBuilder )
{
  IGraphBuilderEx* Builder = (IGraphBuilderEx*) pGraphBuilder;
  REGISTER_RUNTIME_GADGET( USBCamera , Builder );
  // Add filter register string just above.
}

_declspec(dllexport) void __stdcall PluginExit( LPVOID pGraphBuilder )
{
  IGraphbuilder* Builder = (IGraphbuilder*) pGraphBuilder;
  UNREGISTER_RUNTIME_GADGET( USBCamera , Builder );
  // Add filter unregister string just above.
}

_declspec(dllexport) LPCTSTR __stdcall GetPluginName()
{
  return TVDB400_PLUGIN_NAME;
}

/////////////////////////////////////////////////////////////////////////////
// Wave Gadgets help

/* helpitem helpitems[]=
{
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
*/