#include "stdafx.h"

#include <atlbase.h>
#include <atlcom.h>
#define INITGUID
#include "GuidDef.h"
#include <winsdkver.h>
#include <sdkddkver.h>

#include "MFUtility.h"

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "strmiids.lib")

#include "helpers\FramesHelper.h"
#include "video/stdcodec.h"
#include <gadgets\stdsetup.h>
#include "USBCamera.h"
#include <fxfc/fxlogsystem.h>

#define THIS_MODULENAME "USBCamera.cpp"

extern char TVDB400_PLUGIN_NAME[ APP_NAME_MAXLENGTH ];

extern CameraInSystem g_USBCameras[ 32 ] ;
extern DWORD g_dwNumOfConnectedCameras ;

IMPLEMENT_RUNTIME_GADGET( UserCaptureBaseGadget , CCaptureGadget , "Video.capture" )

IMPLEMENT_RUNTIME_GADGET_EX( USBCamera , UserCaptureBaseGadget , "Video.capture" , TVDB400_PLUGIN_NAME )

// Mutex is global for all applications
extern HANDLE g_hGlobalMutex ;
HANDLE USBCamera::m_ghAppMutex = NULL;
CModule USBCamera::m_Module ;
int     USBCamera::m_iNGadgetsInApplication = 0 ;

extern "C"
{
  typedef PVOID HDEVINFO;

  typedef struct _DEVICE_GUID_LIST
  {
    HDEVINFO   DeviceInfo;
    LIST_ENTRY ListHead;
  } DEVICE_GUID_LIST , *PDEVICE_GUID_LIST;

  extern DEVICE_GUID_LIST gHubList;
  extern DEVICE_GUID_LIST gDeviceList;

  VOID InitializeListHead( _Out_ PLIST_ENTRY ListHead )
  {
    ListHead->Flink = ListHead->Blink = ListHead;
  }

  VOID EnumerateHostControllers(
    HTREEITEM  hTreeParent ,
    ULONG     *DevicesConnected
  );

}
extern const char * GetDevices() ;



USBCamera::USBCamera():
    m_FrameReady( FALSE , FALSE )
  , m_iCaptureMode(0)
//  , m_iCameraNum(0)   // Not Selected
  , m_iIndex(-1) // not selected
  , m_iOutputFormat( BI_YUV12 )
  , m_pSourceReader( NULL )
  , m_pMediaSource( NULL )
  , m_pCallback(NULL)
  , m_OutputFormat( OF_YUV12 )
  , m_bRescanCameras( true )
  , m_bWasFirst( false )
{
  __C( MFStartup( MF_VERSION ) );
  SetThreadName( "USBCamera" ) ;
  m_CamerasListForStdDlg = "Not Selected;" ;
  if ( !g_hGlobalMutex )
  {
    g_hGlobalMutex = CreateMutex(NULL, TRUE, "USB_CAMERA_SHARED_MUTEX");
    if (g_hGlobalMutex) // mutex is created, necessary to initialize shared 
    {
      for (int i = 0; i < ARRSZ(g_USBCameras); i++)
        g_USBCameras[i] = CameraInSystem(USBLocation(), NULL);
      ReleaseMutex(g_hGlobalMutex);
      m_ghAppMutex = g_hGlobalMutex;
      m_bWasFirst = true;
    }
    else
      SENDERR("\n!!!!! Can't create global MUTEX   ");
  }
  if ( !m_ghAppMutex )
  {
    m_ghAppMutex = OpenMutex( NULL , FALSE , "USB_CAMERA_SHARED_MUTEX" ) ;
    if ( !m_ghAppMutex )
    {
      DWORD dwLastError = GetLastError() ;
      if ( dwLastError == ERROR_FILE_NOT_FOUND )
      {
        m_ghAppMutex = CreateMutex( NULL , TRUE , "USB_CAMERA_SHARED_MUTEX" ) ;
      }
      else
        SENDERR("\n!!!!! Can't create/open application MUTEX   ");
    }
  }
  m_iNGadgetsInApplication++ ;
  init() ;
  DriverInit();
}

USBCamera::~USBCamera()
{
  if (--m_iNGadgetsInApplication == 0)
  {
    if ( GetNAllocatedUSBCAmeras() == 0 ) // last gadget in computer
    {
      CloseHandle(g_hGlobalMutex);
      g_hGlobalMutex = NULL;
      if (m_bWasFirst)
        m_ghAppMutex = NULL;
    }
    if ( !m_bWasFirst )
    {
      CloseHandle(m_ghAppMutex);
      m_ghAppMutex = NULL;
    }
  }
}

CamProperty USBCamProperties[] = 
{
  { VCapP_CameraNum , "Camera" , VCapP_Unknown , NULL , false } ,
  { VCapP_FormatAndSize , "CaptureMode" , VCapP_Unknown , NULL , false } ,
  { VCapP_Exposure , "Exposure" , VCapP_Auto , NULL , false } ,
  { VCapP_Gain , "Gain" , VCapP_Auto , NULL , false } ,
  { VCapP_Brightness , "Brightness" , VCapP_Unknown , NULL , false } ,
  { VCapP_Contrast , "Contrast" , VCapP_Unknown , NULL , false } ,
  { VCapP_Saturation , "Saturation" , VCapP_Unknown , NULL , false } ,
  { VCapP_ColorEnable , "Monochrome" , VCapP_Unknown , NULL , false } ,
  { VCapP_Sharpness , "Sharpness" , VCapP_Unknown , NULL , false } ,
  { VCapP_Hue , "Hue" , VCapP_Unknown , NULL , false } ,
  { VCapP_Gamma , "Gamma" , VCapP_Unknown , NULL , false } ,
  { VCapP_WhiteBalance , "WhiteBalance" , VCapP_Unknown , NULL , false } ,
} ;

LPCTSTR OutputFormats = { "YUV12;YUV9;MONO;" } ;

void USBCamera::OnScanSettings() 
{ 
  if ( m_bRescanCameras )
  {
    PropertiesReregistration() ;
    m_bRescanCameras = false ;
  }
  return ; 
}

void USBCamera::PropertiesRegistration()
{
  FXAutolock al( m_SettingsLock ) ;
  ClearProperties() ;
  FXString DlgFormat ;
  CheckAvailableCameras( DlgFormat , m_CamerasListForStdDlg ) ;

  if ( WaitForSingleObject( m_ghAppMutex , 2000) == WAIT_OBJECT_0 )
  {
    addProperty(SProperty::INDEXED_COMBO, "Camera",
      &m_CurrentLocation , SProperty::Int64, m_CamerasListForStdDlg, g_ActiveIndexes );
    SetChangeNotification(_T("Camera"), CameraChange, this);
    ReleaseMutex( m_ghAppMutex ) ;
  }
  if ( (m_iIndex >= 0) && (g_USBCameras[m_iIndex].m_pLocalPtr == this) )
  {
    FilterFormats( m_ThisCamera ) ;
    m_CaptureModesListForStdDlg = m_ThisCamera.m_CaptureModesForList.c_str() ;
    if ( m_iCaptureMode >= (int)m_ThisCamera.m_CaptureModes.size() )
      m_iCaptureMode = 0 ;
    bool bRes = false ;
    do
    {
      m_ImageSize.cx = m_ThisCamera.m_CaptureModes[ m_iCaptureMode ].m_uiWidth ;
      m_ImageSize.cy = m_ThisCamera.m_CaptureModes[ m_iCaptureMode ].m_uiHeight ;
      m_Fourcc = *(DWORD*) (m_ThisCamera.m_CaptureModes[ m_iCaptureMode ].m_AsFOURCC) ;
      bRes = FormBMPIH( m_ImageSize , m_Fourcc , &m_FromCameraFormat ) ;
    } while ( !bRes && (++m_iCaptureMode < (int)m_ThisCamera.m_CaptureModes.size()) );

    addProperty( SProperty::COMBO , "CaptureMode" ,
      &m_iCaptureMode , SProperty::Long , m_CaptureModesListForStdDlg );
    SetChangeNotification( _T( "CaptureMode" ) , FormatChange , this );
    addProperty( SProperty::COMBO , "OutputFormat" , &m_OutputFormat ,
      SProperty::Long , OutputFormats ) ;
    for ( size_t i = 0 ; i < m_ThisCamera.m_ControlVector.size() ; i++ )
    {
      CaptureControl& Control = m_ThisCamera.m_ControlVector[ i ] ;
      if ( Control.m_lFlags & Control_Flags_Auto )
      {
        Control.m_bAuto = Control.m_lCurrentFlags & Control_Flags_Auto ;
        addProperty( SProperty::SPIN_BOOL ,
          Control.m_Name , &Control.m_lCurrentValue , SProperty::SpinBool ,
          Control.m_lMin , Control.m_lMax , &Control.m_bAuto ) ;
      }
      else
      {
        addProperty( SProperty::SPIN ,
          Control.m_Name , &Control.m_lCurrentValue , SProperty::Long ,
          Control.m_lMin , Control.m_lMax ) ;
      }
      SetChangeNotificationWithId( Control.m_Name , ControlChange , this , (int)i ) ;
    }
  }
}

void USBCamera::CameraChange(
  LPCTSTR pName , void* pObject , bool& bInvalidate , bool& bRescan )
{
  USBCamera * pGadget = (USBCamera*) pObject;
  if ( pGadget )
  {
    BOOL bWasRun = pGadget->IsRun() ;
    if ( pGadget->m_propertiesArray.GetCount() > 1 ) // camera was selected
    {
      // There is camera was selected, necessary to clear properties.
      pGadget->CameraClose() ;
      bInvalidate = true ;
    }
    if (WaitForSingleObject( m_ghAppMutex , 2000 ) == WAIT_OBJECT_0)
    {
      if ( pGadget->m_CurrentLocation == 0 )
      {
        pGadget->m_iIndex = -1 ;
      }
      else
      {
        int iNewIndex = GetCameraForLocation( pGadget->m_CurrentLocation ) ;
        if (iNewIndex >= 0) // No previous camera selected
        {
          g_USBCameras[ iNewIndex ].m_pLocalPtr = pGadget ;
          g_USBCameras[ iNewIndex ].m_ProcessId = GetCurrentProcessId() ;
          pGadget->m_iIndex = iNewIndex ;
          FXString DevicePath( g_USBCameras[ iNewIndex ].m_FullInfo ) ;
          const char * pAllDevices = GetDevices() ;
          FXString AllUSBDevices( pAllDevices );
          ExtractCameraData( DevicePath , pGadget->m_ThisCamera , AllUSBDevices ) ;
          HRESULT hr = GetCaptureDevice( g_USBCameras[ iNewIndex ].m_FullInfo ,
            pGadget->m_ThisCamera ) ;
          if (hr != S_OK)
          {
            FxSendLogMsg( MSG_ERROR_LEVEL , pGadget->m_GadgetInfo ,
              0 , "Camera Get parameters ERROR=%d" , hr ) ;
          }
        }
        else
        {
          bInvalidate = true ;
          pGadget->m_CurrentLocation = 0 ;
        }
      }
      ReleaseMutex( m_ghAppMutex ) ;
    }
//     if ( iSelCam ) // Camera is selected
//     {
      if ( pGadget->BuildPropertyList() && (pGadget->m_iIndex >= 0) )
      {
        pGadget->CameraInit() ;
        bInvalidate = true ;
        if ( bWasRun )
          pGadget->CameraStart() ;
      }
//    }
  }
}

void USBCamera::FormatChange(
  LPCTSTR pName , void* pObject , bool& bInvalidate , bool& bRescan )
{
  USBCamera * pGadget = (USBCamera*) pObject;
  if ( pGadget && (pGadget->m_iIndex >= 0 ) 
    && (pGadget->m_iIndex < MAX_NUMBER_OF_USB_CAMERAS_IN_SYSTEM) )
  {
    BOOL bWasRun = pGadget->IsRun() ;
    FXAutolock al( pGadget->m_SettingsLock ) ;
    pGadget->CameraInit() ;
    if ( bWasRun )
      pGadget->CameraStart() ;
  }
}

void USBCamera::ControlChange(
  LPCTSTR pName , void* pObject , int iId ) // Id is index in control array
{
  USBCamera * pGadget = (USBCamera*) pObject;
  if ( pGadget )
  {
    FXAutolock al( pGadget->m_SettingsLock ) ;
    if ( (0 <= iId) && (iId < (int)pGadget->m_ThisCamera.m_ControlVector.size()) )
    {
      CaptureControl& Control = pGadget->m_ThisCamera.m_ControlVector[ iId ] ;
      if ( pGadget->m_pMediaSource )
      {
        HRESULT hr = S_OK ;
        switch( Control.m_Node )
        {
        case CN_Camera:
          {
            CComQIPtr<IAMCameraControl> spCameraControl( pGadget->m_pMediaSource );
            if ( spCameraControl )
            {
              Control.m_lCurrentFlags = Control.m_bAuto ? 1 : 2 ;
              hr = spCameraControl->Set( (CameraControlProperty) Control.m_Prop ,
                Control.m_lCurrentValue , Control.m_lCurrentFlags ) ;
            }
          }
          break ;
        case CN_Amplifier:
          {
            CComQIPtr<IAMVideoProcAmp> spVideoAmp( pGadget->m_pMediaSource );
            if ( spVideoAmp )
            {
              hr = spVideoAmp->Set( (VideoProcAmpProperty) Control.m_Prop ,
                Control.m_lCurrentValue , Control.m_lCurrentFlags ) ;
            }
          }
          break ;
        }
      }
    }
  }
}

void USBCamera::ConnectorsRegistration()
{
  addInputConnector( transparent , "Trigger" , fn_capture_trigger , this );
  addOutputConnector( transparent , "OutVideo" );
  addDuplexConnector( text , text , "Properties" ) ;
};

UINT USBCamera::GrabThread( void * pGadget )
{
  USBCamera * pUSBCamera = (USBCamera*) pGadget ;
  FXString GadgetName ;
  pUSBCamera->GetGadgetName( GadgetName ) ;
  GadgetName += _T( "_Grab" ) ;
  SetCurrentThreadName( GadgetName ) ;
  return pUSBCamera->GrabFunc() ;
}

UINT USBCamera::GrabFunc()
{
  DWORD dwStreamIndex = 0 ;
  DWORD dwStreamFlags = 0 ;
  LONGLONG llTimeStamp = 0 ;
  IMFSample * pSample = NULL ;
  while ( IsRun() )
  {
    if ( m_pSourceReader )
    {
      HRESULT hr = m_pSourceReader->ReadSample( 0/*MF_SOURCE_READER_ANY_STREAM*/ ,
        MF_SOURCE_READER_CONTROLF_DRAIN , &dwStreamIndex , 
        &dwStreamFlags , &llTimeStamp , &pSample ) ;
      if ( hr == S_OK )
      {
        if ( pSample )
        {
          TRACE( "\nNext frame " ) ;
          pSample->Release() ;
        }
        else
          Sleep( 2 ) ;
      }
    }
  }
  return 0 ;
}

bool USBCamera::DriverInit()
{
  return EnumCameras() ;
}

void USBCamera::CameraTriggerPulse( CDataFrame * pDataFrame )
{
  if ( m_pCallback )
  {
    m_pCallback->m_ProcessLock.Lock() ;
    m_pCallback->SetNFramesForCapture( Tvdb400_IsEOS( pDataFrame ) ? -1 : 1 ) ;
    m_pCallback->ReadSample() ;
    m_pCallback->m_ProcessLock.Unlock() ;
  }

}

//CVideoFrame*  USBCamera::CameraDoGrab( double * dStartTime )
CDataFrame* USBCamera::GetNextFrame( double * dStartTime )
{
  CVideoFrame * pOutFrame = NULL ;
  m_GrabLock.lock() ;
  if ( m_CapturedQueue.size() )
  {
    pOutFrame = m_CapturedQueue.front() ;
    m_CapturedQueue.pop() ;
  }
  else
  {
    m_GrabLock.unlock() ;
    if ( WaitForSingleObject( m_FrameReady.m_hObject , 100 ) == WAIT_OBJECT_0 )
    {
      m_GrabLock.lock() ;
      if ( m_CapturedQueue.size() )
      {
        pOutFrame = m_CapturedQueue.front() ;
        m_CapturedQueue.pop() ;
      }
    }
    else
      return NULL ;
  }
  m_GrabLock.unlock() ;
  return pOutFrame ;
}

bool USBCamera::CameraStart()
{
  if ( m_GrabThread != NULL )
    CameraStop() ;
  if ( !m_pSourceReader || !m_pMediaSource || !m_pCallback )
  {
    if ( (m_iIndex < 0) || !CameraInit() )
      return false ;
  }
  m_bRun = TRUE ;

#ifdef CALLBACK_MODE
  m_pCallback->Initialize( m_pSourceReader , this ) ;
  CInputConnector * pIn = GetInputConnector( 0 ) ;
  if ( !pIn->IsConnected() )
  {
    m_pCallback->SetNFramesForCapture( -1 ) ;
    m_pCallback->ReadSample() ;
  }
  else
    m_pCallback->SetNFramesForCapture( 0 ) ;

#else
  m_GrabThread = new std::thread( GrabThread , this ) ;
  if ( m_GrabThread != NULL )
  {
    void * h = m_GrabThread->native_handle() ;
    return (h != NULL) && (h != INVALID_HANDLE_VALUE) ;
  }
#endif // endif

  return false ;
}


void USBCamera::CameraStop()
{
  
#ifdef CALLBACK_MODE
  m_GrabLock.lock() ;
  if ( m_pCallback )
  {
    m_pCallback->m_ProcessLock.Lock() ;
    int iNRestFRames = m_pCallback->GetNRestFrames() ;
    if ( iNRestFRames || m_pCallback->GetNWaiting() )
    {
      m_pCallback->SetNFramesForCapture( 0 ) ;
      int iWaitCnt = 0 ;
      while( m_pCallback->GetNWaiting() && (iWaitCnt++ < 50) )
        Sleep( 2 ) ;
    }
    m_pCallback->m_ProcessLock.Unlock() ;
  }
  m_GrabLock.unlock() ;
#else
  if ( m_GrabThread )
  {
    m_bRun = FALSE ;
    m_GrabThread->join() ;
    delete m_GrabThread ;
    m_GrabThread = NULL ;
  }
#endif
}

void USBCamera::CameraClose()
{
  CameraStop() ;
  m_GrabLock.lock() ;
  SAFE_RELEASE( m_pCallback ) ;
  //SAFE_RELEASE( m_pSourceReader ) ;
  SAFE_RELEASE( m_pMediaSource ) ;
  m_pSourceReader = NULL ;
  m_GrabLock.unlock() ;
  ReleaseCameraGlobal( m_iIndex ) ;
}

bool USBCamera::CameraInit()
{
  CameraClose() ;
  if ( m_iIndex >= 0 )
  {
    if (WaitForSingleObject( m_ghAppMutex , 2000 ) == WAIT_OBJECT_0)
    {
      m_iIndex = GetCameraForLocation( m_CurrentLocation ) ;
      if (m_iIndex >= 0)
      {
        // Mark camera as busy in global camera array
        g_USBCameras[ m_iIndex ].m_pLocalPtr = this ;
        g_USBCameras[ m_iIndex ].m_ProcessId = GetCurrentProcessId() ;
        FXString DevicePath( g_USBCameras[ m_iIndex ].m_FullInfo ) ;
        const char * pAllDevices = GetDevices() ;
        FXString AllUSBDevices( pAllDevices );
        ExtractCameraData( DevicePath , m_ThisCamera , AllUSBDevices ) ;
        HRESULT hr = GetCaptureDevice( g_USBCameras[ m_iIndex ].m_FullInfo ,
          m_ThisCamera ) ;
        if (hr != S_OK)
        {
          FxSendLogMsg( MSG_ERROR_LEVEL , m_GadgetInfo ,
            0 , "Camera Get parameters ERROR=%d" , hr ) ;
        }
      }
      ReleaseMutex( m_ghAppMutex ) ;
      if (m_iIndex < 0)
        return false ;
    }
    else
    {
      FxSendLogMsg( MSG_ERROR_LEVEL , m_GadgetInfo ,
        0 , "CameraInit: Can't get global mutex" ) ;
      return false ;
    }
    HRESULT hr = GetSourceFromCaptureDevice(
      DeviceType::Video , m_iIndex ) ;
    if ( hr == S_OK )
    {
      CaptureMode& CapMode = m_ThisCamera.m_CaptureModes[ m_iCaptureMode ] ;
      CSize OrderedSize( CapMode.m_uiWidth , CapMode.m_uiHeight ) ;
      DWORD dwImageSize = FormBMPIH( OrderedSize , 
        *((DWORD*)(&CapMode.m_AsFOURCC)) , &m_FromCameraFormat ) ;
      return (dwImageSize != 0) ;
    }
    else
    {
      FxSendLogMsg( MSG_ERROR_LEVEL , m_GadgetInfo ,
        0 , "CameraInit: Can't get source for camera" ) ;
      return false ;
    }
  }
  return false ;
}

bool USBCamera::GetCameraProperty(
  unsigned i , FXSIZE &value , bool& bauto )
{
  if ( (i < 0) || (i >= m_Properties.size()) )
    return false ;
  VideoCaptureProperties Pr = m_Properties[ i ].m_pr ;

  switch ( Pr )
  {
  case VCapP_CameraNum:
    value = m_Properties[i].m_LastValue.iInt = m_iIndex ;
    return true ;
  case VCapP_FormatAndSize:
    value = m_Properties[ i ].m_LastValue.iInt = m_iOutputFormat ;
    return true ;
  default:
    {
      if ( m_pMediaSource )
      {
      }
    }
    return true ;
  }
  return false ;
}

bool USBCamera::SetCameraProperty( 
  unsigned i , FXSIZE &value , bool& bauto , bool& Invalidate )
{
  if ( (i < 0) || (i >= m_Properties.size()) )
    return false ;
  VideoCaptureProperties Pr = m_Properties[ i ].m_pr ;
  switch ( Pr )
  {
  case VCapP_CameraNum:
    {
      bool bStopped = ( m_pSourceReader ) ? IsRun() : false ;
      if (m_pSourceReader)
        CameraStop() ;
      int iTmp = ( int ) value ;
      if (m_iIndex >= 0)
      {
        CameraClose() ;
        m_iIndex = 0 ;
      }
      if (iTmp != 0)
      {
        bool bBusy = ( g_USBCameras[ iTmp ].m_pLocalPtr == NULL ) ;
        if (!bBusy)
        {
          g_USBCameras[ iTmp ].m_pLocalPtr = this ;
          g_USBCameras[ iTmp ].m_ProcessId = GetCurrentProcessId() ;
          m_iIndex = iTmp ;
          m_CurrentLocation = g_USBCameras[ m_iIndex ].m_Location.m_Index ;
        }
      }
      Invalidate = true ;
      if (m_iIndex >= 0)
      {
        if (CameraInit())
        {
          if (bStopped)
            return CameraStart() ;
        }
      }
    }
    return true ;
  case VCapP_FormatAndSize:

    return true ;
//  default:
//     return m_Camera.set( Pr , (double)value )  ;
  }
  return false ;
}

void USBCamera::ShutDown()
{
  CameraClose() ;
  CCaptureGadget::ShutDown();
  __C( MFShutdown() );
}

bool USBCamera::CheckAndAllocCamera( void )
{
//   if ( m_Camera.isOpened() )
//   {
//     if ( m_iAllocatedCameraNum >= 0 )
//       return true;
//     if ( !CameraInit() )
//       return false;
//   }
  return true;
}

bool USBCamera::BuildPropertyList()
{
  if ( GetHRTickCount() - m_dLastBuiltPropertyTime < 1000. )
    return true;

  PropertiesRegistration() ;

  TRACE( "\n    USBCamera: property list built with %d items" , m_propertiesArray.size() );
  //m_pCamera->Close() ;
  return true;
}

int USBCamera::FilterFormats( CameraData& Camera )
{
  if ( Camera.m_CaptureModes.size() > 30 )
  {
    for ( size_t i = 0 ; i < Camera.m_CaptureModes.size() ; i++ )
    {
      CaptureMode& Mode = Camera.m_CaptureModes[ i ] ;
      if ( ((Mode.m_uiWidth % 320) != 0) || ((Mode.m_uiHeight % 240) != 0)
        || (    (Mode.m_iFPS != 5) && (Mode.m_iFPS != 10) 
             && (Mode.m_iFPS != 15) && (Mode.m_iFPS < 30)) )
      {
        Camera.m_CaptureModes.erase( Camera.m_CaptureModes.begin() + i ) ;
        Camera.m_CaptureModesAsStrings.erase(
          Camera.m_CaptureModesAsStrings.begin() + i ) ;
        i-- ;
      }
    }
    bool bSwapped ;
    do 
    {
      bSwapped = false ;
      for ( size_t i = 0 ; i < Camera.m_CaptureModes.size() - 1 ; i++ )
      {
        if ( Camera.m_CaptureModes[ i ].m_uiWidth < Camera.m_CaptureModes[ i + 1 ].m_uiWidth )
        {
          swap( Camera.m_CaptureModes[ i ] , Camera.m_CaptureModes[ i + 1 ] ) ;
          swap( Camera.m_CaptureModesAsStrings[ i ] , Camera.m_CaptureModesAsStrings[ i + 1 ] ) ;
          bSwapped = true ;
        }
        else if ( Camera.m_CaptureModes[ i ].m_uiWidth == Camera.m_CaptureModes[ i + 1 ].m_uiWidth 
         && Camera.m_CaptureModes[ i ].m_uiHeight < Camera.m_CaptureModes[ i + 1 ].m_uiHeight )
        {
          swap( Camera.m_CaptureModes[ i ] , Camera.m_CaptureModes[ i + 1 ] ) ;
          swap( Camera.m_CaptureModesAsStrings[ i ] , Camera.m_CaptureModesAsStrings[ i + 1 ] ) ;
          bSwapped = true ;
        }
      }
    } while ( bSwapped );
    string ModesForList ;
    for ( size_t i = 0 ; i < Camera.m_CaptureModes.size() ; i++ )
    {
      CaptureMode& Mode = Camera.m_CaptureModes[ i ] ;
      string Format = Mode.m_AsFOURCC ;
      if ( Format.empty() )
        Format = "RGB24" ;
      ModesForList += Format + " " + std::to_string( Mode.m_uiWidth ) +
        "x" + std::to_string( Mode.m_uiHeight ) + ", "
        + std::to_string( Mode.m_uiFPSnum ) + "/" + std::to_string( Mode.m_uiFPSden )
        + "=" + std::to_string( Mode.m_uiFPSnum / Mode.m_uiFPSden ) + " fps;";
    }
    Camera.m_CaptureModesForList = ModesForList ;
  }
  return (int)Camera.m_CaptureModes.size() ;
}

bool USBCamera::CheckAvailableCameras(
  FXString& DlgFormat , FXString& OnlyItems )
{
  EnumCameras() ;
  FXString camlist( "Not Selected(0)" ) , tmpS;
  OnlyItems = "Not Selected" ;
  bool bIsCameraForThisGadgetFound = false ;
  int iCamCnt = 0 ;
  if (WaitForSingleObject(m_ghAppMutex, 2000) == WAIT_OBJECT_0)
  {
    for (UINT j = 0; j < ARRSZ(g_USBCameras); j++)
    {
      if (!g_USBCameras[ j ].m_Location.m_Index)
        continue ;
      TCHAR cMark = _T('+'); // sign, that camera is free
      bool bIsMatched = false;
      if ( j == m_iIndex )
      {
        cMark = _T('!');
        if ( g_USBCameras[j].m_pLocalPtr != this )
        {
          if ( (g_USBCameras[j].m_pLocalPtr != NULL) 
            || ( g_USBCameras[ j ].m_ProcessId && (g_USBCameras[j].m_ProcessId != GetCurrentProcessId()) ) )
          {
            ASSERT(0);
          }
          g_USBCameras[j].m_pLocalPtr = this;
          g_USBCameras[j].m_ProcessId = GetCurrentProcessId();
        }
      }
      else
      {
        if (g_USBCameras[j].m_pLocalPtr == this)
          ASSERT(0);
        if (g_USBCameras[j].m_pLocalPtr != NULL)
          cMark = _T('-');
      }
      FXString LocationAsString = g_USBCameras[ j ].m_Location.GetLocationAsString() ;
      tmpS.Format( ",%c%s(%lld)" , cMark ,  
        g_USBCameras[j].m_FriendlyName , g_USBCameras[j].m_Location.m_Index );
      camlist += tmpS;
      tmpS.Format( ";%c%s/%s" , cMark , ( LPCTSTR ) LocationAsString , 
        g_USBCameras[j].m_FriendlyName );
      OnlyItems += tmpS;
    }
    ReleaseMutex(m_ghAppMutex);
  }
  tmpS.Format( "ComboBox(Camera(%s))" , camlist );
  DlgFormat = tmpS ;
  m_CamerasListForStdDlg = OnlyItems + ';' ;
  return true ;
}

bool USBCamera::IsMatch( LPCTSTR pLocationAndVPM ) // VPM - VID,PID,MI
{
  FXString ForCompare( pLocationAndVPM ) ;
  ForCompare = ForCompare.MakeUpper() ;
  return ( ForCompare.Find( m_VID_PID_MIAsString ) >= 0)
    && (ForCompare.Find( m_Location.GetLocationAsString() ) >= 0 ) ;
}

bool USBCamera::EnumCameras()
{
  CamerasVector AllCameras;

  bool bRes = ( ListCaptureDevices( DeviceType::Video , AllCameras ) == S_OK );

  if (WaitForSingleObject(m_ghAppMutex, 2000) == WAIT_OBJECT_0)
  {
    for ( size_t uFound = 0 ; uFound < AllCameras.size() ; uFound++ )
    {
      CameraData& Camera = AllCameras.at(uFound);
      bool bIsMatched = false;
      for (size_t uGlobalCntr = 0; uGlobalCntr < ARRSZ(g_USBCameras); uGlobalCntr++)
      {
        if (g_USBCameras[uGlobalCntr].m_Location.IsMatched(Camera.m_Index)) // exists in the list
        {
          bIsMatched = true;
          break;
        }
      }
      if (!bIsMatched)
      {
        for (size_t uGlobalCntr = 0; uGlobalCntr < ARRSZ(g_USBCameras); uGlobalCntr++)
        {
          if (g_USBCameras[uGlobalCntr].m_Location.IsEmpty())
          {
            FXString LocPlusIDs(Camera.m_Location.c_str());
            LocPlusIDs += '/';
            LocPlusIDs += Camera.m_VID_PID_MI.c_str();
            g_USBCameras[uGlobalCntr].m_Location.FromString(LocPlusIDs);
            strcpy_s(g_USBCameras[uGlobalCntr].m_FriendlyName, Camera.m_CameraFriendlyName.c_str() );
            strcpy_s(g_USBCameras[uGlobalCntr].m_FullInfo, Camera.m_FullSymbolicLink.c_str());
            break;
          }
        }
      }
    }
    DWORD dwNCameras = 0;
    g_ActiveIndexes[ 0 ] = 0 ;
    for (size_t uGlobalCntr = 0; uGlobalCntr < ARRSZ( g_USBCameras ); uGlobalCntr++)
    {
      if (!g_USBCameras[ uGlobalCntr ].m_Location.IsEmpty())
        g_ActiveIndexes[ ++dwNCameras ] = g_USBCameras[ uGlobalCntr ].m_Location.m_Index ;
    }
    ReleaseMutex(m_ghAppMutex);
    g_dwNumOfConnectedCameras = dwNCameras;
    return (dwNCameras != 0) ;
  }
  return false ;
}

int USBCamera::GetCamNumByIndex(__int64 i64Index)
{
  if ( i64Index )
  {
    for (size_t uGlobalCntr = 0; uGlobalCntr < ARRSZ( g_USBCameras ); uGlobalCntr++)
    {
      if (g_USBCameras[ uGlobalCntr ].m_Location.IsMatched( i64Index ))
        return ( int ) uGlobalCntr + 1;
    }
  }
  return 0;
}

/**
* Gets an audio or video source reader from a capture device such as a webcam or microphone.
* @param[in] deviceType: the type of capture device to get a source reader for.
* @param[in] nDevice: the capture device index to attempt to get the source reader for.
* [out] m_pMediaSource: will be set with the source for the reader if successful.
* [out] m_pSourceReader: will be set with the reader if successful. Set this parameter
* @@Returns S_OK if successful or an error code if not.
*/
HRESULT USBCamera::GetSourceFromCaptureDevice( 
  DeviceType deviceType , UINT nDevice )
{
  UINT32 captureDeviceCount = 0;
  IMFAttributes* pDeviceConfig = NULL;
  IMFActivate** ppCaptureDevices = NULL;
  WCHAR* deviceFriendlyName;
  UINT nameLength = 0;
  IMFAttributes* pAttributes = NULL;

  HRESULT hr = S_OK;

  hr = MFCreateAttributes( &pDeviceConfig , 1 );
  ASSERT( hr == S_OK ) ;// "Error creating capture device configuration." );
  if ( hr == S_OK )
  {
    GUID captureType = (deviceType == DeviceType::Audio) ?
      MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_AUDCAP_GUID :
      MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID;

    // Request video capture devices.
    hr = pDeviceConfig->SetGUID(
      MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE ,
      captureType );
    if ( hr != S_OK ) //  "Error activating capture device."
      SENDERR( "Can't set type GUID for video Err=0x08X (%s)" , hr , (LPCTSTR) ProcessHRESULT( hr ) ) ;
    else
    {
      hr = MFEnumDeviceSources( pDeviceConfig , 
        &ppCaptureDevices , &captureDeviceCount );
      if ( hr != S_OK ) //  "Error activating capture device."
        SENDERR( "Can't enumerate devices Err=0x08X (%s)" , hr , (LPCTSTR) ProcessHRESULT( hr ) ) ;
      else
      {
        if ( nDevice >= captureDeviceCount )
          hr = E_INVALIDARG;
        else
        {
          hr = ppCaptureDevices[ nDevice ]->GetAllocatedString(
            MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME , &deviceFriendlyName , &nameLength );
          ASSERT( hr == S_OK ) ;// "Error retrieving video device friendly name.\n"
          if ( hr == S_OK )
          {
            hr = ppCaptureDevices[ nDevice ]->ActivateObject( IID_PPV_ARGS( &m_pMediaSource ) );
            if ( hr != S_OK ) //  "Error activating capture device."
              SENDERR( "Create Source Err=0x08%X (%s)" , hr , (LPCTSTR) ProcessHRESULT( hr ) ) ;
            else 
            {
              hr = MFCreateAttributes( &pAttributes , 1 ) ;
              if ( hr != S_OK ) 
                SENDERR( "Create Attributes Err=0x08%X (%s)" , hr , (LPCTSTR) ProcessHRESULT( hr ) ) ;
              else
              {
                if ( deviceType == DeviceType::Video )
                {
#ifdef CALLBACK_MODE
                  __C( CComObject<CCallback>::CreateInstance( &m_pCallback ) );
                  ASSERT( m_pCallback->m_dwRef == 0 );

                  __C( pAttributes->SetUnknown( MF_SOURCE_READER_ASYNC_CALLBACK , m_pCallback ) );
#endif
                  // Adding this attribute creates a video source reader that will handle
                  // color conversion and avoid the need to manually convert between RGB24 and RGB32 etc.
                  //hr = pAttributes->SetUINT32( MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING , 1 ) ;
                  hr = pAttributes->SetUINT32( MF_READWRITE_DISABLE_CONVERTERS , 1 ) ;
                  if ( hr != S_OK ) //  "Error activating capture device."
                    SENDERR( "Disable Converters Err=0x08X (%s)" , hr , (LPCTSTR) ProcessHRESULT( hr ) ) ;
                  else
                  {
                     // Create a source reader.
                    hr = MFCreateSourceReaderFromMediaSource(
                      m_pMediaSource ,
                      pAttributes ,
                      &m_pSourceReader );
                    if ( hr != S_OK ) //  "Error activating capture device."
                      SENDERR( "Create Source Reader Err=0x08X (%s)" , hr , (LPCTSTR) ProcessHRESULT( hr ) ) ;
                    else
                    {
                      DWORD dwCaptureMode = m_ThisCamera.m_CaptureModes[ m_iCaptureMode ].m_iModeIndexInCamera ;
                      hr = SetMediaFormat( m_pMediaSource , dwCaptureMode ) ;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  SAFE_RELEASE( pDeviceConfig );
  SAFE_RELEASE( ppCaptureDevices );
  SAFE_RELEASE( pAttributes );

  return hr;
}

int ConvertSampleToImage( LPBYTE pData , int iLen , void * pHostGadget )
{
  USBCamera * pGadget = (USBCamera*) pHostGadget ;
  return pGadget->ConvertSampleToImage( pData , iLen ) ;
}

LPBYTE GetSampleData( IMFSample * pSample , int& iLen )
{
  IMFMediaBuffer* buf = NULL;
  DWORD bufLength = 0 , lockedBufLength = 0;
  BYTE* pByteBuf = NULL , * pDest = NULL ;

  if ( 0 <= pSample->ConvertToContiguousBuffer( &buf ) )
  {
    if ( 0 <= buf->GetCurrentLength( &bufLength ) )
    {
      if ( 0 <= buf->Lock( &pByteBuf , NULL , &lockedBufLength ) )
      {
        if ( lockedBufLength )
        {
          pDest = new BYTE[ lockedBufLength ] ;
          memcpy( pDest , pByteBuf , lockedBufLength ) ;
          iLen = lockedBufLength ;
        }

        if ( 0 > buf->Unlock() )
          TRACE( _T( "Failed to unlock source buffer.\n" ) );
        else
          SAFE_RELEASE( buf );
      }
    }
  }
  return pDest ;
}

int USBCamera::ConvertSampleToImage( LPBYTE pData , int iLen ) // gadget knows current image format
{
  if ( GetInputConnector(0)->IsConnected() )
    m_pCallback->SetNFramesForCapture( 0 ) ;

  if ( m_FromCameraFormat.biSizeImage > (DWORD)iLen )
    return 0 ;
  pTVFrame pForSave = new TVFrame ;
  pForSave->lpBMIH = (LPBITMAPINFOHEADER)malloc( sizeof(BITMAPINFOHEADER) /*+ iLen*/ ) ;
  memcpy( pForSave->lpBMIH , &m_FromCameraFormat , sizeof( BITMAPINFOHEADER ) ) ;
  pForSave->lpData = pData ;
  LPBITMAPINFOHEADER pConverted = NULL ;
  bool bConverted = false , bCatched = false ;
  m_Fourcc = m_FromCameraFormat.biCompression ;
  if ( m_Fourcc == BI_NV12 )
  {
    try
    {
      switch ( m_OutputFormat )
      {
      case OF_YUV12: pConverted = _convertNV12toYUV12( pForSave ) ; break ;
      case OF_YUV9:  pConverted = _convertNV12toYUV9( pForSave ) ; break ;
      case OF_Y800:
        {
          pForSave->lpData = (LPBYTE) malloc( iLen ) ;
          memcpy( pForSave->lpData , pData , iLen ) ;
          pForSave->lpBMIH->biCompression = BI_Y800 ;
          pForSave->lpBMIH->biBitCount = 8 ;
          pForSave->lpBMIH->biSizeImage = 
            pForSave->lpBMIH->biWidth * pForSave->lpBMIH->biHeight ;
          bConverted = true ;
        }
        break ;
      }
    }
    catch (CException* e)
    {
      char ErrExplanation[ 200 ] = "" ;
      e->GetErrorMessage( ErrExplanation , 199 ) ;
      FxSendLogMsg( MSG_ERROR_LEVEL , "USBCam_ConvertNV12" , 0 ,
        "Exception %s" , ErrExplanation ) ;
      bCatched = true ;
    }
  }
  else if ( m_Fourcc == BI_RGB )
  {
    try
    {
      switch ( m_OutputFormat )
      {
      case OF_YUV12: pConverted = rgb24yuv9( pForSave->lpBMIH , pData ); ; break ;
      case OF_YUV9:  pConverted = rgb24yuv12( pForSave->lpBMIH , pData ) ; break ;
      case OF_Y800:  pConverted = rgb24yuv12( pForSave->lpBMIH , pData ) ; break ;
      }
    }
    catch (CException* e)
    {
      char ErrExplanation[ 200 ] = "" ;
      e->GetErrorMessage( ErrExplanation , 199 ) ;
      FxSendLogMsg( MSG_ERROR_LEVEL , "USBCam_ConvertRGB" , 0 ,
        "Exception %s" , ErrExplanation ) ;
      bCatched = true ;
    }
  }
  else if ( m_Fourcc == BI_YUY2 )
  {
    switch ( m_OutputFormat )
    {
    case OF_Y800:
    case OF_YUV12: pConverted = _convertYUY2YUV12( pForSave ) ;
      if ( m_OutputFormat == OF_Y800 )
      {
        pConverted->biCompression = BI_Y800 ;
        pConverted->biBitCount = 8 ;
        pConverted->biSizeImage = pConverted->biWidth * pConverted->biHeight ;
      }
      break ;
    case OF_YUV9:  pConverted = _convertYUY2YUV9( pForSave ) ; break ;
    default:
      break ;
    }
  }
  if ( pConverted )
  {
    free( pForSave->lpBMIH ) ;
    pForSave->lpBMIH = pConverted ;
    pForSave->lpData = NULL ;
    bConverted = true ;
  }
  if ( !bCatched )
  {
    if ( !bConverted )
    {
      pForSave->lpData = (LPBYTE) malloc( iLen ) ;
      memcpy( pForSave->lpData , pData , iLen ) ;
    }
    if ( m_Fourcc == BI_I420 )
    {
      switch ( m_OutputFormat )
      {
      case OF_YUV12:
        _swapUVforI420orYUV12( pForSave );
        pForSave->lpBMIH->biBitCount = 12 ;
        pForSave->lpBMIH->biSizeImage = 
          (pForSave->lpBMIH->biWidth * pForSave->lpBMIH->biHeight * 12) / 8 ;
        pForSave->lpBMIH->biPlanes = 1 ;
        bConverted = true ;
        break ;
      case OF_Y800:
        pForSave->lpBMIH->biCompression = BI_Y800 ;
        bConverted = true ; ;
      }
    }
    if ( !bConverted && !pConverted )
      bConverted = makeYUV9( pForSave ) ;
    if ( bConverted )
    {
      CVideoFrame * pVideoFrame = CVideoFrame::Create( pForSave ) ;
      if ( pVideoFrame )
      {
        m_GrabLock.lock() ;
        if ( m_CapturedQueue.size() > 10 )
        {
          CVideoFrame * pFirst = m_CapturedQueue.front() ;
          pFirst->Release() ;
          m_CapturedQueue.pop() ;
        }
        m_CapturedQueue.push( pVideoFrame ) ;
        m_FrameReady.PulseEvent() ;
        m_GrabLock.unlock() ;
        return (int)m_CapturedQueue.size() ;
      }
    }
    freeTVFrame( pForSave ) ;
  }
  else
    free( pForSave->lpBMIH ) ;
  return 0 ;
}

HRESULT USBCamera::SetMediaFormat( IMFMediaSource* pSource , DWORD dwFormatIndex )
{
  IMFPresentationDescriptor* pPD = NULL;
  IMFStreamDescriptor* pSD = NULL;
  IMFMediaTypeHandler* pHandler = NULL;
  IMFMediaType* pType = NULL;
  IMFMediaType** pTypex = NULL;

  HRESULT hr = pSource->CreatePresentationDescriptor( &pPD );
  if ( hr == S_OK )
  {
    BOOL fSelected;
    hr = pPD->GetStreamDescriptorByIndex( 0 , &fSelected , &pSD );
    if ( hr == S_OK )
    {
      hr = pSD->GetMediaTypeHandler( &pHandler );
      if ( hr == S_OK )
      {
        hr = pHandler->GetMediaTypeByIndex( dwFormatIndex , &pType );
        if ( hr == S_OK )
        {
          hr = pHandler->SetCurrentMediaType( pType );
          MFGetAttributeSize( pType , MF_MT_FRAME_SIZE , 
            (UINT32*) &m_ImageSize.cx , (UINT32*) &m_ImageSize.cy );
        }
      }
    }
  }

  SAFE_RELEASE( &pPD );
  SAFE_RELEASE( &pSD );
  SAFE_RELEASE( &pType );
  SAFE_RELEASE( &pHandler );
  return hr;
}

bool USBCamera::ReleaseCameraGlobal( int iIndex )
{
  if ( 0 <= iIndex && iIndex < ARRSZ(g_USBCameras) )
  {
    if (WaitForSingleObject( m_ghAppMutex , 3000 ) == WAIT_OBJECT_0)
    {
      ASSERT( g_USBCameras[ iIndex ].m_pLocalPtr == this ) ;
      ASSERT( g_USBCameras[ iIndex ].m_ProcessId == GetCurrentProcessId() ) ;
      g_USBCameras[ iIndex ].m_pLocalPtr = NULL ;
      g_USBCameras[ iIndex ].m_ProcessId = 0 ;
      ReleaseMutex( m_ghAppMutex ) ;
      return true ;
    }
    else
      ASSERT( 0 ) ;
  }
  return false ;
}

