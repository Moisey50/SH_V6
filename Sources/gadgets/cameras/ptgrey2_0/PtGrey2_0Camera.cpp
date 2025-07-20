// PtGrey2_0.cpp: implementation of the PtGrey2_0 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <gadgets\gadbase.h>
#include <gadgets\stdsetup.h>
#include <gadgets\textframe.h>
#include <math\Intf_sup.h>
#include "ptgrey2_0.h"
#include "PtGrey2_0Camera.h"
#include <video\shvideo.h>
#include <helpers\FramesHelper.h>


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#define THIS_MODULENAME "PtGrey2_0Camera.cpp"


IMPLEMENT_RUNTIME_GADGET_EX( PtGrey2_0 , CCaptureGadget , "Video.capture" , TVDB400_PLUGIN_NAME );


HANDLE                      PtGrey2_0::m_hConfigMutex = NULL ;
FXLockObject                PtGrey2_0::m_ConfigLock ;
Device::DeviceInfo          PtGrey2_0::m_CamInfo[ MAX_DEVICESNMB ] ;
int                         PtGrey2_0::m_iCamNum ;
int                         PtGrey2_0::m_iGadgetCount = 0 ;
FXArray<DWORD>              PtGrey2_0::m_BusyCameras ;
bool                        PtGrey2_0::m_bCamerasEnumerated = false ;
bool                        PtGrey2_0::m_bRescanCameras = true  ;
CameraInfo                  PtGrey2_0::m_GigECamerasInfo[ MAX_DEVICESNMB ];
UINT                        PtGrey2_0::m_uiNGigECameras;





#ifdef _DEBUG
DWORD PtGrey2_0::m_dwDefaultTimeout = 60000 ;
#else
DWORD PtGrey2_0::m_dwDefaultTimeout = 1000 ;
#endif

void PtGrey2_0::NewImageCallBack( Image * pImage , const void * pCallbackData )
{
  double dStartTime = GetHRTickCount() ;
  PtGrey2_0 * pGadget = ( PtGrey2_0* )pCallbackData ;
  if ( pGadget->m_iNSkipImages > 0 )
  {
    pGadget->m_iNSkipImages-- ;
    return ;
  }

  Error error;
  unsigned int rows , cols , stride;
  PixelFormat format;
  BayerTileFormat BayerFormat ;

  
  pImage->GetDimensions( &rows , &cols , &stride , &format , &BayerFormat );
  DWORD dwDataSize = pImage->GetDataSize() ;

  FlyCapture2::Image * pCopy = new FlyCapture2::Image(
    rows , cols , stride , pImage->GetData() ,
    dwDataSize , format , BayerFormat ) ;


  if ( pCopy )
  {
    if ( pGadget->m_bStopInitialized )
    {
      TRACE( "Frame after stop initialized, cam %u\n" , pGadget->m_dwSerialNumber ) ;
    }
    pGadget->m_GrabLock.Lock() ;
    if ( pGadget->m_bStopInitialized )
    {
      TRACE( "Lock after stop initialized, cam %u\n" , pGadget->m_dwSerialNumber ) ;
    }
    if ( pGadget->m_pNewImage )
    {
      if ( pGadget->m_bStopInitialized )
      {
        TRACE( "Image delete after stop initialized\n" ) ;
      }
      delete pGadget->m_pNewImage ;
      pGadget->m_pNewImage = NULL ;
    }
    pGadget->m_pNewImage = pCopy ;
    SetEvent( pGadget->m_evFrameReady ) ;
    pGadget->m_dLastFrameTime = GetHRTickCount() ;
    pGadget->m_dLastInCallbackTime = pGadget->m_dLastFrameTime - dStartTime ;
    pGadget->m_bTriedToRestart = false ;
    pGadget->m_GrabLock.Unlock() ;
    if ( pGadget->m_bStopInitialized )
    {
      TRACE( "Unlock after stop initialized, cam %u\n" , pGadget->m_dwSerialNumber ) ;
    }
    if ( pGadget->m_CallbackThreadName.IsEmpty() )
    {
      pGadget->GetGadgetName( pGadget->m_CallbackThreadName ) ;
      pGadget->m_CallbackThreadName += _T( "_CB" ) ;
      DWORD dwThreadId = ::GetCurrentThreadId() ;
      ::SetThreadName( ( LPCSTR )( ( LPCTSTR )pGadget->m_CallbackThreadName ) , dwThreadId ) ;
    }
  }
}


Device::Property cProperties[] =
{
  {(DWORD) SETROI , "ROI" } ,
  {(DWORD) FORMAT_MODE , "Format" } ,
  {(DWORD) SHUTTER , "Shutter_us" } ,
  {(DWORD) GAIN , "Gain_dBx10"} ,
  {(DWORD) TRIGGER_MODE , "Trigger_mode"} ,
  {(DWORD) TRIGGER_DELAY , "Trigger_delay" } ,
  {(DWORD) MAX_PACKET_SIZE , "SetMaxPacket"} ,
  {(DWORD) FRAME_RATE , "Frame_rate_x10"} ,
  {(DWORD) PACKETSIZE , "PacketSize" } ,
  {(DWORD) GAMMA , "Gamma"} ,
  {(DWORD) BRIGHTNESS , "Brightness"} ,
  {(DWORD) AUTO_EXPOSURE , "Auto_exposure"} ,
  {(DWORD) SHARPNESS , "Sharpness"} ,
  {(DWORD) WHITE_BALANCE , "White_balance"} ,
  {(DWORD) HUE , "Hue"} ,
  {(DWORD) SATURATION , "Saturation"} ,
  {(DWORD) SETSTROBE0 , "Strobe0"} ,
  {(DWORD) SETSTROBE1 , "Strobe1"} ,
  {(DWORD) SETSTROBE2 , "Strobe2"} ,
  {(DWORD) SETSTROBE3 , "Strobe3"} ,
  {(DWORD) WHITE_BAL_RED , "W.B.Red"} ,
  {(DWORD) WHITE_BAL_BLUE , "W.B.Blue"} ,
  {(DWORD) EMBED_INFO ,     "EmbedInfo"} ,
  {(DWORD) SAVE_SETTINGS , "SaveSettings"} ,
  {(DWORD) TEMPERATURE , "Temperature"} ,
  {(DWORD) RESTART_TIMEOUT , "Restart_ms"}
  //   {IRIS,"Iris"},
  //   {FOCUS,"Focus"},
  //   {ZOOM,"Zoom"},
  //   {PAN,"Pan"},
  //   {TILT,"Tilt"},
};

__forceinline int _getPropertyCount()
{
  return ARRSZ( cProperties ) ;
}

__forceinline LPCTSTR _getPropertyname( int id )
{
  for ( int i = 0; i < _getPropertyCount(); i++ )
  {
    if ( cProperties[ i ].id == id )
      return cProperties[ i ].name;
  }
  return "Unknown property";
}

Videoformat vFormats[] =
{
  { PIXEL_FORMAT_MONO8 , "Mono8" } ,
  { PIXEL_FORMAT_MONO16 , "Mono16" } ,
  { PIXEL_FORMAT_411YUV8 , "YUV411" } ,
  {PIXEL_FORMAT_422YUV8,"YUV422"},
  { PIXEL_FORMAT_RGB , "RGB24" } ,
  { PIXEL_FORMAT_RGB16 , "RGB48" } ,
  { PIXEL_FORMAT_RAW8 , "RAW8" },
  {PIXEL_FORMAT_RAW16,"RAW16"}
};

LPCTSTR GetPixelFormatName( PixelFormat Format )
{
  for ( int i = 0 ; i < ARRSZ( vFormats ) ; i++ )
  {
    if ( Format == vFormats[ i ].vm )
      return vFormats[ i ].name ;
  }
  return NULL ;
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PtGrey2_0::PtGrey2_0()
{
  m_bTimeOutDisable = false ;
  double dStart = GetHRTickCount() ;
  m_pCamera = m_pCamera = NULL ;
  m_CurrentCameraGUID = PGRGuid();
  m_dwSerialNumber = m_dwConnectedSerialNumber = m_dwInitializedSerialNumber = 0 ;
  m_Mode = MODE_0;
  m_CurrentROI = CRect( 0 , 0 , 640 , 480 );
  m_pixelFormat = PIXEL_FORMAT_MONO8;
  m_uiPacketSize = 960 ;
  m_FormatNotSupportedDispayed = false;
  m_GadgetInfo = "PtGrey2_0 gadget";
  m_WaitEventFrameArray[ 0 ] = m_evFrameReady = CreateEvent( NULL , FALSE , FALSE , NULL ) ;
  m_WaitEventBusChangeArr[ 1 ] = m_WaitEventFrameArray[ 1 ] = m_evExit ;
  m_WaitEventBusChangeArr[ 0 ] = m_evBusChange = CreateEvent( NULL , FALSE , FALSE , NULL ) ;
  m_evControlRequestFinished = CreateEvent( NULL , FALSE , FALSE , NULL ) ;
  m_hCamAccessMutex = CreateMutex( NULL , FALSE , NULL ) ;
  m_continueGrabThread = false ;
  m_disableEmbeddedTimeStamp = false ;
  //m_isSelectingNewCamera = false ;
  m_dLastStartTime = 0. ;
  m_pNewFrame = NULL ;
  m_pNewImage = NULL ;
  m_hGrabThreadHandle = NULL ;
  m_BusEvents = 0 ;
  m_bCamerasEnumerated = false ;
  m_cbArrivalHandle = NULL ;
  m_cbRemovalHandle = NULL ;
  m_cbResetHandle = NULL ;
  m_iNoCameraStatus = 0 ;
  m_bInitialized = false ;
  m_bStopInitialized = false ;
  m_bIsRunning = false ;
  m_bRescanCameras = true ;
  m_iNNoSerNumberErrors = 0 ;
  m_IntfType = INTERFACE_UNKNOWN ;
  m_iWBRed = m_iWBlue = 512 ;
  m_bLocalStopped = FALSE ;
  m_TriggerMode = TrigNotSupported ;
  m_dLastInCallbackTime = 0. ;
  m_dwNArrivedEvents = 0 ;
  m_iFPSx10 = 150 ; // 15 FPS
  m_iRestartTimeOut_ms = 0 ;
  m_iSaveSettingsState = -1 ;
  m_iNSkipImages = 0 ;
  m_bMaxPacketSize = FALSE;
  m_uiSavedPacketSize = 0 ;
  m_bEmbedFrameInfo = FALSE ;

  memset( &m_f7is , 0 , sizeof( m_f7is ) );
  memset( &m_f7pi , 0 , sizeof( m_f7pi ) );
  memset( &m_BMIH , 0 , sizeof( BITMAPINFOHEADER ) );
  memset( &m_RealBMIH , 0 , sizeof( BITMAPINFOHEADER ) );
#ifdef _DEBUG
  m_iMutexTakeCntr = 0 ;
#endif
  DriverInit() ;
  if ( m_hConfigMutex == NULL )
  {
    m_hConfigMutex = CreateMutex( NULL , FALSE , NULL ) ;
    m_iGadgetCount = 1 ;
  }
  else
  {
    UINT uiRes = WaitForSingleObject( m_hConfigMutex , 2000 ) ;
    if ( uiRes == WAIT_OBJECT_0 )
    {
      m_iGadgetCount++ ;
      ReleaseMutex( m_hConfigMutex ) ;
    }
    else
    {
      m_iGadgetCount++ ;
      SENDERR( "Can't take ConfigMutex" ) ;
    }
  }
  double dBusyTime = GetHRTickCount() - dStart ;
  TRACE( "PtGrey2_0::PtGrey2_0: Start %g , Busy %g\n" , dStart , dBusyTime ) ;
}

PtGrey2_0::~PtGrey2_0()
{
  if ( m_hGrabThreadHandle )
  {
    DWORD dwRes = WaitForSingleObject( m_hGrabThreadHandle , 1000 ) ;
    ASSERT( dwRes == WAIT_OBJECT_0 ) ;
    dwRes = WaitForSingleObject( m_hCamAccessMutex , 1000 ) ;
    ASSERT( dwRes == WAIT_OBJECT_0 ) ;
    CloseHandle( m_hCamAccessMutex ) ;
  }
  UINT uiRes = WaitForSingleObject( m_hConfigMutex , 10000 ) ;
  if ( uiRes == WAIT_OBJECT_0 )
  {
    if ( --m_iGadgetCount <= 0 )
    {
      ReleaseMutex( m_hConfigMutex ) ;
      CloseHandle( m_hConfigMutex ) ;
    }
    else
      ReleaseMutex( m_hConfigMutex ) ;

  }
  else
  {
    --m_iGadgetCount ;
    SENDERR( "Can't take ConfigMutex: Err=%u" , uiRes ) ;
  }

}

PropertyType PtGrey2_0::GetThisCamPropertyId( LPCTSTR name )
{
  int i;
  PropertyType retV = UNSPECIFIED_PROPERTY_TYPE ;
  for ( i = 0; i < m_ThisCamProperty.GetSize(); i++ )
  {
    if ( m_ThisCamProperty[ i ].name.CompareNoCase( name ) == 0 )
    {
      retV = m_ThisCamProperty[ i ].id;
      break;
    }
  }
  return retV;
}

int PtGrey2_0::GetThisCamPropertyIndex( LPCTSTR name )
{
  int i;
  for ( i = 0; i < m_ThisCamProperty.GetSize(); i++ )
  {
    if ( m_ThisCamProperty[ i ].name.CompareNoCase( name ) == 0 )
      return i ;
  }
  return -1 ;
}

int PtGrey2_0::GetThisCamPropertyIndex( PropertyType Type )
{
  int i ;
  for ( i = 0; i < m_ThisCamProperty.GetSize(); i++ )
  {
    if ( m_ThisCamProperty[ i ].id == Type )
      return i ;
  }
  return -1 ; // bad index
};

LPCTSTR PtGrey2_0::GetThisCamPropertyName( PropertyType Type )
{
  int iIndex = GetThisCamPropertyIndex( Type ) ;
  if ( iIndex < 0 )
    return _T( "Unknown Name" ) ;
  else
    return ( LPCTSTR )m_ThisCamProperty[ iIndex ].name ;
  {
  }
};

PropertyType PtGrey2_0::GetThisCamPropertyType( int iIndex )
{
  if ( iIndex < 0 || iIndex >= m_ThisCamProperty.GetCount() )
    return UNSPECIFIED_PROPERTY_TYPE ;
  return m_ThisCamProperty[ iIndex ].id ;
};

LPCTSTR PtGrey2_0::GetThisCamPropertyName( int iIndex )
{
  if ( iIndex < 0 || iIndex >= m_ThisCamProperty.GetCount() )
    return _T( "Unknown Name" ) ;
  return ( LPCTSTR )m_ThisCamProperty[ iIndex ].name ;
};

PropertyDataType PtGrey2_0::GetPropertyDataType( int iIndex )
{
  if ( iIndex < 0 || iIndex >= m_ThisCamProperty.GetCount() )
    return PDTUnknown ;
  return m_ThisCamProperty[ iIndex ].m_DataType ;
};

PropertyDataType PtGrey2_0::GetPropertyDataType( PropertyType Type )
{
  int iIndex = GetThisCamPropertyIndex( Type ) ;
  if ( iIndex < 0 )
    return PDTUnknown ;
  return m_ThisCamProperty[ iIndex ].m_DataType ;
};

PropertyDataType PtGrey2_0::GetPropertyDataType( LPCTSTR name )
{
  int iIndex = GetThisCamPropertyIndex( name ) ;
  if ( iIndex < 0 )
    return PDTUnknown ;
  return m_ThisCamProperty[ iIndex ].m_DataType ;
};

bool PtGrey2_0::DriverInit()
{
  return EnumCameras();
}
bool PtGrey2_0::EnumCameras()
{
  FXAutolock al( m_ConfigLock ) ;
  if ( m_bCamerasEnumerated && !m_bRescanCameras )
  {
    m_NConnectedDevices = m_iCamNum ;
    return true ;
  }

  FlyCapture2::CameraInfo AllGigECameras[ MAX_DEVICESNMB ];
  UINT uiGigECamNumber = MAX_DEVICESNMB;
  m_LastError = BusManager::DiscoverGigECameras( AllGigECameras , &uiGigECamNumber );
  m_LastError = m_BusManager.GetNumOfCameras( ( UINT* )&m_NConnectedDevices );

  ASSERT( m_NConnectedDevices < MAX_DEVICESNMB );

  if ( m_LastError != PGRERROR_OK )
  {
    if ( m_iNoCameraStatus++ == 0 )
      DEVICESENDERR_1( "Fatal error: %s" , m_LastError.GetDescription() );
    return false;
  }
  m_iCamNum = m_NConnectedDevices ;
  for ( unsigned i = 0; i < m_NConnectedDevices; i++ )
  {
    PGRGuid guid;
    Camera tmpC;
    FlyCapture2::CameraInfo ci;
    m_BusManager.GetCameraFromIndex( i , &guid );
    tmpC.Connect( &guid );
    tmpC.GetCameraInfo( &ci );
    tmpC.Disconnect() ;
    m_CamInfo[ i ].serialnmb = m_DevicesInfo[ i ].serialnmb = ci.serialNumber;
    m_CamInfo[ i + m_NConnectedDevices ].ulInterfaceType =
      m_DevicesInfo[ i + m_NConnectedDevices ].ulInterfaceType = ci.interfaceType;
    strcpy_s( m_DevicesInfo[ i ].m_sGivenName , ci.modelName );
    strcpy_s( m_CamInfo[ i ].m_sGivenName , m_DevicesInfo[ i ].m_sGivenName ) ;
    m_iNoCameraStatus = 0 ;
  }


  m_uiNGigECameras = uiGigECamNumber;


  m_bCamerasEnumerated = ( m_NConnectedDevices + m_uiNGigECameras > 0 ) ;
  m_bRescanCameras = false ;
  return true;
}

void PtGrey2_0::ShutDown()
{
  CDevice::ShutDown() ;
  if ( m_hGrabThreadHandle )
  {
    bool bRes = CamCNTLDoAndWait( PGR_EVT_SHUTDOWN , 1000 ) ;
    DWORD dwRes = WaitForSingleObject( m_hGrabThreadHandle , 2000 ) ;
    ASSERT( dwRes == WAIT_OBJECT_0 ) ;
  }
  FxReleaseHandle( m_evFrameReady ) ;
  FxReleaseHandle( m_evBusChange ) ;
}

void PtGrey2_0::AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame )
{
  if ( !pParamFrame )
    return;
  CTextFrame* tf = pParamFrame->GetTextFrame( DEFAULT_LABEL );
  if ( tf )
  {
    FXParser pk = ( LPCTSTR )tf->GetString();
    FXString cmd;
    FXString param;
    FXSIZE pos = 0;
    pk.GetWord( pos , cmd );

    if ( cmd.CompareNoCase( "list" ) == 0 )
    {
      pk.Empty();
      FXAutolock al( m_PropertyListLock ) ;
      for ( FXSIZE i = 0; i < m_ThisCamProperty.GetSize(); i++ )
      {
        //FXParser
        pk += m_ThisCamProperty[ i ].name; pk += "\r\n";
      }
    }
    else if ( ( cmd.CompareNoCase( "get" ) == 0 ) && ( pk.GetWord( pos , cmd ) ) )
    {
      if ( cmd == "camlist" )
      {
        if ( !m_bCamerasEnumerated || !m_iCamNum )
          pk = "No cameras" ;
        else
        {
          FXAutolock al( m_ConfigLock ) ;
          FXString CamList , CamDescription ;
          CamList.Format( "%d cameras: " , m_iCamNum ) ;
          bool bCamFound = false ;
          for ( int i = 0; i < m_iCamNum; i++ )
          {
            TCHAR cMark = _T( '+' ) ; // sign, that camera is free
            if ( m_dwSerialNumber != m_DevicesInfo[ i ].serialnmb )
            {
              for ( int j = 0 ; j < (int) m_BusyCameras.GetCount() ; j++ )
              {
                if ( m_dwSerialNumber != m_BusyCameras[ j ] )
                {
                  if ( m_DevicesInfo[ i ].serialnmb == m_BusyCameras[ j ] )
                  {
                    cMark = _T( '-' ) ; // sign, that camera is busy by other gadget
                    break ;
                  }
                }
              }
            }
            else
            {
              cMark = _T( '!' ) ;// this gadget camera sign
              bCamFound = true ;
            }
            CamDescription.Format( "%c%u_%s; " , cMark ,
              m_DevicesInfo[ i ].serialnmb ,
              ( LPCTSTR )m_DevicesInfo[ i ].m_sGivenName ) ;
            CamList += CamDescription ;
          }
          if ( !bCamFound && m_dwSerialNumber && ( m_dwSerialNumber != 0xffffffff ) )
          {
            CamDescription.Format( "?%u;" , m_dwSerialNumber ) ;
            CamList += CamDescription ;
          }
          pk = CamList ;
        }
      }
      else if ( cmd == "properties" )
      {
        bool bRes = PrintProperties( pk ) ;
        if ( !bRes )
          pk = "error" ;
      }
      else
      {
        unsigned id = GetPropertyId( cmd );
        if ( id != WRONG_PROPERTY )
        {
          FXSIZE value;
          bool bauto;
          bool bRes = GetCameraProperty( id , value , bauto ) ;
          if ( bRes )
          {
            if ( IsDigitField( id ) )
            {
              if ( bauto )
                pk = "auto";
              else
                pk.Format( "%d" , value );
            }
            else
              pk = ( LPCTSTR )value;
          }
          else
            pk = "error";
        }
        else
          pk = "error";
      }

    }
    else if ( ( cmd.CompareNoCase( "set" ) == 0 )
      && ( pk.GetWord( pos , cmd ) )
      && ( pk.GetParamString( pos , param ) ) )
    {
      if ( cmd == "properties" )
      {
        bool bInvalidate = false ;
        bool bRes = ScanProperties( param , bInvalidate ) ;
        pk = ( bRes ) ? "OK" : "error" ;
      }
      else
      {
        bool bRes = false ;
        unsigned iIndex = GetPropertyIndex( cmd );
        if ( iIndex < 0x7fffffff )
        {
          FXSIZE value = 0;
          bool bauto = false , Invalidate = false;
          if ( m_ThisCamProperty[(int)iIndex].id == FRAME_RATE )
          {
            FXString szNewFrameRate ;
            if ( param[0] == _T('+') || param[0] == _T('-') )
            {
              if ( m_bEmbedFrameInfo )
              {
                double dAddition = atof( param ) ;
                double dRatio = dAddition / (double)m_dwLastFramePeriod ;
                double dNewFrameRate = ( 1. - dRatio ) * m_fLastRealFrameRate ;
                szNewFrameRate.Format( _T( "%8.2f" ) , dNewFrameRate * 10. ) ;
                value = ( FXSIZE )( LPCTSTR )szNewFrameRate ;
              }
            }
            else
              value = ( FXSIZE )( LPCTSTR )param;
            if ( value )
              bRes = SetCameraProperty( iIndex , value , bauto , Invalidate ) ;
          }
          else
          {
          if ( m_ThisCamProperty[ (int)iIndex ].m_DataType == PDTInt )
          {
            if ( param.CompareNoCase( "auto" ) == 0 )
              bauto = true;
            else
              value = atoi( param );
          }
          else
            value = ( FXSIZE )( LPCTSTR )param;
          bool bWasStopped = m_bWasStopped ;
          m_bWasStopped = false ;
            bRes = SetCameraProperty( iIndex , value , bauto , Invalidate ) ;
          if ( !bWasStopped  && m_bWasStopped )
            CamCNTLDoAndWait( PGR_EVT_INIT | PGR_EVT_START_GRAB ) ;
          m_bWasStopped = bWasStopped ;
          }
          pk = ( bRes ) ? "OK" : "error" ;
        }
        else
          pk = "error";
      }
    }
    else
    {
      pk = "List of available commands:\r\n"
        "list - return list of properties\r\n"
        "get <item name> - return current value of item\r\n"
        "set <item name>(<value>) - change an item\r\n";
    }
    CTextFrame* retV = CTextFrame::Create( pk );
    retV->ChangeId( NOSYNC_FRAME );
    if ( !m_pControl->Put( retV ) )
      retV->RELEASE( retV );
  }
  pParamFrame->Release( pParamFrame );
}

bool PtGrey2_0::CameraInit()
{
#ifdef _DEBUG
  if ( CamCNTLDoAndWait( PGR_EVT_INIT , 1500000 ) )
#else
  if ( CamCNTLDoAndWait( PGR_EVT_INIT , 5000) )
#endif
  {
    return true ;
    //     if ( m_pCamera )
    //       return BuildPropertyList();
  }
  return false ;
}

bool PtGrey2_0::PtGreyCamInit()
{
  GetGadgetName( m_GadgetInfo ) ;
  DriverInit();


  if ( m_pCamera )
    PtGreyCamClose();

  //FXAutolock al( m_LocalConfigLock ) ;

  if ( !m_dwSerialNumber || !m_iCamNum
    || ( ( m_CurrentDevice != 0xffffffff ) && ( m_CurrentDevice >= ( unsigned )m_iCamNum ) ) )
  {  // nothing to connect
    //     if ( bMain )
    //       DEVICESENDERR_0("Fatal error: No PTGrey cameras found on a bus.");
    return false;
  }

  CAutoLockMutex_H alm( m_hConfigMutex , 5000 ) ;
  if ( alm.GetStatus() != WAIT_OBJECT_0 )
  {
    SEND_GADGET_ERR( "Can't get ConfigMutex: %u" , alm.GetStatus() );
    return false ;

  }
  m_LastError = m_BusManager.GetCameraFromSerialNumber(
    m_dwSerialNumber , &m_CurrentCameraGUID );
  if ( m_LastError != PGRERROR_OK )
  {
    if ( ++m_iNNoSerNumberErrors <= 2 )
      SEND_GADGET_ERR( "Fatal error GetCameraFromSerialNumber: %s" ,
      m_LastError.GetDescription() );
    return false;
  }
  m_iNNoSerNumberErrors = 0 ;

  m_LastError = m_BusManager.GetInterfaceTypeFromGuid( &m_CurrentCameraGUID , &m_IntfType );
  if ( m_LastError != PGRERROR_OK )
  {
    SEND_GADGET_ERR( "Fatal error GetInterfaceTypeFromGuid: %s" ,
      m_LastError.GetDescription() );
    return false;
  }

  m_Status.Empty() ;

  FlyCapture2::CameraBase*&  pCam = m_pCamera ;
  if ( m_IntfType == INTERFACE_GIGE )
    pCam = new GigECamera ;   // GIGE interface
  else
    pCam = new Camera ; // USB or 1394 interface

  if ( !pCam )
    return FALSE ;

//  m_iNSkipImages = 3 ;
  m_LastError = pCam->Connect( &m_CurrentCameraGUID );
  if ( m_LastError != PGRERROR_OK )
  {
    m_CurrentCameraGUID = PGRGuid() ;
    SEND_GADGET_ERR( "Fatal error Connect: %s" , m_LastError.GetDescription() );
    return false;
  }
  m_LastError = pCam->GetCameraInfo( &m_CameraInfo );
  if ( m_LastError != PGRERROR_OK )
    return false;

  Format7Info F7Info ;
  bool bSupported = false ;
  ( ( Camera* )m_pCamera )->GetFormat7Info( &F7Info , &bSupported ) ;
  if ( m_LastError != PGRERROR_OK )
  {
    DEVICESENDERR_1( "PtGrey2_0::GetFormat7Info can't get format 7 info, error: %s" , m_LastError.GetDescription() );
    return 0;
  }
  if ( !( m_pixelFormat & F7Info.pixelFormatBitField ) )
    m_pixelFormat = PIXEL_FORMAT_MONO8 ;
  
  if ( m_CurrentROI.left > ( LONG )F7Info.maxWidth || m_CurrentROI.top > ( LONG )F7Info.maxHeight
    || m_CurrentROI.right > ( LONG )F7Info.maxWidth || m_CurrentROI.bottom > ( LONG )F7Info.maxHeight )
  {
    m_CurrentROI = CRect( 0 , 0 , F7Info.maxWidth , F7Info.maxHeight ) ;
  }

  FC2Config   Config;
  m_pCamera->GetConfiguration( &Config );
  if ( m_LastError != PGRERROR_OK )
  {
    PtGreyCamClose() ;
    SEND_GADGET_ERR( "SetConfiguration Fatal error: %s" ,
      m_LastError.GetDescription() );
    return false;
  }
  Config.grabTimeout = 1000 ;
  Config.grabMode = DROP_FRAMES ;
  Config.numImageNotifications = 1 ;
  if ( m_IntfType == INTERFACE_IEEE1394 
    || m_IntfType == INTERFACE_USB2
    || m_IntfType == INTERFACE_USB3 )
  {
    Config.isochBusSpeed = BUSSPEED_S_FASTEST ;
    Config.asyncBusSpeed = BUSSPEED_S_FASTEST ;
    Config.bandwidthAllocation = BANDWIDTH_ALLOCATION_ON ;
    Config.numBuffers = 5 ;
  }
  else if ( m_IntfType == INTERFACE_GIGE )
  {

    unsigned int numStreamChannels = 0;
    m_LastError = ( ( GigECamera* )m_pCamera )->GetNumStreamChannels( &numStreamChannels );
    if ( m_LastError != PGRERROR_OK )
    {
      PtGreyCamClose() ;
      SEND_GADGET_ERR( "GetNumStreamChannels Fatal error: %s" ,
        m_LastError.GetDescription() );
      return false;
    }

    for ( unsigned int i = 0; i < numStreamChannels; i++ )
    {
      GigEStreamChannel streamChannel;
      m_LastError = ( ( GigECamera* )m_pCamera )->GetGigEStreamChannelInfo( i , &streamChannel );
      if ( m_LastError != PGRERROR_OK )
      {
        PtGreyCamClose() ;
        SEND_GADGET_ERR( "GetGigEStreamChannelInfo(%d) Fatal error: %s" , i ,
          m_LastError.GetDescription() );
        return false ;
      }

      streamChannel.destinationIpAddress.octets[ 3 ] = ( m_CameraInfo.applicationIPAddress >> 24 ) & 0xff ;
      streamChannel.destinationIpAddress.octets[ 2 ] = ( m_CameraInfo.applicationIPAddress >> 16 ) & 0xff ;
      streamChannel.destinationIpAddress.octets[ 1 ] = ( m_CameraInfo.applicationIPAddress >> 8 ) & 0xff ;
      streamChannel.destinationIpAddress.octets[ 0 ] = m_CameraInfo.applicationIPAddress & 0xff ;
      streamChannel.hostPort = m_CameraInfo.applicationPort ;
      m_uiPacketSize = streamChannel.packetSize = ( m_uiPacketSize < 1400 ) ? 1400 : 9000 ;

      m_LastError = ( ( GigECamera* )m_pCamera )->SetGigEStreamChannelInfo( i , &streamChannel );
      if ( m_LastError != PGRERROR_OK )
      {
        PtGreyCamClose() ;
        SEND_GADGET_ERR( "GetGigEStreamChannelInfo(%d) Fatal error: %s" , i ,
          m_LastError.GetDescription() );
        return false ;
      }
    }
    GigEImageSettingsInfo imageSettingsInfo ;
    m_LastError = ( ( GigECamera* )m_pCamera )->GetGigEImageSettingsInfo( &imageSettingsInfo );
    if ( m_LastError != PGRERROR_OK )
    {
      PtGreyCamClose() ;
      SEND_GADGET_ERR( "GetGigEImageSettingsInfo Fatal error: %s" ,
        m_LastError.GetDescription() );
      return false ;
    }
    m_LastError = pCam->SetConfiguration( &Config );
    if ( m_LastError != PGRERROR_OK )
    {
      PtGreyCamClose() ;
      DEVICESENDERR_1( "SetConfiguration Fatal error: %s" ,
        m_LastError.GetDescription() );
      return false;
    }

    GigEImageSettings imageSettings;
    imageSettings.offsetX = m_CurrentROI.left ;
    imageSettings.offsetY = m_CurrentROI.top ;
    imageSettings.height = m_CurrentROI.Width() ;
    imageSettings.width = m_CurrentROI.Height() ;
    imageSettings.pixelFormat = m_pixelFormat ;

    m_LastError = ( ( GigECamera* )m_pCamera )->SetGigEImageSettings( &imageSettings );
    if ( m_LastError != PGRERROR_OK )
    {
      PtGreyCamClose() ;
      SEND_GADGET_ERR( "GetGigEImageSettingsInfo Fatal error: %s" ,
        m_LastError.GetDescription() );
      return false;
    }
  }

  bool bColorCam = m_CameraInfo.isColorCamera /*(m_CameraInfo.bayerTileFormat != BayerTileFormat::NONE)*/ ;

//   PropertyInfo prpI ;
//   memset( &prpI , 0 , sizeof( prpI ) );
//   prpI.type = TRIGGER_MODE ;
//   m_LastError = m_pCamera->GetPropertyInfo( &prpI );
//   if ( ( m_LastError == PGRERROR_OK ) )
//   {
//     if ( prpI.present )
//     {
//       TriggerMode triggerMode;
//       m_LastError = m_pCamera->GetTriggerMode( &triggerMode );
//       m_TriggerMode = ( triggerMode.onOff ) ? ( ( triggerMode.polarity ) ? TriggerInv : TriggerOn ) : TriggerOff ;
//       if ( (m_TriggerMode > 0) ^ triggerMode.onOff )
//       {
//         memset( &triggerMode , 0 , sizeof( triggerMode ) ) ;
//         triggerMode.onOff = (m_TriggerMode > 0) ;
//         triggerMode.polarity = (m_TriggerMode > 1);
//         m_LastError = m_pCamera->SetTriggerMode( &triggerMode );
//       }
//     }
//     else
//       m_TriggerMode = TrigNotSupported ;
//   }
//   else
//     ASSERT( 0 ) ;

  if ( m_IntfType != INTERFACE_GIGE )
  {
    unsigned uiPacketSize = SetPacketSize(
      ( m_TriggerMode >= TriggerOn ) ? m_uiPacketSize : m_iFPSx10 ,
      ( m_TriggerMode < TriggerOn ) ) ;
    m_LastError = ( ( Camera* )m_pCamera )->SetFormat7Configuration( &m_f7is , uiPacketSize );
    if ( m_LastError != PGRERROR_OK )
    {
      PtGreyCamClose() ;
      DEVICESENDERR_1( "SetFormat7Configuration Fatal error: %s" ,
        m_LastError.GetDescription() );
      return false;
    }
    else
    {
      m_LastError = ( ( Camera* )m_pCamera )->
        GetFormat7Configuration( &m_f7is , &m_uiPacketSize , &m_fPercentage );
      TRACE( "Init Cam #%u Packet=%d (requested %d) Bus Percentage=%g\n " ,
        m_dwSerialNumber , m_uiPacketSize , uiPacketSize , ( double )m_fPercentage ) ;
    }
  }
  else  // GIGE
  {

  }

  if ( m_LastError != PGRERROR_OK )
  {
    m_Status.Format( "CameraInit(): %s" , m_LastError.GetDescription() );
  }
  int iLastChannel = -1 ;
  for ( int i = 0 ; i < 5 ; i++ )
  {
    StrobeInfo NewStrobe ;
    NewStrobe.source = i ;
    m_LastError = m_pCamera->GetStrobeInfo( &NewStrobe ) ;
    if ( m_LastError == PGRERROR_OK )
    {
      iLastChannel = i ;
      m_StrobeInfo.Add( NewStrobe ) ;
    }
  }
  m_CameraID.Format( "%d_%s" , m_dwSerialNumber , m_CameraInfo.modelName );
  FXString GadgetName ;
  BOOL bInLoop = GetGadgetName( GadgetName ) ;
  m_GadgetInfo.Format( "%s: %s" , GadgetName , m_CameraID );

  //   RegisterCallbacks() ;
  if ( m_Status.IsEmpty() )
  {
    if ( bInLoop )
      m_Status = "Connected" ;
  }
  if ( !m_Status.IsEmpty() )
  {
    DEVICESENDINFO_0( m_Status );
    m_Status.Empty() ;
  }
  m_bInitialized = true ;
  FXAutolock al( m_ConfigLock ) ;
  m_dwConnectedSerialNumber = m_dwSerialNumber ;
  int i = 0 ;
  for ( ; i < m_BusyCameras.GetCount() ; i++ )
  {
    if ( m_BusyCameras[ i ] == m_dwSerialNumber )
      break ;
  }
  if ( i >= m_BusyCameras.GetCount() )
    m_BusyCameras.Add( m_dwSerialNumber ) ;
  if ( PtGreyBuildPropertyList() )
  {
    UINT uiNChannels = 0 ;
    if ( m_pCamera->GetMemoryChannelInfo( &uiNChannels ) == PGRERROR_OK )
    {
      if ( m_iSaveSettingsState > 0  &&  uiNChannels > 0 )
        m_pCamera->RestoreFromMemoryChannel( 1 ) ;
    }
    TRACE( "PtGrey2_0::PtGreyCamInit - Camera %u initialized\n" ,
      m_CameraInfo.serialNumber ) ;
  }
  else
    PtGreyCamClose() ;

  return true;
}

void PtGrey2_0::PtGreyDeleteCam( FlyCapture2::CameraBase* pCam )
{
  if ( pCam )
  {
    if ( m_disableEmbeddedTimeStamp )
    {
      DisableEmbeddedTimeStamp( pCam );
    }
    //     UnregisterCallbacks() ;
    if ( pCam->IsConnected() )
      m_LastError = pCam->Disconnect();
    delete pCam ;
    FXAutolock al( m_ConfigLock ) ;
    m_dwConnectedSerialNumber = 0 ;
    for ( int i = 0 ; i < m_BusyCameras.GetCount() ; i++ )
    {
      if ( m_BusyCameras[ i ] == m_dwSerialNumber )
      {
        m_BusyCameras.RemoveAt( i ) ;
      }
    }
  }
  m_bInitialized = false ;
  TRACE( "PtGrey2_0::PtGreyDeleteCam - Camera %u deleted\n" , m_CameraInfo.serialNumber ) ;
}


void PtGrey2_0::PtGreyCamClose()
{
  if ( m_pCamera )
  {
    PtGreyCameraStop() ;

    PtGreyDeleteCam( m_pCamera );
    m_CallbackThreadName.Empty() ;
    //       m_Status.Format("CameraClose() %s: %s", bMain ? "Main" : "GUI" , m_LastError.GetDescription( ));
    //       DEVICESENDINFO_0(m_Status);
    m_CameraID = "-PtGrey2_0";
    m_CurrentCameraGUID = PGRGuid() ;
    m_pCamera = NULL ;
  }
  TRACE( "PtGrey2_0::PtGreyCamClose - Camera %u closed\n" , m_CameraInfo.serialNumber ) ;
}
void PtGrey2_0::PtGreyLocalStreamStart()
{
  if ( m_pCamera )
  {
    UINT uiRead ;
    m_LastError = m_pCamera->ReadRegister( 0x614 , &uiRead ) ;
    if ( m_LastError == PGRERROR_OK )
    {
      if ( uiRead == 0 ) // stopped
      {
        m_LastError = m_pCamera->WriteRegister( 0x614 , 0x80000000 ) ;
        TRACE( "PtGrey2_0::PtGreyLocalStreamStart - Camera %u stream started\n" , m_CameraInfo.serialNumber ) ;
      }
      m_bLocalStopped = false ;
      return ;
    }
  }
  TRACE( "PtGrey2_0::PtGreyLocalStreamStart - Camera %u stream not started\n" , m_CameraInfo.serialNumber ) ;
}
void PtGrey2_0::PtGreyLocalStreamStop()
{
  if ( m_pCamera )
  {
    UINT uiRead ;
    m_LastError = m_pCamera->ReadRegister( 0x614 , &uiRead ) ;
    if ( m_LastError == PGRERROR_OK )
    {
      if ( uiRead != 0 ) // not stopped
      {
        m_LastError = m_pCamera->WriteRegister( 0x614 , 0 ) ;
        TRACE( "PtGrey2_0::PtGreyLocalStreamStart - Camera %u stream stopped\n" , m_CameraInfo.serialNumber ) ;
      }
      m_bLocalStopped = true ;
      return ;
    }
  }
  TRACE( "PtGrey2_0::PtGreyLocalStreamStart - Camera %u stream not stopped\n" , m_CameraInfo.serialNumber ) ;
}


void PtGrey2_0::CameraClose()
{
  CamCNTLDoAndWait( PGR_EVT_RELEASE , 2000 ) ;
}

bool PtGrey2_0::DriverValid()
{

  return  ( m_iCamNum != 0 ) /*&& (m_pCamera && m_pCamera->IsConnected())*/;
}

bool PtGrey2_0::DeviceStart()
{
  CDevice::DeviceStart() ;
  m_bRun = TRUE ;

  if ( m_ControlThreadName.IsEmpty() && m_continueGrabThread && m_dwGrabThreadId )
  {
    GetGadgetName( m_ControlThreadName ) ;
    m_ControlThreadName += _T( "_CTRL" ) ;
    ::SetThreadName( ( LPCSTR )( ( LPCTSTR )m_ControlThreadName ) , m_dwGrabThreadId ) ;
  }
  bool bRes = CamCNTLDoAndWait( PGR_EVT_START_GRAB , 1000 )  ;
//   if ( bRes  &&  m_TriggerMode >= TriggerOn )
//   {
//     Sleep( 200 ) ;
//     bRes = CamCNTLDoAndWait( PGR_EVT_STOP_GRAB , 2000 ) ;
//     Sleep( 200 ) ;
//     bRes = CamCNTLDoAndWait( PGR_EVT_START_GRAB , 1000 )  ;
//   }
  return bRes ;
}

bool PtGrey2_0::PtGreyCameraStart()
{
  if ( m_dwSerialNumber && m_dwSerialNumber != ( -1 ) )
  {
    if ( !m_bIsRunning )
    {
      if ( !m_pCamera || m_bShouldBeReprogrammed || m_FrameCounter == 0 )
      {
        if ( !PtGreyCamInit() )
          return false ;
        else
          m_bShouldBeReprogrammed = false ;
      }

      m_BusEvents &= ~( PGR_EVT_START_GRAB ) ;
      Error error ;
      do
      {
        error = m_pCamera->StartCapture( NewImageCallBack , this ) ;
        if ( ( error == PGRERROR_OK ) || ( error == PGRERROR_ISOCH_ALREADY_STARTED ) )
        {
          TRACE( "PtGreyCameraStart - Camera %u %s started\n" ,
            m_dwSerialNumber , ( error == PGRERROR_OK ) ? "" : "already" ) ;
          m_bStopInitialized = false ;
          m_bIsRunning = true ;
          return true ;
        }
        if ( m_uiPacketSize > m_f7pi.unitBytesPerPacket )
        {
          m_uiPacketSize -= m_f7pi.unitBytesPerPacket ;
          m_LastError = ( ( Camera* )m_pCamera )->SetFormat7Configuration( &m_f7is , m_uiPacketSize );
          if ( m_LastError != PGRERROR_OK )
          {
            PtGreyCamClose() ;
            DEVICESENDERR_1( "SetFormat7Configuration Fatal error: %s" ,
              m_LastError.GetDescription() );
            return false;
          }
          else
          {
            m_LastError = ( ( Camera* )m_pCamera )->
              GetFormat7Configuration( &m_f7is , &m_uiPacketSize , &m_fPercentage );
            TRACE( "Format 7 Init Cam #%u Packet=%d Bus Percentage=%g\n " ,
              m_dwSerialNumber , m_uiPacketSize , ( double )m_fPercentage ) ;
            continue ;
          }
        }
        else
          break ;
      }
      while ( 1 );
      m_BusEvents &= ~( PGR_EVT_START_GRAB ) ;
      FXString Msg ;
      Msg.Format( "Start Failure: %s" , error.GetDescription() );
      LogError( Msg ) ;
      PtGreyCamClose() ;
      Sleep( 20 ) ;
      return false ; // will be reinitialized above
    }
    return true ;
  }
  return false ;
}

void PtGrey2_0::DeviceStop()
{
  m_bRun = FALSE ;
  if ( !m_pCamera || !m_continueGrabThread )
    return;

  CamCNTLDoAndWait( PGR_EVT_STOP_GRAB , 1000 )  ;
  CDevice::DeviceStop();

  SetEvent( m_evFrameReady ) ;
  TRACE( "+++ PtGrey2_0::CameraStop() for #%u\n " , m_dwSerialNumber );
}

void PtGrey2_0::PtGreyCameraStop() // for calling from ControlLoop
{
  if ( !m_pCamera || !m_continueGrabThread )
    return;
  if ( m_bIsRunning )
  {
//     TriggerMode triggerMode;
//     bool bTriggerWasOn = false ;
//     if ( m_TriggerMode != TrigNotSupported )
//     {
//       m_LastError = m_pCamera->GetTriggerMode( &triggerMode );
//       if ( triggerMode.onOff )
//       {
//         triggerMode.onOff = false;
//         m_pCamera->SetTriggerMode( &triggerMode );
//         bTriggerWasOn = true ;
//         Sleep( 30 ) ;
//       }
//     }
    m_bStopInitialized = true ;
    m_LastError = m_pCamera->StopCapture();
    m_bIsRunning = false ;
    m_GrabLock.Lock() ;
    if ( m_pNewImage )
    {
      delete m_pNewImage ;
      m_pNewImage = NULL ;
    }
    m_GrabLock.Unlock() ;

    if ( m_LastError != PGRERROR_OK
      && m_LastError != PGRERROR_ISOCH_NOT_STARTED )
    {
      DEVICESENDERR_1( "Stop Failure: %s" , m_LastError.GetDescription() );
    }
//     if ( bTriggerWasOn )
//     {
//       triggerMode.onOff = true;
//       m_pCamera->SetTriggerMode( &triggerMode );
//     }
  }

  TRACE( "+++ PtGreyCameraStop()for #%u\n " , m_dwSerialNumber );
}

CVideoFrame* PtGrey2_0::DeviceDoGrab( double* StartTime )
{
  if ( !m_pCamera )
  {
    Sleep( 500 ) ;
    *StartTime = GetHRTickCount() ;
    return NULL ;
  }
  CVideoFrame * pOut = NULL ;
  // waiting for frame or exit
  DWORD dwRes = WaitForMultipleObjects( 2 , m_WaitEventFrameArray , FALSE , /*INFINITE*/ 1000 ) ;
  if ( dwRes == WAIT_OBJECT_0 )
  {
    m_dLastStartTime = ::GetHRTickCount() ;
    *StartTime = m_dLastStartTime - m_dLastInCallbackTime ;
    m_GrabLock.Lock() ;
    FlyCapture2::Image * pImage = m_pNewImage ;
    m_pNewImage = NULL ;
    m_GrabLock.Unlock() ;
    if ( !m_pCamera && pImage )
    {
      delete pImage ;
    }
    else if ( pImage )
    {
      unsigned int rows , cols , stride;
      PixelFormat format;
      BayerTileFormat BayerFormat ;

      pImage->GetDimensions( &rows , &cols , &stride , &format , &BayerFormat );
      if ( format == m_pixelFormat )
      {
        switch ( format )
        {
          case PIXEL_FORMAT_RAW8:
          case PIXEL_FORMAT_MONO8:
          {
            unsigned iSize = rows * cols ;
            LPBYTE data = ( LPBYTE )pImage->GetData() ;

            pTVFrame pNew = makeNewY8Frame( cols , rows ) ;
            if ( pNew )
            {
              memcpy( ( LPBYTE )( pNew->lpBMIH + 1 ) , data , iSize ) ;
              pOut = CVideoFrame::Create( pNew ) ;
            }
          }
          break ;
          case PIXEL_FORMAT_MONO16:
          case PIXEL_FORMAT_RAW16:
          {
            unsigned iSize = rows * cols * 2 ;
            LPBYTE data = ( LPBYTE )pImage->GetData() ;

            pTVFrame pNew = makeNewY16Frame( cols , rows ) ;
            if ( pNew )
            {
              memcpy( ( LPBYTE )( pNew->lpBMIH + 1 ) , data , iSize ) ;
              pOut = CVideoFrame::Create( pNew ) ;
            }
          }
          break ;
          case PIXEL_FORMAT_411YUV8:
          {
            unsigned iSize = ( 3 * rows * cols ) / 2 ;
            BITMAPINFOHEADER BMIH ;

            memset( &BMIH , 0 , sizeof( BITMAPINFOHEADER ) );
            BMIH.biSize = sizeof( BITMAPINFOHEADER );
            BMIH.biHeight = rows;
            BMIH.biWidth = cols;
            BMIH.biSizeImage = iSize;
            BMIH.biCompression = BI_YUV411 ;
            BMIH.biBitCount = 12 ;
            BMIH.biPlanes = 1;
            LPBYTE data = pImage->GetData() ;
            TVFrame TF = { &BMIH, data };
//            LPBITMAPINFOHEADER lpYUV9 = yuv411yuv9( &BMIH , data ) ;
            LPBITMAPINFOHEADER lpYUV9 = _convertY411YUV9( &TF );
            if ( lpYUV9 )
            {
              CVideoFrame* vf = CVideoFrame::Create();
              vf->lpBMIH = lpYUV9 ;
              pOut = vf ;
            }
          }
          break ;
          case PIXEL_FORMAT_422YUV8:
          {
            unsigned iSize = 2 * rows * cols ;
            BITMAPINFOHEADER BMIH ;

            memset( &BMIH , 0 , sizeof( BITMAPINFOHEADER ) );
            BMIH.biSize = sizeof( BITMAPINFOHEADER );
            BMIH.biHeight = rows;
            BMIH.biWidth = cols;
            BMIH.biSizeImage = iSize;
            BMIH.biCompression = BI_UYVY ;
            BMIH.biBitCount = 16 ;
            BMIH.biPlanes = 1;
            TVFrame Tmp ;
            Tmp.lpBMIH = &BMIH ;
            Tmp.lpData = pImage->GetData() ;

            LPBITMAPINFOHEADER lpYUV9 = _convertUYVY2YUV12( &Tmp );
            if ( lpYUV9 )
            {
              CVideoFrame* vf = CVideoFrame::Create();
              vf->lpBMIH = lpYUV9 ;
              pOut = vf ;
            }
          }
          break ;
          case PIXEL_FORMAT_RGB8: // the same with PIXEL_FORMAT_RGB
          {
            unsigned iSize = ( 3 * rows * cols ) ;
            LPBITMAPINFOHEADER lpBMIH = ( LPBITMAPINFOHEADER )malloc( sizeof( BITMAPINFOHEADER ) + iSize );
            LPBYTE data = pImage->GetData() ;

            memset( lpBMIH , 0 , sizeof( BITMAPINFOHEADER ) );
            lpBMIH->biSize = sizeof( BITMAPINFOHEADER );
            lpBMIH->biHeight = rows;
            lpBMIH->biWidth = cols;
            lpBMIH->biSizeImage = iSize;
            lpBMIH->biCompression = BI_RGB ;
            lpBMIH->biBitCount = 24 ;
            lpBMIH->biPlanes = 1;

            LPBITMAPINFOHEADER pYUV9 = rgb24yuv9( lpBMIH , data ) ;
            CVideoFrame* vf = CVideoFrame::Create();
            vf->lpBMIH = pYUV9 ;
            pOut = vf ;
            delete lpBMIH ;
          }
          break ;
          default:
            if ( !m_FormatNotSupportedDispayed )
            {
              DEVICESENDERR_1( "Format not supported: 0x%x" , format );
              m_FormatNotSupportedDispayed = true;
            }
            break ;
        }
        if ( pOut )
        {
          pOut->lpData = NULL;
          pOut->SetTime( GetHRTickCount() );
          if ( !m_bEmbedFrameInfo )
            pOut->SetLabel( m_CameraID );
          else
          {
            pFrameInfo pInfo = (pFrameInfo) GetData( pOut );
            FXString Label;
            DWORD dwBusTime = _byteswap_ulong( pInfo->m_dwTimeStamp );
            DWORD dwFrameCnt = _byteswap_ulong( pInfo->m_dwFrameCntr );
            m_dwLastFramePeriod = dwBusTime - m_dwPrevBusTime;
//             Label.Format( _T( "%u %d %s" ) , m_dwLastFramePeriod ,
//               dwFrameCnt - m_dwPrevBusFrameCnt , m_CameraID );
            Label.Format( _T( "%u %d %8.3f %s" ) , m_dwLastFramePeriod ,
              dwFrameCnt - m_dwPrevBusFrameCnt , 
              (double)m_fLastRealFrameRate , m_CameraID );

            m_dwPrevBusTime = dwBusTime;
            m_dwPrevBusFrameCnt = dwFrameCnt ;
            pOut->SetLabel( Label );
          }
          
        }
        
      }
      delete pImage ;
    }
  }
  else if ( dwRes == WAIT_TIMEOUT )
  {
    if ( !m_pCamera || !m_pCamera->IsConnected() )
    {
      if ( m_dwSerialNumber && ( m_dwSerialNumber != ( 0 - 1 ) ) )
      {
        m_BusEvents |= PGR_EVT_START_GRAB ;
        SetEvent( m_evBusChange ) ;
        SetEvent( m_evFrameReady ) ;
      }
    }
    *StartTime = GetHRTickCount() ;
  }
  return pOut ;
}

int PtGrey2_0::ReceiveImage()
{
  Error error;
  if ( m_pCamera && m_pCamera->IsConnected() )
  {
    FlyCapture2::Image * pImage = new FlyCapture2::Image ;
    error = m_pCamera->RetrieveBuffer( pImage );

    if ( error == PGRERROR_OK )
    {
      m_GrabLock.Lock() ;
      if ( m_pNewImage )
      {
        delete m_pNewImage ;
        m_pNewImage = NULL ;
      }
      m_pNewImage = pImage ;
      SetEvent( m_evFrameReady ) ;
      m_GrabLock.Unlock() ;
      return 0 ;
    }
    else
    {
      delete pImage ;
      if ( error != PGRERROR_TIMEOUT )
      {
        //         LogError( error.GetDescription() ) ;
        return error.GetType() ;
      }
      return 0 ;
    }
  }
  return -1 ;  // no camera state /*_T("No Camera") ;*/
}


bool PtGrey2_0::BuildPropertyList()
{
  bool bRes = CamCNTLDoAndWait( PGR_EVT_BUILD_PROP , 5000 ) ;
  return ( bRes && m_ThisCamProperty.GetCount() ) ;
}


bool PtGrey2_0::PtGreyBuildPropertyList( bool bOutside )
{
  m_ThisCamProperty.RemoveAll();
  PgrCamProperty P;

  if ( bOutside && !m_pCamera && !PtGreyCamInit() )
    return false ;

  int iWhiteBalanceMode = GetColorMode() ; // 0 - no color, 1 - by program, 2 - by camera


  for ( int i = 0; i < _getPropertyCount(); i++ )
  {
    P.Reset() ;
    if ( ( int )cProperties[ i ].id < BRIGHTNESS ) // virtual property
    {
      switch ( ( int )cProperties[ i ].id )
      {
        case FORMAT_MODE:
        {
          if ( m_IntfType != INTERFACE_GIGE )
          {
            int iNmb = sizeof( vFormats ) / sizeof( Videoformat );
            FXString items , tmpS;
            bool supported;
            Format7Info f7i;
            f7i.mode = MODE_0;
            ( ( Camera* )m_pCamera )->GetFormat7Info( &f7i , &supported );
            if ( !supported )
            {
              DEVICESENDERR_0( "Camera doesn't support Format7 mode 0" );
              break;
            }
            for ( int j = 0; j < iNmb; j++ )
            {
              if ( vFormats[ j ].vm & f7i.pixelFormatBitField )
              {
                tmpS.Format( _T( "%s%s(%d)" ) , ( j != 0 ) ? _T( "," ) : _T( "" ) ,
                  vFormats[ j ].name , vFormats[ j ].vm );
                items += tmpS;
              }
            }
            P.name = cProperties[ i ].name;
            P.id = ( PropertyType )cProperties[ i ].id;
            P.m_DataType = PDTInt ;
            P.bSupported = true ;
            P.m_GUIFormat.Format( "ComboBox(%s(%s))" , P.name , items );
          }
          break;
        }
        case SETROI:
        {
          P.name = cProperties[ i ].name;
          P.id = ( PropertyType )cProperties[ i ].id;
          P.m_GUIFormat.Format( "EditBox(%s)" , P.name );
          P.m_DataType = PDTString ;
          P.bSupported = true ;
          break;
        }
        case SETSTROBE0:
        case SETSTROBE1:
        case SETSTROBE2:
        case SETSTROBE3:
        {
          int iIndex = SETSTROBE0 - ( unsigned )cProperties[ i ].id ;
          if ( iIndex < m_StrobeInfo.GetCount() )
          {
            if ( m_StrobeInfo[ iIndex ].present )
            {
              P.name = cProperties[ i ].name;
              P.id = ( PropertyType )cProperties[ i ].id;
              P.m_GUIFormat.Format( "EditBox(%s)" , P.name );
              P.m_DataType = PDTString ;
              P.bSupported = true ;
            }
          }
          break;
        }
        case MAX_PACKET_SIZE:
          P.name = cProperties[ i ].name;
          P.id = (PropertyType) cProperties[ i ].id;
          P.m_GUIFormat.Format( "ComboBox(%s(No(0),Yes(1)))" ,
            (LPCTSTR)cProperties[ i ].name );
          P.m_DataType = PDTInt;
          P.bSupported = true;
          break;
        case EMBED_INFO:
          P.name = cProperties[ i ].name;
          P.id = ( PropertyType )cProperties[ i ].id;
          P.m_GUIFormat.Format( "ComboBox(%s(No(0),Yes(1)))" ,
            ( LPCTSTR )cProperties[ i ].name );
          P.m_DataType = PDTInt;
          P.bSupported = true;
          break;
        case PACKETSIZE:
          if ( m_TriggerMode >= TriggerOn && !m_bMaxPacketSize )
          {
            P.name = cProperties[ i ].name;
            P.id = ( PropertyType )cProperties[ i ].id;
            P.m_GUIFormat.Format( "EditBox(%s)" , P.name );
            P.m_DataType = PDTString ;
            P.bSupported = true ;
          }
          break ;
        case WHITE_BAL_RED:
        case WHITE_BAL_BLUE:
        {
          if ( iWhiteBalanceMode == COLOR_BY_SOFTWARE ) // Program white balance
          {
            P.name = cProperties[ i ].name ;
            P.id = ( PropertyType )cProperties[ i ].id;
            P.m_GUIFormat.Format( "Spin(%s,0,1023)" , P.name );
            P.m_DataType = PDTInt ;
            P.bSupported = true ;
          }
        }
        break ;
        case SAVE_SETTINGS:
        {
          UINT iNSettingsChannels = 0 ;
          if ( m_pCamera->GetMemoryChannelInfo( &iNSettingsChannels ) == PGRERROR_OK )
          {
            P.name = cProperties[ i ].name ;
            P.id = ( PropertyType )cProperties[ i ].id;
            P.m_GUIFormat.Format( "Spin(%s,0,%d)" , P.name , iNSettingsChannels + 1 );
            P.m_DataType = PDTInt ;
            P.bSupported = true ;
          }
        }
        break ;
        case RESTART_TIMEOUT:
        {
          P.name = cProperties[ i ].name ;
          P.id = ( PropertyType )cProperties[ i ].id;
          P.m_GUIFormat.Format( "Spin(%s,0,%d)" , P.name , 1000000 );
          P.m_DataType = PDTInt ;
          P.bSupported = true ;
        }
        break ;
        default:
          DEVICESENDERR_1( "Undefined property:'%s'" , cProperties[ i ].name );
      }
    }
    else
    {
      if ( ( cProperties[ i ].id == WHITE_BALANCE ) && ( iWhiteBalanceMode == 0 ) )
      { // no white balance for BW images
      }
      else
      {
        PropertyInfo  prpI;
        memset( &prpI , 0 , sizeof( prpI ) );
        prpI.type = ( PropertyType )cProperties[ i ].id;
        m_LastError = m_pCamera->GetPropertyInfo( &prpI );
        if ( ( m_LastError == PGRERROR_OK ) && ( prpI.present ) )
        {
          P.name = cProperties[ i ].name;
          P.id = ( PropertyType )cProperties[ i ].id;
          switch ( prpI.type )
          {
            case TRIGGER_MODE:
            {
              P.m_GUIFormat.Format( "ComboBox(%s(NoTrigger(0),Trigger0+(1),Trigger0-(2)))" , cProperties[ i ].name );
              TriggerMode triggerMode;
              m_LastError = m_pCamera->GetTriggerMode( &triggerMode );
              m_TriggerMode = ( triggerMode.onOff ) ?
                ( ( triggerMode.polarity ) ? TriggerInv : TriggerOn ) : TriggerOff ;
              P.m_DataType = PDTInt ;
              P.bSupported = true ;
              P.LastValue.iInt = ( int )m_TriggerMode ;
            }
            break ;
            case TRIGGER_DELAY:
              if ( m_TriggerMode != TriggerOff )
              {
                P.m_GUIFormat.Format( "%s(%s,%d,%d)" ,
                  SETUP_SPIN , P.name , 0 , 65000000 /*prpI.min , prpI.max*/ );
                P.m_DataType = PDTInt ;
                P.bSupported = true ;
              }
              break ;
            case SHUTTER:
            {
              if ( prpI.absValSupported )
              {
                P.m_GUIFormat.Format( "%s(%s,%d,%d)" ,
                  ( prpI.autoSupported ) ? SETUP_SPINABOOL : SETUP_SPIN ,
                  P.name , ( int )( prpI.absMin * 1000. ) , ( int )( prpI.absMax * 1000. ) );
                P.m_DataType = PDTFloat ;
              }
              else
              {
                P.m_GUIFormat.Format( "%s(%s,%d,%d)" ,
                  ( prpI.autoSupported ) ? SETUP_SPINABOOL : SETUP_SPIN ,
                  P.name , prpI.min , prpI.max );
                P.m_DataType = PDTInt ;
              }
              P.bSupported = true ;
            }
            break ;
            case GAIN:
            {
              if ( prpI.absValSupported )
              {
                P.m_GUIFormat.Format( "%s(%s,%d,%d)" ,
                  ( prpI.autoSupported ) ? SETUP_SPINABOOL : SETUP_SPIN ,
                  P.name , ( int )( prpI.absMin * 10. ) , ( int )( prpI.absMax * 10. ) );
                P.m_DataType = PDTFloat ;
              }
              else
              {
                P.m_GUIFormat.Format( "%s(%s,%d,%d)" ,
                  ( prpI.autoSupported ) ? SETUP_SPINABOOL : SETUP_SPIN ,
                  P.name , prpI.min , prpI.max );
                P.m_DataType = PDTInt ;
              }
              P.bSupported = true ;
            }
            break ;
            case FRAME_RATE:
            {
              if ( m_TriggerMode == TriggerOff )
              {
                if ( prpI.absValSupported )
                {
                  P.m_GUIFormat.Format( "%s(%s,%d,%d)" ,
                    /*(prpI.autoSupported)?SETUP_SPINABOOL:*/SETUP_SPIN ,
                    P.name , ( int )( prpI.absMin * 10. ) , 10000/*(int)(prpI.absMax * 10.)*/ );
                  P.m_DataType = PDTFloat ;
                }
                else
                {
                  P.m_GUIFormat.Format( "%s(%s,%d,%d)" ,
                    ( prpI.autoSupported ) ? SETUP_SPINABOOL : SETUP_SPIN ,
                    P.name , prpI.min , prpI.max );
                  P.m_DataType = PDTInt ;
                }
                P.bSupported = true ;
              }
            }
            break ;
            case WHITE_BALANCE:
            case SATURATION:
            case HUE:
            {
              if ( iWhiteBalanceMode == COLOR_BY_CAMERA ) // Program white balance
              {
                P.m_GUIFormat.Format( "%s(%s,%d,%d)" ,
                  ( prpI.autoSupported ) ? SETUP_SPINABOOL : SETUP_SPIN ,
                  P.name , prpI.min , prpI.max );
                P.m_DataType = PDTInt ;
                P.bSupported = true ;
              }
            }
            break ;
            default:
            {
              P.m_GUIFormat.Format( "%s(%s,%d,%d)" ,
                ( prpI.autoSupported ) ? SETUP_SPINABOOL : SETUP_SPIN ,
                P.name , prpI.min , prpI.max );
              P.m_DataType = PDTInt ;
              P.bSupported = true ;
            }
            break ;
          }
        }
        else if ( m_LastError == PGRERROR_OK )
        {
          switch ( prpI.type )
          {
            case TRIGGER_MODE:
              m_TriggerMode = TrigNotSupported ;
              break ;

            default:
              break;
          }
        }
      }
    }
    if ( !P.m_GUIFormat.IsEmpty() )
    {
      FXAutolock al( m_PropertyListLock ) ;
      m_ThisCamProperty.Add( P );
    }
  }
  return true;
}

bool PtGrey2_0::GetCameraProperty( PropertyType Type , FXSIZE &value , bool& bauto )
{
  int iIndex = GetThisCamPropertyIndex( Type ) ;
  if ( iIndex >= 0 )
  {
    if ( Type == RESTART_TIMEOUT )
    {
      value = m_iRestartTimeOut_ms ;
      return true ;
    }
    return GetCameraProperty( iIndex , value , bauto ) ;
  }
  return false ;
}


bool PtGrey2_0::GetCameraProperty( int iIndex , FXSIZE &value , bool& bauto )
{
  m_PropertyDataReturn.Reset() ;
  if ( !OtherThreadGetProperty( iIndex , &m_PropertyDataReturn ) )
    return false ;
  bauto = m_PropertyDataReturn.m_bAuto ;
  FXAutolock al( m_PropertyListLock ) ;
  switch ( m_ThisCamProperty[ iIndex ].m_DataType )
  {
    case PDTInt:
      value = m_PropertyDataReturn.m_int ;
      m_ThisCamProperty[ iIndex ].LastValue.iInt = m_PropertyDataReturn.m_int ;
      return true ;
    case PDTBool:
      value = ( FXSIZE )m_PropertyDataReturn.m_bBool ;
      m_ThisCamProperty[ iIndex ].LastValue.iInt = (int)value ;
      m_ThisCamProperty[ iIndex ].LastValue.bBool = ( value != 0 ) ;
      return true ;
    case PDTString:
      value = ( FXSIZE )m_PropertyDataReturn.m_szString ;
      _tcscpy_s( m_ThisCamProperty[ iIndex ].LastValue.szAsString , m_PropertyDataReturn.m_szString ) ;
      return true ;
    case PDTFloat:
      m_ThisCamProperty[ iIndex ].LastValue.dDouble = m_PropertyDataReturn.m_double ;
      sprintf_s( m_PropertyDataReturn.m_szString , "%g" , m_PropertyDataReturn.m_double ) ;
      value = ( FXSIZE )m_PropertyDataReturn.m_szString ;
      return true ;
  }
  return false;
}

bool PtGrey2_0::PtGreyGetCameraProperty( int iIndex , SetCamPropertyData * pData )
{
  if ( !pData )
    return false ;
  if ( !m_pCamera && !PtGreyCamInit() )
    return false ;
  FXAutolock al( m_PropertyListLock ) ;
  PropertyType Type = GetThisCamPropertyType( iIndex ) ;
  if ( Type == UNSPECIFIED_PROPERTY_TYPE )
    return false ;

  pData->m_bAuto = false;
  if ( ( ( int )Type ) < BRIGHTNESS ) // virtual property
  {
    switch ( Type )
    {
      case FORMAT_MODE:
      {
        pData->m_int = m_pixelFormat ;
        return true;
      }
      case SETROI:
      {
        static FXString sROI;
        CRect rc;
        GetROI( rc );
        sprintf_s( pData->m_szString , "%d,%d,%d,%d" , rc.left , rc.top , rc.right , rc.bottom );
        return true;
      }
      case SETSTROBE0:
      case SETSTROBE1:
      case SETSTROBE2:
      case SETSTROBE3:
      {
        int iIndex = SETSTROBE0 - ( int )Type ;
        if ( iIndex < m_StrobeInfo.GetCount() )
        {
          if ( m_StrobeInfo[ iIndex ].present )
          {
            FXString StrobeDataAsText ;
            GetStrobe( StrobeDataAsText , iIndex ) ;
            strcpy_s( pData->m_szString , ( LPCTSTR )StrobeDataAsText ) ;
            return true ;
          }
        }
        return false ;
      }
      case MAX_PACKET_SIZE:
        pData->m_int = m_bMaxPacketSize ;
        return true ;
      case EMBED_INFO:
        pData->m_int = m_bEmbedFrameInfo ;
        return true ;
      case PACKETSIZE:
      {
        if ( m_IntfType != INTERFACE_GIGE )
        {
          unsigned int PacketSize;
          float Percentage;
          Format7ImageSettings f7is;
          m_LastError = ( ( Camera* )m_pCamera )->GetFormat7Configuration( &f7is , &PacketSize , &Percentage );
          if ( m_LastError == PGRERROR_OK )
          {
            FXString sPacketSize ;
            sprintf_s( pData->m_szString , "%d" , PacketSize ) ;
            m_uiPacketSize = PacketSize ;
            return true ;
          }
          else
          {
            DEVICESENDERR_1( "Can't take PAcket Size: %s" , m_LastError.GetDescription() ) ;
            return false ;
          }
        }
        else
        {

        }
      }
      break ;
      case WHITE_BAL_RED:
        pData->m_int = m_iWBRed ;
        return true ;
      case WHITE_BAL_BLUE:
        pData->m_int = m_iWBlue ;
        return true ;
      case SAVE_SETTINGS:
        pData->m_int = 0 ; // m_iSaveSettingsState ;
        return true ;
      case RESTART_TIMEOUT:
        pData->m_int = m_iRestartTimeOut_ms ;
        return true ;
      default:
        DEVICESENDERR_1( "Undefined property:'%s'" , GetThisCamPropertyName( Type ) );
    }
  }
  else
  {
    pData->m_bBool = false;
    FlyCapture2::Property prp;
    memset( &prp , 0 , sizeof( prp ) );

    prp.type = Type ;
    m_LastError = m_pCamera->GetProperty( &prp );
    if ( ( prp.present ) && ( m_LastError == PGRERROR_OK ) )
    {
      switch ( Type )
      {
        case TRIGGER_MODE:
        {
          TriggerMode triggerMode;
          m_LastError = m_pCamera->GetTriggerMode( &triggerMode );
          pData->m_int = ( triggerMode.onOff ) ? 1 : 0;
          if ( pData->m_int && ( triggerMode.polarity != 0 ) )
            pData->m_int = 2 ;
          return true;
        }
        case TRIGGER_DELAY:
        {
          TriggerDelay tDelay ;
          tDelay.absControl = true;
          m_LastError = m_pCamera->GetTriggerDelay( &tDelay );

          if ( m_LastError == PGRERROR_OK )
          {
            pData->m_int = ( int )( tDelay.absValue * 1.0e6 ) ; // convert from seconds to microseconds
            return true ;
          }
          return false ;
        }
        case SHUTTER:
        {
          pData->m_bAuto = prp.autoManualMode ;
          if ( m_ThisCamProperty[ iIndex ].m_DataType == PDTFloat )
          {
            pData->m_double = prp.absValue * 1000. ;
            pData->m_int = ROUND( pData->m_double ) ;
//             TRACE( "\nPtGrey2_0::PtGreyGetCameraProperty Shutter float %d , auto=%d" , 
//               pData->m_int , ( pData->m_bAuto ) ? 1 : 0 ) ;
          }
          else
          {
            pData->m_int = prp.valueA ;
//             TRACE( "\nPtGrey2_0::PtGreyGetCameraProperty Shutter int %d , auto=%d" , 
//               pData->m_int , ( pData->m_bAuto ) ? 1 : 0 ) ;
          }
          return true ;
        }
        case GAIN:
          pData->m_bAuto = prp.autoManualMode ;
        case FRAME_RATE:
          if ( m_ThisCamProperty[ iIndex ].m_DataType == PDTFloat )
          {
            pData->m_double = prp.absValue * 10. ;
            pData->m_int = ROUND( pData->m_double ) ;
          }
          else
            pData->m_int = prp.valueA ;
          return true ;
        default:
        {
          if ( prp.autoManualMode )
            pData->m_bAuto = true;
          pData->m_int = prp.valueA ;
          return true;
        }
      }
    }
  }
  return false;
}

bool PtGrey2_0::SetCameraProperty( int iIndex , FXSIZE &value , bool& bauto , bool& Invalidate )
{
#ifdef _DEBUG
#ifdef DEBUG_TRIGGER_MODE
  TriggerMode triggerModeDebugIn;
  if ( ( m_ThisCamProperty[ iIndex ].id != TRIGGER_MODE ) && m_pCamera )
  {
    m_LastError = m_pCamera->GetTriggerMode( &triggerModeDebugIn );
//     ASSERT( m_ThisCamProperty[ iIndex ].id != PACKETSIZE ) ;
//     {
//     }
  }
#endif
#endif
  bool bRes = SetCameraPropertyEx( iIndex , value , bauto , Invalidate ) ;
#ifdef _DEBUG
#ifdef DEBUG_TRIGGER_MODE
  if ( ( m_ThisCamProperty[ iIndex ].id != TRIGGER_MODE ) && m_pCamera )
  {
    TriggerMode triggerModeDebugOut;
    m_LastError = m_pCamera->GetTriggerMode( &triggerModeDebugOut );
    if ( triggerModeDebugIn.onOff != triggerModeDebugOut.onOff )
    {
      TriggerMode triggerMode;
      triggerMode.onOff = (m_TriggerMode != 0);
      triggerMode.polarity = (m_TriggerMode) ? 1 : 0 ;
      triggerMode.mode = 0;
      triggerMode.parameter = 0;
      triggerMode.source = 0;
      m_LastError = m_pCamera->SetTriggerMode( &triggerMode );
    }  ;
  }
#endif
#endif

  return bRes ;

}


bool PtGrey2_0::SetCameraProperty( PropertyType Type , FXSIZE &value , bool& bauto , bool& Invalidate )
{
  int iIndex = GetThisCamPropertyIndex( Type ) ;
  if ( iIndex >= 0 )
  {
    if ( Type == RESTART_TIMEOUT )
    {
      m_iRestartTimeOut_ms = (int)value ;
      return true ;
    }
    return SetCameraProperty( iIndex , value , bauto , Invalidate ) ;
  }
  return false;
}

bool PtGrey2_0::SetCameraPropertyEx( int iIndex , FXSIZE &value , bool& bauto , bool& Invalidate )
{
  m_PropertyDataSend.Reset() ;
  m_PropertyDataSend.m_bAuto = bauto ;
  m_PropertyListLock.Lock() ;
  PropertyDataType Type = m_ThisCamProperty[ iIndex ].m_DataType ;
  m_PropertyListLock.Unlock() ;
  TRACE( "PtGrey2_0::SetCameraPropertyEx %s " , m_ThisCamProperty[ iIndex ].name ) ;
  switch ( Type )
  {
    case PDTInt: 
      m_PropertyDataSend.m_int = (int)value ; 
      if ( m_ThisCamProperty[iIndex].id != FORMAT_MODE )
        TRACE( "%d auto=%d\n" , (int)value , ( int )bauto ) ;
      else
      {
        LPCTSTR pName = GetPixelFormatName( ( PixelFormat )value ) ;
        TRACE( "%s(0x%08X) auto=%d\n" , (pName)? pName : "Unknown" , value , ( int )bauto ) ;
      }

      break ;
    case PDTBool: 
      m_PropertyDataSend.m_bBool = ( value != 0 ) ; 
      TRACE( "%s auto=%d\n" , ( value != 0 )? "true" : "false" , ( int )bauto ) ;
      break ;
    case PDTString: 
      strcpy_s( m_PropertyDataSend.m_szString , ( LPCTSTR )value ) ; 
      TRACE( "%s auto=%d\n" , ( LPCTSTR )value , ( int )bauto ) ;
      break ;
    case PDTFloat: 
      m_PropertyDataSend.m_double = atof( ( LPCTSTR )value ) ; 
      TRACE( "%s auto=%d\n" , ( LPCTSTR )value , ( int )bauto ) ;
      break ;
  }
  
  bool bRes = OtherThreadSetProperty( iIndex , &m_PropertyDataSend , &Invalidate ) ;
  if ( bRes )
  {
    Invalidate = m_PropertyDataSend.m_bInvalidate ;
    return true ;
  }
  return false ;
}

bool PtGrey2_0::SetCameraPropertyEx( PropertyType Type , FXSIZE &value , bool& bauto , bool& Invalidate )
{
  int iIndex = GetThisCamPropertyIndex( Type ) ;
  if ( iIndex >= 0 )
    return SetCameraPropertyEx( iIndex , value , bauto , Invalidate ) ;
  return false ;
}

bool PtGrey2_0::PtGreyCheckAndStopCapture()
{
  bool bWasRunning = IsRunning() ;
  if ( bWasRunning )
  {
    PtGreyCameraStop() ;
    m_BusEvents &= ~( PGR_EVT_STOP_GRAB ) ;
    Sleep( 10 ) ;
    m_bWasStopped = true ;
  }
  return bWasRunning ;
}

bool PtGrey2_0::PtGreyInitAndConditialStart()
{
  if ( PtGreyCamInit() )
  {
    if ( m_bWasStopped )
      return PtGreyCameraStart();
    return true ;
  }
  else
    return false ;
}

bool PtGrey2_0::PtGreySetCameraProperty( int iIndex , SetCamPropertyData * pData )
{
  if ( !pData )
    return false ;
  PropertyType Type = GetThisCamPropertyType( iIndex ) ;
  if ( Type == UNSPECIFIED_PROPERTY_TYPE )
    return false ;
  if ( !m_pCamera && !PtGreyCamInit() )
    return false ;

  SetCamPropertyData Data = *pData ;

  if ( ( ( int )Type ) < BRIGHTNESS ) // virtual property
  {
    switch ( Type )
    {
      case FORMAT_MODE:
      {
        if ( m_IntfType != INTERFACE_GIGE )
        {
          PtGreyCheckAndStopCapture() ;
          m_pixelFormat = ( PixelFormat )Data.m_int ;
          pData->m_bInvalidate = true;
          TRACE( " PtGrey2_0::PtGreySetCameraProperty: Set Pixel Format %s  \n" , GetPixelFormatName( m_pixelFormat ) ) ;
          if ( m_bInScanProperties )
            m_bShouldBeReprogrammed = true ;
          else if ( m_continueGrabThread )
            return PtGreyInitAndConditialStart() ;
          return ( true );
        }
        else // GIGE
        {

        }
      }
      case SETROI:
      {
        CRect rc;
        rc.left = 0 ;
        TRACE( "PtGrey2_0::PtGreySetCameraProperty: SetROI %s  \n" , Data.m_szString ) ;
        if ( sscanf( Data.m_szString , "%d,%d,%d,%d" , &rc.left , &rc.top , &rc.right , &rc.bottom ) == 4 )
        {
          rc.right += rc.left ;
          rc.bottom += rc.top ;
          SetROI( rc );
          return true;
        }
        else if ( rc.left == -1 )
        {
          SetROI( rc ) ; // max ROI will be settled
          return true ;
        }
        return false;
      }
      case SETSTROBE0:
      case SETSTROBE1:
      case SETSTROBE2:
      case SETSTROBE3:
      {
        int iIndex = SETSTROBE0 - ( int )Type ;
        TRACE( "PtGrey2_0::PtGreySetCameraProperty: SetStrobe %d %s  \n" , iIndex , Data.m_szString ) ;
        if ( iIndex < m_StrobeInfo.GetCount() )
        {
          FXString StrobeDataAsText = ( LPCTSTR )Data.m_szString ;
          return SetStrobe( StrobeDataAsText , iIndex ) ;
        }
        else
          return false;
      }
      case MAX_PACKET_SIZE:
        PtGreyCheckAndStopCapture();
        if ( pData->m_int )
        {  // Save current packet size
          UINT uiMaxPacketSize = GetMaxPacketSize();
          if ( uiMaxPacketSize != m_uiPacketSize )
            m_uiSavedPacketSize = m_uiPacketSize;
            SetMaxPacketSize();
          }
        else
        {
          int iMaxPacketSize = GetMaxPacketSize() ;
          if ( !m_uiSavedPacketSize || ( iMaxPacketSize != m_uiSavedPacketSize ) )
            m_bMaxPacketSize = TRUE ;
          else 
            m_bMaxPacketSize = FALSE ;
          SetPacketSize( m_uiSavedPacketSize , false );
      }
        if ( m_bInScanProperties )
          m_bShouldBeReprogrammed = true;
        else if ( m_continueGrabThread )
          return PtGreyInitAndConditialStart();
        return true;
      case EMBED_INFO:
        m_bEmbedFrameInfo = ( Data.m_int != 0 );
        m_pCamera->WriteRegister( 0x12F8 , ( m_bEmbedFrameInfo ) ? 0x000003FF : 0x0 );
        return true ;
      case PACKETSIZE:
      {
        if ( ( int )m_TriggerMode >= ( int )TriggerOn )
        {
          TRACE( "PtGrey2_0::PtGreySetCameraProperty: PacketSize %s  \n" , Data.m_szString ) ;
          if ( m_IntfType != INTERFACE_GIGE )
          {
            PtGreyCheckAndStopCapture() ;
            int iPacketSize = _tstoi( Data.m_szString ) ;

            iPacketSize = SetPacketSize( iPacketSize , false ) ;
            ASSERT( iPacketSize > 0 ) ;
            //            pData->m_bInvalidate = true;

            Sleep( 30 ) ;
            m_uiPacketSize = iPacketSize ;
            m_LastError = ( ( Camera* )m_pCamera )->SetFormat7Configuration( &m_f7is , m_uiPacketSize );
            if ( m_LastError != PGRERROR_OK )
            {
              PtGreyCamClose() ;
              DEVICESENDERR_1( "SetFormat7Configuration Fatal error: %s" ,
                m_LastError.GetDescription() );
              return false;
            }
            else
            {
              m_LastError = ( ( Camera* )m_pCamera )->
                GetFormat7Configuration( &m_f7is , &m_uiPacketSize , &m_fPercentage );
              TRACE( "\nFormat 7 Cam #%u Packet=%d Bus Percentage=%g\n " ,
                m_dwSerialNumber , m_uiPacketSize , ( double )m_fPercentage ) ;
            }
            if ( m_bInScanProperties )
              m_bShouldBeReprogrammed = true ;
            else if ( m_continueGrabThread )
              return PtGreyInitAndConditialStart() ;
            return true ;
          }
          else
          {

          }
        }
      }
      break ;
      case WHITE_BAL_RED:
      {
        Property whitebalance;
        whitebalance.type = WHITE_BALANCE;
        whitebalance.autoManualMode = Data.m_bAuto ;
        TRACE( "PtGrey2_0::PtGreySetCameraProperty: WB Red %d   \n" , Data.m_int ) ;

        m_LastError = m_pCamera->GetProperty( &whitebalance );
        if ( m_LastError == PGRERROR_OK )
        {
          whitebalance.valueA = m_iWBRed = Data.m_int ;
          m_LastError = m_pCamera->SetProperty( &whitebalance );
          if ( m_LastError == PGRERROR_OK )
            return true ;
        }
        break ;
      }
      case WHITE_BAL_BLUE:
      {
        Property whitebalance;
        whitebalance.type = WHITE_BALANCE;
        whitebalance.autoManualMode = Data.m_bAuto ;
        TRACE( "PtGrey2_0::PtGreySetCameraProperty: WB Blue %d auto=%d  \n" , Data.m_int , Data.m_bAuto == true ) ;

        m_LastError = m_pCamera->GetProperty( &whitebalance );
        if ( m_LastError == PGRERROR_OK )
        {
          whitebalance.valueB = m_iWBlue = Data.m_int ;
          m_LastError = m_pCamera->SetProperty( &whitebalance );
          if ( m_LastError == PGRERROR_OK )
            return true ;
        }
      }
      break ;
      case SAVE_SETTINGS:
      {
        m_iSaveSettingsState = Data.m_int ;
        
        TRACE( "PtGrey2_0::PtGreySetCameraProperty: SaveSettings=%d \n" , 
          m_iSaveSettingsState ) ;
        if ( Data.m_int > 0 )
          m_pCamera->SaveToMemoryChannel( 1 ) ;
        //         pData->m_bInvalidate = true ;
      }
      break ;
      default:
        DEVICESENDERR_1( "Undefined property:'%s'" ,
          GetThisCamPropertyName( Type ) );
    }
  }
  else
  {
    FlyCapture2::Property prp;
    memset( &prp , 0 , sizeof( prp ) );
    prp.type = Type ;

    m_LastError = m_pCamera->GetProperty( &prp );
    if ( ( m_LastError == PGRERROR_OK ) && ( prp.present ) )
    {
      switch ( Type )
      {
        case TRIGGER_MODE:
        {
          TriggerMode triggerMode;
          m_LastError = m_pCamera->GetTriggerMode( &triggerMode );
          triggerMode.onOff = ( Data.m_int != 0 );
          triggerMode.polarity = ( Data.m_int > 1 ) ? 1 : 0 ;
          triggerMode.mode = 0;
          triggerMode.parameter = 0;
          triggerMode.source = 0;
          m_LastError = m_pCamera->SetTriggerMode( &triggerMode );
          m_TriggerMode = ( CamTriggerMode )( Data.m_int ) ;
          pData->m_bInvalidate = true ;
          m_LastError = m_pCamera->GetTriggerMode( &triggerMode );
          TRACE( " PtGrey2_0::PtGreySetCameraProperty: Trigger %d \n" , Data.m_int ) ;
          return ( m_LastError == PGRERROR_OK );
        }
        case TRIGGER_DELAY:
        {
          TriggerDelay tDelay ;
          tDelay.absControl = true;
          tDelay.absValue = ( float )( ( double )Data.m_int / 1.0e6 ) ; // convert from microseconds to seconds
          tDelay.onOff = true ;
          tDelay.autoManualMode = false ;
          m_LastError = m_pCamera->SetTriggerDelay( &tDelay );
          TRACE( " PtGrey2_0::PtGreySetCameraProperty: Trigger Delay %d usec \n" , Data.m_int ) ;
          return ( m_LastError == PGRERROR_OK );
        }
        case SHUTTER:
        {
          prp.absControl = true;
          prp.absValue = ( float )( Data.m_double / 1000. ) ;
          prp.onOff = true ;
          prp.autoManualMode = Data.m_bAuto ;
          TRACE( " PtGrey2_0::PtGreySetCameraProperty: '%s' to %f, auto=%d\n" ,
            GetThisCamPropertyName( iIndex ) , Data.m_double , ( int )Data.m_bAuto );
          m_LastError = m_pCamera->SetProperty( &prp );

          //FlyCapture2::Property PropRead ;
          //PropRead.type = AUTO_EXPOSURE ;
          //m_LastError = m_pCamera->GetProperty( &PropRead ) ;
          //PropRead.onOff = Data.m_bAuto ;
          //m_LastError = m_pCamera->SetProperty( &PropRead ) ;
          return ( m_LastError == PGRERROR_OK );
        }
        case GAIN:
          prp.absControl = true;
          prp.absValue = ( float )( Data.m_double / 10. ) ;
          prp.onOff = true ;
          prp.autoManualMode = Data.m_bAuto ;
          TRACE( "PtGrey2_0::PtGreySetCameraProperty: '%s' to %f\n" ,
            GetThisCamPropertyName( iIndex ) , Data.m_double );
          m_LastError = m_pCamera->SetProperty( &prp );
          return true ;
        case FRAME_RATE:
          {
            m_iFPSx10 = ( int )Data.m_double ;
            double dFrameRate = Data.m_double * 0.1;
            return SetFrameRate( dFrameRate );
          }

//           if ( ( int )m_TriggerMode < ( int )TriggerOn )
//           {
//             int iOldPacketSize = m_uiPacketSize ;
//             UINT iNewPacketSize = SetPacketSize( m_iFPSx10 , true ) ; // adjust packet size
//             bool bPacketChanged = ( iOldPacketSize != iNewPacketSize ) ;
//             if ( bPacketChanged )
//             {
//               if ( m_IntfType != INTERFACE_GIGE )
//               {
//                 ASSERT( iNewPacketSize > 0 ) ;
//                 PtGreyCheckAndStopCapture() ;
//                 if ( m_bInScanProperties && m_bWasStopped )
//                   m_bShouldBeReprogrammed = true ;
//                 Format7Info f7i ;
//                 bool bSupported = false ;
//                 ( ( FlyCapture2::Camera* )m_pCamera )->GetFormat7Info( &f7i , &bSupported ) ;
//                 if ( iNewPacketSize > f7i.maxPacketSize )
//                   iNewPacketSize = f7i.maxPacketSize ;
//                 Format7ImageSettings f7is ;
//                 UINT uiCurrentPacketSize ;
//                 float fPercentage ;
//                 ( ( FlyCapture2::Camera* )m_pCamera )->GetFormat7Configuration(
//                   &f7is , &uiCurrentPacketSize , &fPercentage ) ;
//                 m_LastError = ( ( FlyCapture2::Camera* )m_pCamera )->
//                   SetFormat7Configuration( &f7is , ( unsigned )iNewPacketSize );
//                 TRACE( "\nPtGrey2_0::PtGreySetCameraProperty: FPS to %f, Packet=%d " ,
//                   Data.m_double , iNewPacketSize );
//                 if ( m_LastError == PGRERROR_OK )
//                 {
//                   m_uiPacketSize = iNewPacketSize ;
//                   m_LastError = ( ( FlyCapture2::Camera* )m_pCamera )->
//                     GetFormat7Configuration( &m_f7is , &m_uiPacketSize , &m_fPercentage );
//                   TRACE( " Cam #%u Packet=%d Bus Percentage=%g\n" , m_dwSerialNumber ,
//                     m_uiPacketSize , ( double )m_fPercentage ) ;
//                 }
//                 else
//                   DEVICESENDERR_1( "Set Format 7 for FPS error: %s" , m_LastError.GetDescription() );
//               }
//               else
//               {
//               }
//             }
//             PropertyInfo  prpI;
//             memset( &prpI , 0 , sizeof( prpI ) );
//             prpI.type = FRAME_RATE ;
//             m_LastError = m_pCamera->GetPropertyInfo( &prpI );
//             double dFPS = Data.m_double / 10. ;
//             if ( dFPS < prpI.absMin )
//               dFPS = prpI.absMin ;
//             if ( dFPS > prpI.absMax )
//               dFPS = prpI.absMax ;
//             prp.absControl = true;
//             prp.absValue = ( float )dFPS ;
//             prp.onOff = true ;
//             prp.autoManualMode = false ;
//             m_LastError = m_pCamera->SetProperty( &prp );
//             TRACE( "\nPtGrey2_0::PtGreySetCameraProperty: '%s' to %f (ordered %f), read %f " ,
//               GetThisCamPropertyName( iIndex ) , dFPS * 10. , 
//               Data.m_double , ( double )prp.absValue * 10. );
// 
//             if ( m_LastError == PGRERROR_OK )
//             {
//               bool bRes = true ;
//               if ( bPacketChanged )
//               {
//                 if ( m_IntfType != INTERFACE_GIGE )
//                 {
//                   if ( !m_bInScanProperties && m_continueGrabThread )
//                     bRes = PtGreyInitAndConditialStart() ;
//                 }
//                 else
//                 {
// 
//                 }
//               }
//               m_pCamera->GetProperty( &prp ) ;
//               return bRes ;
//             }
//             else
//             {
//               TRACE( "+++ PgrSetProp '%s' Error %s\n" , GetThisCamPropertyName( iIndex ) , m_LastError.GetDescription() );
//               DEVICESENDERR_2( " PgrSetProp %s error %s" , GetThisCamPropertyName( iIndex ) , m_LastError.GetDescription() ) ;
//             }
//           }
          return false ;
        default:
        {
          prp.absControl = false;
          prp.valueA = Data.m_int ;
          prp.onOff = true /*( prp.type != AUTO_EXPOSURE )*/ ;
          prp.autoManualMode = Data.m_bAuto ;
          TRACE( "\nPtGrey2_0::PtGreySetCameraProperty: '%s' to %d" ,
            GetThisCamPropertyName( iIndex ) , Data.m_int );
          m_LastError = m_pCamera->SetProperty( &prp );
          return ( m_LastError == PGRERROR_OK );
        }
      }
    }
  }
  return false;
}

void PtGrey2_0::GetROI( CRect& rc )
{
  rc = m_CurrentROI;
  rc.right -= rc.left ;
  rc.bottom -= rc.top ;
}

void PtGrey2_0::SetROI( CRect& rc )
{
  if ( !m_pCamera )
    return ;

  if (
    ( m_CurrentROI.Width() == rc.Width() ) &&
    ( m_CurrentROI.Height() == rc.Height() )
    )
  {

    // if format 7 and size the same simply do origin shift
    // by writing into IMAGE_ORIGIN register
    // without grab stop
    m_pCamera->WriteRegister( 0xa08 , ( rc.left << 16 ) | rc.top );
    m_CurrentROI = rc;
    return ;
  }

  PtGreyCheckAndStopCapture() ;

  Format7Info fi;
  bool supported = false ;

  ( ( Camera* )m_pCamera )->GetFormat7Info( &fi , &supported );
  if ( rc.left != -1 )
  {
    rc.left &= ~3;
    if ( rc.left < 0 || rc.left >( int )fi.maxWidth )
      rc.left = 0 ;

    rc.right &= ~3;
    if ( rc.right < 0 || rc.right >( int )fi.maxWidth )
      rc.right = fi.maxWidth ;
    rc.top &= ~3;
    if ( rc.top < 0 || rc.top >( int )fi.maxHeight )
      rc.top = 0 ;
    rc.bottom &= ~3;
    if ( rc.bottom < 0 || rc.bottom >( int )fi.maxHeight )
      rc.bottom = fi.maxHeight ;
  }
  else
  {
    rc.left = rc.top = 0;
    rc.right = fi.maxWidth ;
    rc.bottom = fi.maxHeight;
  }
  m_CurrentROI = rc ;
  //   FXAutolock al(m_GrabLock);
  UINT iPacketSize = 0 ;
  if ( ( int )m_TriggerMode < ( int )TriggerOn ) // FPS
    iPacketSize = SetPacketSize( m_iFPSx10 , true ) ;
  else // packet size
    iPacketSize = SetPacketSize( m_uiPacketSize , false ) ;

  if ( iPacketSize )
  {
    m_LastError = ( ( Camera* )m_pCamera )->
      SetFormat7Configuration( &m_f7is , iPacketSize );
    if ( m_LastError == PGRERROR_OK )
    {
      m_LastError = ( ( Camera* )m_pCamera )->
        GetFormat7Configuration( &m_f7is , &m_uiPacketSize , &m_fPercentage );
      if ( m_bInScanProperties  &&  m_bWasStopped )
        m_bShouldBeReprogrammed = true ;
      TRACE( " Cam #%u Packet=%d Bus Percentage=%g\n" , m_dwSerialNumber ,
        m_uiPacketSize , ( double )m_fPercentage ) ;
    }
    else
      DEVICESENDERR_1( "Set Format 7 for ROI error: %s" , m_LastError.GetDescription() );
  }
}

bool PtGrey2_0::SetStrobe( const FXString& StrobeDataAsText , int iIndex )
{
  if ( !m_pCamera )
    return false;

  StrobeControl SControl ;
  int iDelay , iDuration , iOnOff ;
  if ( sscanf( ( LPCTSTR )StrobeDataAsText , _T( "%d,%d,%d,%d" ) ,
    &iOnOff , &SControl.polarity ,
    &iDelay , &iDuration ) == 4 )
  {
	SControl.onOff = (iOnOff != 0);
    SControl.delay = ( float )( ( double )iDelay * 1.e-3 ) ;
    SControl.duration = ( float )( ( double )iDuration * 1.e-3 ) ;
    SControl.source = iIndex ;
    m_LastError = m_pCamera->SetStrobe( &SControl ) ;
    if ( m_LastError != PGRERROR_OK )
    {
      DEVICESENDERR_2( "Set Strobe %d error: %s" , iIndex , m_LastError.GetDescription() );
      return false ;
    }
    return true ;
  }

  return false ;
}

void PtGrey2_0::GetStrobe( FXString& StrobeDataAsText , int iIndex )
{
  if ( !m_pCamera )
    return ;

  StrobeControl SControl ;
  SControl.source = iIndex ;
  m_LastError = m_pCamera->GetStrobe( &SControl ) ;
  if ( m_LastError != PGRERROR_OK )
  {
    DEVICESENDERR_1( "Get Strobe error: %s" , m_LastError.GetDescription() );
  }
  else
  {
    StrobeDataAsText.Format( _T( "%d,%d,%d,%d" ) ,
      SControl.onOff , SControl.polarity ,
      _ROUND( SControl.delay * 1.e3 ) , _ROUND( SControl.duration * 1.e3 ) ) ;
  }
}

void PtGrey2_0::GetCamResolutionAndPixelFormat(
  unsigned int* rows , unsigned int* cols , PixelFormat* pixelFmt )
{
  if ( !m_pCamera )
    return ;

  // get the current source-image settings
  Error error;
  VideoMode videoMode;
  FrameRate frameRate;
  CameraInfo camInfo;

  error = m_pCamera->GetCameraInfo( &camInfo );

  if ( camInfo.interfaceType == INTERFACE_GIGE )
  {
    GigECamera* gigeCam = ( GigECamera* )( m_pCamera );
    GigEImageSettings gigeImageSettings;
    error = gigeCam->GetGigEImageSettings( &gigeImageSettings );
    *cols = gigeImageSettings.width;
    *rows = gigeImageSettings.height;
    *pixelFmt = gigeImageSettings.pixelFormat;
  }
  else
  {
    Camera* pCam = static_cast< Camera* >( m_pCamera );
    error = pCam->GetVideoModeAndFrameRate( &videoMode , &frameRate );

    bool isStippled = false;

    if ( videoMode == VIDEOMODE_FORMAT7 )
    {
      Format7ImageSettings f7ImageSettings;
      unsigned int packetSize;
      float percentage;

      error = pCam->GetFormat7Configuration( &f7ImageSettings , &packetSize , &percentage );

      *cols = f7ImageSettings.width;
      *rows = f7ImageSettings.height;
      *pixelFmt = f7ImageSettings.pixelFormat;
    }
    else
    {
      // if white balance property is present then stippled is true. This detects
      // when camera is in Y8/Y16 and raw bayer output is enabled
      PropertyInfo propInfo;
      propInfo.type = WHITE_BALANCE;

      m_pCamera->GetPropertyInfo( &propInfo );

      if ( propInfo.present )
      {
        isStippled = true;
      }

      if ( !GetPixelFormatFromVideoMode( videoMode , isStippled , pixelFmt ) )
      {
        *pixelFmt = PIXEL_FORMAT_RAW8;
      }
      GetDimensionsFromVideoMode( videoMode , rows , cols );
    }
  }
}

BOOL PtGrey2_0::GetDimensionsFromVideoMode(
  VideoMode mode , unsigned int* rows , unsigned int* cols )
{
  if ( rows == NULL || cols == NULL )
  {
    return FALSE;
  }

  switch ( mode )
  {
    case VIDEOMODE_160x120YUV444:
      *cols = 160;
      *rows = 120;
      break;
    case VIDEOMODE_320x240YUV422:
      *cols = 320;
      *rows = 240;
      break;
    case VIDEOMODE_640x480YUV411:
    case VIDEOMODE_640x480YUV422:
    case VIDEOMODE_640x480RGB:
    case VIDEOMODE_640x480Y8:
    case VIDEOMODE_640x480Y16:
      *cols = 640;
      *rows = 480;
      break;
    case VIDEOMODE_800x600YUV422:
    case VIDEOMODE_800x600RGB:
    case VIDEOMODE_800x600Y8:
    case VIDEOMODE_800x600Y16:
      *cols = 800;
      *rows = 600;
      break;
    case VIDEOMODE_1024x768YUV422:
    case VIDEOMODE_1024x768RGB:
    case VIDEOMODE_1024x768Y8:
    case VIDEOMODE_1024x768Y16:
      *cols = 1024;
      *rows = 768;
      break;
    case VIDEOMODE_1280x960YUV422:
    case VIDEOMODE_1280x960RGB:
    case VIDEOMODE_1280x960Y8:
    case VIDEOMODE_1280x960Y16:
      *cols = 1280;
      *rows = 960;
      break;
    case VIDEOMODE_1600x1200YUV422:
    case VIDEOMODE_1600x1200RGB:
    case VIDEOMODE_1600x1200Y8:
    case VIDEOMODE_1600x1200Y16:
      *cols = 1600;
      *rows = 1200;
      break;
    default:
      return FALSE;
  }
  return TRUE;
}

BOOL PtGrey2_0::GetPixelFormatFromVideoMode(
  VideoMode mode ,
  bool stippled ,
  PixelFormat* pixFormat )
{
  switch ( mode )
  {
    case VIDEOMODE_640x480Y8:
    case VIDEOMODE_800x600Y8:
    case VIDEOMODE_1024x768Y8:
    case VIDEOMODE_1280x960Y8:
    case VIDEOMODE_1600x1200Y8:
      if ( stippled )
      {
        *pixFormat = PIXEL_FORMAT_RAW8;
      }
      else
      {
        *pixFormat = PIXEL_FORMAT_MONO8;
      }
      break;
    case VIDEOMODE_640x480Y16:
    case VIDEOMODE_800x600Y16:
    case VIDEOMODE_1024x768Y16:
    case VIDEOMODE_1280x960Y16:
    case VIDEOMODE_1600x1200Y16:
      if ( stippled )
      {
        *pixFormat = PIXEL_FORMAT_RAW16;
      }
      else
      {
        *pixFormat = PIXEL_FORMAT_MONO16;
      }
      break;
    case VIDEOMODE_640x480RGB:
    case VIDEOMODE_800x600RGB:
    case VIDEOMODE_1024x768RGB:
    case VIDEOMODE_1280x960RGB:
    case VIDEOMODE_1600x1200RGB:
      *pixFormat = PIXEL_FORMAT_RGB;
      break;
    case VIDEOMODE_320x240YUV422:
    case VIDEOMODE_640x480YUV422:
    case VIDEOMODE_800x600YUV422:
    case VIDEOMODE_1024x768YUV422:
    case VIDEOMODE_1280x960YUV422:
    case VIDEOMODE_1600x1200YUV422:
      *pixFormat = PIXEL_FORMAT_422YUV8;
      break;
    case VIDEOMODE_160x120YUV444:
      *pixFormat = PIXEL_FORMAT_444YUV8;
      break;
    case VIDEOMODE_640x480YUV411:
      *pixFormat = PIXEL_FORMAT_411YUV8;
      break;
    case VIDEOMODE_FORMAT7:
      return FALSE;
    default:
      return FALSE;
  }
  return TRUE;
}

unsigned int PtGrey2_0::GetBppFromPixelFormat( PixelFormat pixelFormat )
{
  switch ( pixelFormat )
  {
    case PIXEL_FORMAT_MONO8:
    case PIXEL_FORMAT_RAW8:
      return 8;
      break;
    case PIXEL_FORMAT_411YUV8:
    case PIXEL_FORMAT_MONO12:
    case PIXEL_FORMAT_RAW12:
      return 12;
      break;
    case PIXEL_FORMAT_MONO16:
    case PIXEL_FORMAT_S_MONO16:
    case PIXEL_FORMAT_422YUV8:
    case PIXEL_FORMAT_RAW16:
      return 16;
      break;
    case PIXEL_FORMAT_444YUV8:
    case PIXEL_FORMAT_RGB:
    case PIXEL_FORMAT_BGR:
      return 24;
      break;
    case PIXEL_FORMAT_BGRU:
    case PIXEL_FORMAT_RGBU:
      return 32;
      break;
    case PIXEL_FORMAT_S_RGB16:
    case PIXEL_FORMAT_RGB16:
    case PIXEL_FORMAT_BGR16:
      return 48;
      break;
    default:
      return 0;
      break;
  }
}

bool PtGrey2_0::CheckCameraPower( FlyCapture2::CameraBase * pCam )
{
  if ( !pCam )
    return false ;

  unsigned int powerReg;
  unsigned int PowerInqReg;

  // Make sure camera supports power control
  Error error = pCam->ReadRegister( 0x400 , &PowerInqReg );

  // Only proceed if there was no error and power control was supported
  if ( ( error == PGRERROR_OK ) && ( ( PowerInqReg & 0x00008000 ) != 0 ) )
  {
    error = pCam->ReadRegister( 0x610 , &powerReg );
    if ( error == PGRERROR_OK )
    {
      powerReg = powerReg >> 31;
      return ( powerReg != 0 ) ;
    }
  }
  return false ;
}

bool PtGrey2_0::IsGrabThreadRunning()
{
  return m_continueGrabThread;
}

bool PtGrey2_0::EnableEmbeddedTimeStamp( FlyCapture2::CameraBase* cam )
{
  if ( cam != NULL && cam->IsConnected() )
  {
    Error error = cam->GetEmbeddedImageInfo( &m_embeddedInfo );
    if ( error != PGRERROR_OK )
    {
      return false;
    }

    if ( m_embeddedInfo.timestamp.available && !m_embeddedInfo.timestamp.onOff )
    {
      m_embeddedInfo.timestamp.onOff = true;
      error = cam->SetEmbeddedImageInfo( &m_embeddedInfo );
      if ( error != PGRERROR_OK )
      {
        return false;
      }
      else
      {
        return true;
      }
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}

bool PtGrey2_0::DisableEmbeddedTimeStamp( FlyCapture2::CameraBase* cam )
{
  if ( cam != NULL && cam->IsConnected() )
  {
    m_embeddedInfo.timestamp.onOff = false;
    Error error = cam->SetEmbeddedImageInfo( &m_embeddedInfo );
    if ( error != PGRERROR_OK )
    {
      return false;
    }
    else
    {
      return true;
    }
  }
  else
  {
    return false;
  }
}

DWORD WINAPI PtGrey2_0::ControlLoop( LPVOID pParam )
{
  TRACE( "---------Entry to ControlLoop\n" ) ;
  PtGrey2_0 * pGadget = ( PtGrey2_0* )pParam ;
  Error error;
  FXString csMessage;
  BOOL isCorruptFrame = FALSE;
  unsigned int cols = 0 , rows = 0 , colsPrev = 0 , rowsPrev = 0;
  DWORD dwWaitRes = 0 ;
  pGadget->m_continueGrabThread = true ;
  pGadget->RegisterCallbacks() ;
  // Start of main grab loop
  while ( pGadget->m_continueGrabThread )
  {
    dwWaitRes = WaitForMultipleObjects( 2 ,
      pGadget->m_WaitEventBusChangeArr , FALSE ,
      ( !( pGadget->m_bInitialized ) ) ? 500 : 1000 ) ;
    if ( dwWaitRes == WAIT_FAILED )  // gadget deleted
    {
      DWORD dwError = GetLastError() ;
      break ;
    }
    if ( dwWaitRes == WAIT_TIMEOUT  &&  pGadget->m_bInitialized 
      && pGadget->m_iRestartTimeOut_ms > 1000  &&  !pGadget->m_bTriedToRestart )
    {
      FXAutolock al( pGadget->m_GrabLock ) ;
      double dTimeFromLastFrame = GetHRTickCount() - pGadget->m_dLastFrameTime ;
      if ( dTimeFromLastFrame > pGadget->m_iRestartTimeOut_ms 
        && ( pGadget->m_TriggerMode < TriggerOn ) )
      {
        pGadget->m_BusEvents |= PGR_EVT_INIT ;
        pGadget->m_bTriedToRestart = true ;
      }
    }
    if ( dwWaitRes == WAIT_OBJECT_0 || pGadget->m_bTriedToRestart ) // some bus or command event message
    {
      DWORD InitialBusEvents = pGadget->m_BusEvents ;
      int iNInits = 0 ;
      while ( pGadget->m_BusEvents )
      {
        if ( iNInits > 5 )
        {
          pGadget->m_BusEvents = 0 ;
          break ;
        }
        if ( !pGadget->m_dwSerialNumber || ( pGadget->m_dwSerialNumber == 0xffffffff ) )
        {
          if ( !( pGadget->m_BusEvents & PGR_EVT_SHUTDOWN ) )
          {
            pGadget->m_BusEvents = 0 ;
            break ;
          }
        }
        if ( !pGadget->m_bRun )
          pGadget->m_BusEvents &= ~( PGR_EVT_START_GRAB | PGR_EVT_LOCAL_START ) ;
        //         FXAutolock al( pGadget->m_LocalConfigLock , "ControlLoop") ;
        if ( pGadget->m_BusEvents & PGR_EVT_SHUTDOWN )
        {
          if ( pGadget->m_pCamera )
          {
            pGadget->PtGreyCameraStop() ;
            pGadget->PtGreyDeleteCam( pGadget->m_pCamera ) ;
            pGadget->m_pCamera = NULL ;
          }
          pGadget->m_BusEvents = 0 ;
          pGadget->m_continueGrabThread = false ;
          TRACE( "Camera #%u Shut downed\n" , pGadget->m_dwSerialNumber ) ;
          Sleep( 20 ) ;
          break ;
        }
        if ( pGadget->m_BusEvents & PGR_EVT_RESTART )
        {
          pGadget->m_BusEvents &= ~( PGR_EVT_RESTART ) ;
          pGadget->PtGreyCamClose() ;
          TRACE( "Camera #%u closed in ControlLoop\n" , pGadget->m_dwSerialNumber ) ;
          pGadget->m_BusEvents |= PGR_EVT_INIT ;
          break ;
        }
        if ( pGadget->m_BusEvents & ( BUS_EVT_REMOVED | BUS_EVT_BUS_RESET ) )
        {
          if ( pGadget->m_BusEvents & BUS_EVT_BUS_RESET )
          {
            pGadget->m_BusEvents |= PGR_EVT_INIT ;
            Sleep( 10 ) ;
            TRACE( "Camera #%u Bus Reset\n" , pGadget->m_dwSerialNumber ) ;
          }
          if ( pGadget->m_BusEvents & BUS_EVT_REMOVED )
          {
            pGadget->PtGreyCamClose() ;
            TRACE( "Camera #%u Removed\n" , pGadget->m_dwSerialNumber ) ;
          }

          pGadget->m_BusEvents &= ~( ( BUS_EVT_REMOVED | BUS_EVT_BUS_RESET ) ) ;
        }
        if ( ( pGadget->m_BusEvents & PGR_EVT_START_GRAB )
          && !pGadget->m_bInitialized )
        {
          pGadget->m_BusEvents |= PGR_EVT_INIT ;
        }
        if ( pGadget->m_BusEvents & PGR_EVT_INIT )
        {
          if ( pGadget->m_dwSerialNumber )
          {
            pGadget->PtGreyCamInit() ;
            if ( pGadget->m_bInitialized  &&  pGadget->m_bRun )
            {
              pGadget->m_BusEvents |= PGR_EVT_START_GRAB ;
              Sleep( 50 ) ;
              pGadget->m_BusEvents &= ~( PGR_EVT_BUILD_PROP ) ;
            }
            iNInits++ ;
          }
          pGadget->m_BusEvents &= ~( PGR_EVT_INIT ) ;
        }
        else if ( pGadget->m_BusEvents &   PGR_EVT_BUILD_PROP )
        {
          pGadget->PtGreyBuildPropertyList() ;
          pGadget->m_BusEvents &= ~( PGR_EVT_BUILD_PROP ) ;
        }
        if ( pGadget->m_BusEvents & PGR_EVT_SET_PROP )
        {
          pGadget->PtGreySetCameraProperty( pGadget->m_iPropertyIndex , &pGadget->m_PropertyData ) ;
          pGadget->m_BusEvents &= ~( PGR_EVT_SET_PROP ) ;
        }
        if ( pGadget->m_BusEvents & PGR_EVT_GET_PROP )
        {
          pGadget->PtGreyGetCameraProperty( pGadget->m_iPropertyIndex , &pGadget->m_PropertyData ) ;
          pGadget->m_BusEvents &= ~( PGR_EVT_GET_PROP ) ;
        }

        if ( pGadget->m_BusEvents & PGR_EVT_START_GRAB )
        {
          if ( !pGadget->PtGreyCameraStart() )
          {
            if ( pGadget->m_dwSerialNumber  && pGadget->m_dwSerialNumber != ( -1 ) )
              pGadget->m_BusEvents = PGR_EVT_INIT | PGR_EVT_START_GRAB ;
          }
          pGadget->m_BusEvents &= ~( PGR_EVT_START_GRAB ) ;
        }
        if ( pGadget->m_BusEvents & PGR_EVT_STOP_GRAB )
        {
          pGadget->PtGreyCameraStop() ;
          pGadget->m_BusEvents &= ~( PGR_EVT_STOP_GRAB ) ;
        }
        if ( pGadget->m_BusEvents & PGR_EVT_RELEASE )
        {
          pGadget->PtGreyCamClose() ;
          pGadget->m_CurrentDevice = -1 ;
          pGadget->m_BusEvents &= ~( PGR_EVT_RELEASE ) ;
          break ;
        }
        if ( pGadget->m_BusEvents & PGR_EVT_LOCAL_STOP )
        {
          pGadget->PtGreyLocalStreamStop() ;
          pGadget->m_BusEvents &= ~( PGR_EVT_LOCAL_STOP ) ;
          break ;
        }
        if ( pGadget->m_BusEvents & PGR_EVT_LOCAL_START )
        {
          pGadget->PtGreyLocalStreamStart() ;
          pGadget->m_BusEvents &= ~( PGR_EVT_LOCAL_START ) ;
          break ;
        }
        if ( pGadget->m_BusEvents & BUS_EVT_ARRIVED )
        {
          pGadget->PtGreyCamInit() ;
          if ( pGadget->m_bInitialized  &&  pGadget->m_bRun )
          {
            pGadget->m_BusEvents |= PGR_EVT_START_GRAB ;
            Sleep( 50 ) ;
            pGadget->m_BusEvents &= ~( PGR_EVT_BUILD_PROP ) ;
            FxSendLogMsg( MSG_INFO_LEVEL ,
              pGadget->GetDriverInfo() , 0 ,
              " Camera #%u is initialized after connection\n" ,
              pGadget->m_dwSerialNumber ) ;
          }
          else
            FxSendLogMsg( MSG_ERROR_LEVEL ,
            pGadget->GetDriverInfo() , 0 ,
            " Camera #%u is NOT initialized after connection\n" ,
            pGadget->m_dwSerialNumber ) ;


          pGadget->m_BusEvents &= ~( BUS_EVT_ARRIVED ) ;
        }
      }
      if ( InitialBusEvents )
        SetEvent( pGadget->m_evControlRequestFinished ) ;
    }

    if ( pGadget->m_bRun  && pGadget->m_continueGrabThread )
    {
      //       int iRes = pGadget->ReceiveImage(); // will read new image and put pointer to pGadget->m_pNewImage
      //       switch ( iRes ) // no camera
      //       {
      //       case -1 :  // no camera
      //         pGadget->m_BusEvents |= PGR_EVT_INIT ;
      //       case PGRERROR_ISOCH_NOT_STARTED:
      //         pGadget->m_BusEvents |= PGR_EVT_START_GRAB ;
      //         SetEvent( pGadget->m_evBusChange ) ;
      //         break ;
      //       }
    }
  }
  pGadget->m_continueGrabThread = false ;
  pGadget->UnregisterCallbacks() ;

  if ( pGadget->m_pCamera )
    pGadget->PtGreyCamClose() ;
  pGadget->m_ControlThreadName.Empty() ;
  pGadget->m_dwGrabThreadId = NULL ;
  TRACE( "---PGR Normal Exit from ControlLoop for #%u\n" , pGadget->m_dwSerialNumber ) ;
  return 0;
}

void PtGrey2_0::LogError( LPCTSTR Msg )
{
  FXString GadgetName ;
  GetGadgetName( GadgetName ) ;
  FxSendLogMsg( MSG_ERROR_LEVEL , GetDriverInfo() , 0 ,
    _T( "%s - %s" ) , ( LPCTSTR )GadgetName , Msg );
}

void PtGrey2_0::LocalStreamStart()
{
  CamCNTLDoAndWait( PGR_EVT_START_GRAB ) ;
  m_bLocalStopped = false ;
}
void PtGrey2_0::LocalStreamStop()
{
  CamCNTLDoAndWait( PGR_EVT_STOP_GRAB ) ;
  m_bLocalStopped = true ;
}


void
PtGrey2_0::RegisterCallbacks()
{
  Error error;

  // Register arrival callback
  error = m_BusManager.RegisterCallback( &PtGrey2_0::OnBusArrival , ARRIVAL , this , &m_cbArrivalHandle );
  if ( error != PGRERROR_OK )
  {
    DEVICESENDERR_1( "Failed to register bus arrival callback: %s" ,
      error.GetDescription() );
    m_cbArrivalHandle = NULL ;
  }

  // Register removal callback
  error = m_BusManager.RegisterCallback( &PtGrey2_0::OnBusRemoval , REMOVAL , this , &m_cbRemovalHandle );
  if ( error != PGRERROR_OK )
  {
    DEVICESENDERR_1( "Failed to register bus removal callback: %s" ,
      error.GetDescription() );
    m_cbRemovalHandle = NULL ;
  }

  // Register reset callback
  error = m_BusManager.RegisterCallback( &PtGrey2_0::OnBusReset , BUS_RESET , this , &m_cbResetHandle );
  if ( error != PGRERROR_OK )
  {
    DEVICESENDERR_1( "Failed to register bus reset callback: %s" ,
      error.GetDescription() );
    m_cbResetHandle = NULL ;
  }
}

void PtGrey2_0::UnregisterCallbacks()
{
  Error error;

  //   if ( m_cbArrivalHandle )
  //   {
  //     // Unregister arrival callback
  //     error = m_BusManager.UnregisterCallback( m_cbArrivalHandle );
  //     if ( error != PGRERROR_OK )
  //     {
  //       //ShowErrorMessageDialog( "Failed to unregister callback", error );     
  //     }
  //     m_cbArrivalHandle = NULL ;
  //   }

  if ( m_cbRemovalHandle )
  {
    // Unregister removal callback
    error = m_BusManager.UnregisterCallback( m_cbRemovalHandle );
    if ( error != PGRERROR_OK )
    {
      //ShowErrorMessageDialog( "Failed to unregister callback", error );     
    }

    m_cbRemovalHandle = NULL ;
  }
  if ( m_cbResetHandle )
  {
    // Unregister reset callback
    error = m_BusManager.UnregisterCallback( m_cbResetHandle );
    if ( error != PGRERROR_OK )
    {
      //ShowErrorMessageDialog( "Failed to unregister callback", error );     
    }
    m_cbResetHandle = NULL ;
  }
}

bool PtGrey2_0::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  double dStart = GetHRTickCount() ;
  FXAutolock al( m_ScanPropLock ) ;
  m_bInScanProperties = true ;
  m_bShouldBeReprogrammed = false ;

  if ( !m_continueGrabThread )
  {
    FXString ThreadName ;
    m_hGrabThreadHandle = CreateThread( NULL , 0 ,
      ControlLoop , this , CREATE_SUSPENDED , &m_dwGrabThreadId ) ;
    if ( m_hGrabThreadHandle )
    {
      ResumeThread( m_hGrabThreadHandle ) ;
      Sleep( 50 ) ;
      m_continueGrabThread = true ;
    }
    else
    {
      DEVICESENDERR_2( "%s: %s" , ( LPCTSTR )ThreadName , _T( "Can't start thread" ) );
      m_bInScanProperties = false ;
      return false ;
    }
  }

  FXString tmpS;
  FXPropertyKit pc( text );
  unsigned camSN = 0 ;
  m_bWasStopped = false ;
  if ( pc.GetInt( "Camera" , ( int& )camSN ) )
  {
    if ( camSN && ( camSN != 0xffffffff ) )
    {
      int newCamnmb = SerialToNmb( camSN );;
      if ( newCamnmb < m_iCamNum )
      {
        if ( ( m_dwSerialNumber != camSN ) || ( newCamnmb != m_CurrentDevice ) )
        {
          m_bWasStopped = IsRunning() ;
          if ( m_dwSerialNumber && ( m_dwSerialNumber != 0xffffffff ) )
          {
            bool bRes = CamCNTLDoAndWait( PGR_EVT_RELEASE , 2000 ) ;
            m_dwSerialNumber = 0 ;
            m_dwConnectedSerialNumber = 0 ;
          }
          ASSERT( m_pCamera == NULL ) ;
          m_dwSerialNumber = camSN ;
          m_CurrentDevice = newCamnmb;
          if ( m_dwSerialNumber )
          {
            CameraInit();
            m_bShouldBeReprogrammed = m_bWasStopped ;
          }
        }
        else
        {
          bool bCamConnected = CheckAndAllocCamera() ;
          if ( !bCamConnected )
          {
            m_bInScanProperties = false ;
            return false ;
          }
        }
      }
      else
      {
        m_dwSerialNumber = camSN ;
        m_CurrentDevice = -1 ;
      }
      Invalidate |= true; //update setup
    }
    else
    {
      if ( m_dwSerialNumber && ( m_dwSerialNumber != 0xffffffff ) )
      {
        bool bRes = CamCNTLDoAndWait( PGR_EVT_RELEASE , 2000 ) ;
        m_dwSerialNumber = 0 ;
        m_dwConnectedSerialNumber = 0 ;
        m_dwInitializedSerialNumber = 0 ;
        Invalidate = true ;
      }
      m_dwSerialNumber = 0 ;
      m_pixelFormat = PIXEL_FORMAT_MONO8 ;
      m_bIsRunning = false ;
      //       ASSERT( bRes ) ;
    }
  }
  if ( DriverValid() && m_pCamera )
  {
    if ( pc.GetInt( "StreamState" , m_bLocalStopped )
      && IsRunning() )
    {
      if ( m_bLocalStopped )
        LocalStreamStop() ;
      else
        LocalStreamStart() ;
    }
    for ( int i = 0; i < m_ThisCamProperty.GetSize(); i++ )
    {
      m_PropertyListLock.Lock() ;
      PgrCamProperty Prop = m_ThisCamProperty[ i ] ;
      m_PropertyListLock.Unlock() ;
      FXString key;
      FXParser param;
      Prop.m_GUIFormat.GetElementNo( 0 , key , param );
      int value;
      bool bauto = false;
      FXString AsString ;
      if ( ( key == SETUP_COMBOBOX ) || ( key == SETUP_SPIN ) ) // ints result
      {
        switch ( Prop.m_DataType )
        {
          case PDTInt:
            if ( pc.GetInt( Prop.name , value ) )
            {
              FXSIZE xValue = value ;
              if ( !SetCameraProperty( Prop.id , xValue , bauto , Invalidate ) )
              {
                DEVICESENDERR_1( "Can't set property %s" , Prop.name );
              }
            }
            break ;
          case PDTFloat:
            if ( pc.GetString( Prop.name , AsString ) )
            {
              FXSIZE xValue = ( FXSIZE )( ( LPCTSTR )AsString ) ;
              if ( !SetCameraProperty( Prop.id , xValue , bauto , Invalidate ) )
              {
                DEVICESENDERR_1( "Can't set property %s" , Prop.name );
              }
            }
            break ;

        }
      }
      else if ( key == SETUP_SPINABOOL )
      {
        if ( pc.GetString( Prop.name , AsString ) )
        {
          AsString = AsString.MakeLower() ;
          bauto = ( AsString.Find( "auto" ) == 0 ) ;
          FXSIZE xValue ;
          if ( bauto )
          {
            GetCameraProperty( Prop.id , xValue , bauto );
            bauto = true;
            //value = (FXSIZE)_T("Unknown") ;
          }
          else
          {
            switch ( Prop.m_DataType )
            {
              case PDTInt: xValue = atoi( (LPCTSTR)AsString ); break;
              case PDTFloat:
              case PDTString: xValue = ( FXSIZE )( ( LPCTSTR )AsString ) ; break ;
            }

          }
          if ( !SetCameraProperty( Prop.id , xValue , bauto , Invalidate ) )
          {
            DEVICESENDERR_1( "Can't set property %s" , Prop.name );
          }
        }
      }
      else if ( key == SETUP_EDITBOX )
      {
        FXString svalue;
        FXSIZE value; bool bauto = false;
        if ( pc.GetString( Prop.name , svalue ) )
        {
          value = ( FXSIZE )( ( LPCTSTR )svalue );
          if ( !SetCameraProperty( Prop.id , value , bauto , Invalidate ) )
          {
            DEVICESENDERR_2( "Can't set prop %s to %s" ,
              Prop.name , svalue );
          }
        }
      }
      else
      {
        DEVICESENDERR_1( "Unsupported key '%s'in scanproperty" , key );
      }
    }
  }
  if ( m_bShouldBeReprogrammed )
  {
    if ( m_bWasStopped )
      DeviceStart() ;
    //     else
    //       CamCNTLDoAndWait( PGR_EVT_INIT , 2000 ) ;
  }
  m_bInScanProperties = false ;
  FXString GadgetName ;
  GetGadgetName( GadgetName ) ;
  double dBusyTime = GetHRTickCount() - dStart ;
  TRACE( "PtGrey2_0::ScanProperties %s: Start %g , Busy %g\n" , ( LPCTSTR )GadgetName ,
    dStart , dBusyTime ) ;
  return true;
}

void PtGrey2_0::OnBusArrival( void* pParam , unsigned int serialNumber )
{
  PtGrey2_0* pGadget = static_cast< PtGrey2_0* >( pParam );
  if ( pGadget->m_dwSerialNumber == serialNumber )
  {
    bool bRes = pGadget->CamCNTLDoAndWait( BUS_EVT_ARRIVED , 2000 ) ;
    FxSendLogMsg( MSG_WARNING_LEVEL ,
      pGadget->GetDriverInfo() , 0 ,
      " Bus arrival of Camera #%u; Init=%s\n" ,
      serialNumber , ( bRes ) ? _T( "OK" ) : _T( "FAULT" ) ) ;
  }
  //   else
  //     FxSendLogMsg(MSG_WARNING_LEVEL , 
  //     pGadget->GetDriverInfo() , 0 , 
  //     "  Bus arrival of Camera #%u (Gadget cam SN is %u)\n" , serialNumber , pGadget->m_dwSerialNumber ) ;

  TRACE( "Bus Arrival for Camera #%u\n " , pGadget->m_dwSerialNumber ) ;
  pGadget->m_bRescanCameras = true ;
  pGadget->m_dwNArrivedEvents++ ;
}

void PtGrey2_0::OnBusRemoval( void* pParam , unsigned int serialNumber )
{
  PtGrey2_0* pGadget = static_cast< PtGrey2_0* >( pParam );
  if ( serialNumber == pGadget->m_DevicesInfo[ pGadget->m_CurrentDevice ].serialnmb )
  {
    FxSendLogMsg( MSG_WARNING_LEVEL ,
      pGadget->GetDriverInfo() , 0 , "Camera %u disconnected" , serialNumber ) ;
    TRACE( "Camera #%u Removed\n" , pGadget->m_dwSerialNumber ) ;
    pGadget->CamCNTLDoAndWait( BUS_EVT_REMOVED , 2000 ) ;
  }
  //  SENDINFO_2( "  Bus removal of Camera #%u (Gadget cam SN is %u)\n" , serialNumber , pGadget->m_dwSerialNumber /*, pGadget->m_Bus*/) ;

  TRACE( "  Bus removal of Camera #%u (Gadget cam SN is %u)\n" , serialNumber , pGadget->m_dwSerialNumber /*, pGadget->m_Bus*/ ) ;
  pGadget->m_bCamerasEnumerated = false ;
  pGadget->m_bRescanCameras = true ;
}


void PtGrey2_0::OnBusReset( void* pParam , unsigned int serialNumber )
{
  PtGrey2_0* pGadget = static_cast< PtGrey2_0* >( pParam );
  pGadget->m_bCamerasEnumerated = false ;
  //   bool bRes = pGadget->CamCNTLDoAndWait( BUS_EVT_BUS_RESET | PGR_EVT_INIT , 2000)  ;
  pGadget->m_bRescanCameras = true ;
  //  SENDINFO_1( "  Bus Reset for Camera #%u \n" , pGadget->m_dwSerialNumber /*, pGadget->m_Bus*/) ;
  TRACE( "  Bus Reset for Camera #%u \n" , pGadget->m_dwSerialNumber /*, pGadget->m_Bus*/ ) ;
  //   ASSERT( bRes ) ;
}



bool PtGrey2_0::CheckAndAllocCamera( void )
{
  if ( !m_pCamera )
  {
    if ( !m_dwSerialNumber )
      return false ;
    if ( !CameraInit() )
      return false ;
  }
  //   if ( !m_pCamera  ||  m_bGUIReconnect )
  //   {
  //     FXAutolock al( m_LocalConfigLock , "CheckAndAllocCamera") ;
  // 
  //     if ( m_pCamera )
  //     {
  //       m_pCamera->Disconnect() ;
  //       m_pCamera = NULL ;
  //     }
  //     m_LastError = m_BusManager.GetCameraFromSerialNumber(
  //       m_dwSerialNumber , &m_CurrentCameraGUID);
  //     if (m_LastError!=PGRERROR_OK)
  //     {
  //       if ( ++m_iNNoSerNumberErrors <= 2 )
  //         DEVICESENDERR_1("Fatal error GetCameraFromSerialNumber: %s",
  //         m_LastError.GetDescription( ));
  //       return false;
  //     }
  //     m_LastError=m_BusManager.GetInterfaceTypeFromGuid( &m_CurrentCameraGUID, &m_IntfType );
  //     if ( m_LastError != PGRERROR_OK )
  //     {   
  //       DEVICESENDERR_1("Fatal error GetInterfaceTypeFromGuid: %s",
  //         m_LastError.GetDescription( ));
  //       return FALSE;
  //     }    
  // 
  //     m_Status.Empty() ;
  // 
  //     if ( m_IntfType == INTERFACE_GIGE )
  //       m_pCamera = new GigECamera ;   // GIGE interface
  //     else
  //       m_pCamera = new Camera ; // USB or 1394 interface
  // 
  //     m_bGUIReconnect = false ;
  //     if ( m_pCamera )
  //     {
  //       m_LastError=m_pCamera->Connect(&m_CurrentCameraGUID);
  //       if (m_LastError!=PGRERROR_OK)
  //       {
  //         delete m_pCamera ;
  //         m_pCamera = NULL ;
  //         DEVICESENDERR_1("GUI Connect error: %s",m_LastError.GetDescription( ));
  //         return false;
  //       }
  //     }
  //   }
  return true ;
}


bool PtGrey2_0::SetBMIH( void )
{
  m_BMIH.biSize = sizeof( BITMAPINFOHEADER );
  m_BMIH.biWidth = m_CurrentROI.Width() ;
  m_BMIH.biHeight = m_CurrentROI.Height();

  m_BMIH.biPlanes = 1;
  switch ( m_pixelFormat )
  {
    case PIXEL_FORMAT_RAW8:
    case PIXEL_FORMAT_MONO8:
      m_BMIH.biCompression = BI_Y8;
      m_BMIH.biBitCount = 8;
      m_BMIH.biSizeImage = m_BMIH.biWidth*m_BMIH.biHeight;
      break;
    case PIXEL_FORMAT_411YUV8:
      m_BMIH.biCompression = BI_YUV411 ;
      m_BMIH.biBitCount = 12;
      m_BMIH.biSizeImage = 3 * m_BMIH.biWidth*m_BMIH.biHeight / 2;
      break;
    case PIXEL_FORMAT_MONO16:
      m_BMIH.biCompression = BI_Y16;
      m_BMIH.biBitCount = 16;
      m_BMIH.biSizeImage = 2 * m_BMIH.biWidth*m_BMIH.biHeight;
      break;
      //         PIXEL_FORMAT_422YUV8      /**< YUV 4:2:2. */
      //         PIXEL_FORMAT_444YUV8      /**< YUV 4:4:4. */
      //         PIXEL_FORMAT_RGB8         /**< R = G = B = 8 bits. */
      //         PIXEL_FORMAT_RAW16        /**< 16 bit raw data output of sensor. */
      //         PIXEL_FORMAT_MONO12       /**< 12 bits of mono information. */
      //         PIXEL_FORMAT_RAW12        /**< 12 bit raw data output of sensor. */
      //         PIXEL_FORMAT_BGR          /**< 24 bit BGR. */
      //         PIXEL_FORMAT_BGRU         /**< 32 bit BGRU. */
      //         PIXEL_FORMAT_RGB          _RGB8, /**< 24 bit RGB. */
      //         PIXEL_FORMAT_RGBU         /**< 32 bit RGBU. */
      //         PIXEL_FORMAT_BGR16        /**< R = G = B = 16 bits. */
      //         PIXEL_FORMAT_BGRU16       /**< 64 bit BGRU. */
      //         PIXEL_FORMAT_422YUV8_JPEG /**< JPEG compressed stream. */


    default:
      m_BMIH.biSize = 0;
      TRACE( "!!! Unsupported format #%d\n" , m_pixelFormat );
      DEVICESENDERR_1( "!!! Unsupported format #%d" , m_pixelFormat );
      return false;
  }
  return true ;
}


bool PtGrey2_0::ScanSettings( FXString& text )
{
  EnumCameras() ;
  bool bAllocated = CheckAndAllocCamera() ;
  // Prepare cameras list
  FXString camlist( "Not Selected(-1)," ) , paramlist , tmpS;
  int iCurrentCamera = -1 ;
  for ( int i = 0; i < m_iCamNum; i++ )
  {
    TCHAR cMark = _T( '+' ) ; // sign, that camera is free
    if ( m_dwSerialNumber != m_DevicesInfo[ i ].serialnmb )
    {
      for ( int j = 0 ; j < m_BusyCameras.GetCount() ; j++ )
      {
        if ( m_dwSerialNumber != m_BusyCameras[ j ] )
        {
          if ( m_DevicesInfo[ i ].serialnmb == m_BusyCameras[ j ] )
          {
            cMark = _T( '-' ) ; // sign, that camera is busy by other gadget
            break ;
          }
        }
      }
    }
    else
    {
      cMark = _T( '!' ) ;// this gadget camera sign
      iCurrentCamera = m_CurrentDevice = i ;
    }
    tmpS.Format( "%c%d:%s(%d)" , cMark , m_DevicesInfo[ i ].serialnmb ,
      ( LPCTSTR )m_DevicesInfo[ i ].m_sGivenName , m_DevicesInfo[ i ].serialnmb );
    camlist += tmpS;
    if ( i < m_iCamNum - 1 )
      camlist += _T( ',' ) ;
  }
  if ( iCurrentCamera < 0 && m_dwSerialNumber && ( m_dwSerialNumber != 0xffffffff ) )
  {
    if ( !camlist.IsEmpty() )
      camlist += _T( ',' ) ;
    tmpS.Format( "?%d(%d)" , m_dwSerialNumber , m_dwSerialNumber ) ;
    camlist += tmpS ;
    iCurrentCamera = m_iCamNum ; // first after real cameras
  }
  tmpS.Format( "ComboBox(Camera(%s))" , camlist );
  paramlist += tmpS ;
  if ( m_iCamNum && bAllocated && m_pCamera )
  {
    m_dwConnectedSerialNumber = m_dwSerialNumber ;
    if ( m_dwSerialNumber
      && ( m_dwSerialNumber != 0xffffffff ) )
    {
      paramlist += _T( ",ComboBox(StreamState(Run(0),Idle(1)))," ) ;
      int cnt = 0 ;
      do
      {
        bool bRes = CamCNTLDoAndWait( PGR_EVT_BUILD_PROP , 5000 ) ;
        if ( !bRes )
          cnt = 5 ;
      }
      while ( cnt++ < 5 && ( m_ThisCamProperty.GetCount() == 0 ) ) ;

      if ( cnt < 5 )
      {
        FXAutolock al( m_PropertyListLock ) ;
        for ( int i = 0; i < m_ThisCamProperty.GetSize(); i++ )
        {
          paramlist += ( FXString )m_ThisCamProperty[ i ].m_GUIFormat ;
          if ( i < m_ThisCamProperty.GetUpperBound() )
            paramlist += _T( ',' ) ;
        }
      }
      else
      {
        DEVICESENDERR_0( "Can't build property list" ) ;
        m_BusEvents = 0 ;
      }
    }
  }
  text.Format( "template(%s)" , paramlist );
  return true;
}

bool PtGrey2_0::PrintProperties( FXString& text )
{
  FXPropertyKit pc;
  if ( DriverValid() && ( m_dwSerialNumber != 0 )
    && ( m_dwSerialNumber != -1 ) )
  {
    pc.WriteInt( "Camera" , m_dwSerialNumber );
    pc.WriteInt( "StreamState" , m_bLocalStopped ) ;
    if ( m_dwConnectedSerialNumber != 0 )
    {
      for ( int i = 0; i < m_ThisCamProperty.GetSize(); i++ )
      {
        FXSIZE value;
        bool bauto;
        m_PropertyListLock.Lock() ;
        PgrCamProperty Prop = m_ThisCamProperty[ i ] ;
        m_PropertyListLock.Unlock() ;
        if ( GetCameraProperty( Prop.id , value , bauto ) )
        {
          FXString key;
          FXParser param;
          Prop.m_GUIFormat.GetElementNo( 0 , key , param );
          if ( ( key == SETUP_COMBOBOX ) || ( key == SETUP_SPIN ) ) // ints result
          {
            FXString tmpS;
            switch ( Prop.m_DataType )
            {
              case PDTInt: tmpS.Format( "%d" , value ); break ;
              case PDTString:
              case PDTFloat: tmpS = ( LPCTSTR )value ; break ;
              case PDTBool: tmpS = ( value != 0 ) ? '1' : '0' ; break ;
            }
            pc.WriteString( Prop.name , tmpS );
          }
          else if ( key == SETUP_SPINABOOL )
          {
            FXString tmpS;
            switch ( Prop.m_DataType )
            {
              case PDTInt: tmpS.Format( "%d" , (int)value ); break ;
              case PDTString:
              case PDTFloat: tmpS = ( LPCTSTR )value ; break ;
              case PDTBool: tmpS = ( value != 0 ) ? '1' : '0' ; break ;
            }
            if ( bauto )
              tmpS.Insert( 0 , _T( "auto" ) ) ;

/*            if ( bauto )
              tmpS = _T( "auto" ) ;
            else
            {
              switch ( Prop.m_DataType )
              {
                case PDTInt: tmpS.Format( "%d" , value ); break ;
                case PDTString:
                case PDTFloat: tmpS = ( LPCTSTR )value ; break ;
                case PDTBool: tmpS = ( value != 0 ) ? '1' : '0' ; break ;
              }

            }*/
            pc.WriteString( Prop.name , tmpS );
          }
          else if ( key == SETUP_EDITBOX )
          {
            FXString svalue = ( LPCTSTR )value;
            pc.WriteString( Prop.name , svalue );
          }
          else
          {
            DEVICESENDERR_1( "Unsupported key '%s' PrintProperty" , key );
          }
        }
      }
    }
  }
  else
  {
    pc.WriteInt( "Camera" , -1 );
  }
  text = pc;
  //TRACE( "\nPtGrey2_0::PrintProperties - %s\n" , ( LPCTSTR )text ) ;
  return true;
}


int PtGrey2_0::SetPacketSize( int iPacketSize_Or_FPSx10 , bool bFrameRate )
{
  if ( !m_pCamera )
    return 0 ;

  if ( m_IntfType != INTERFACE_GIGE )
  {
    Format7ImageSettings f7is;
    f7is.mode = m_Mode;
    f7is.pixelFormat = m_pixelFormat;
    f7is.offsetX = m_CurrentROI.left;
    f7is.offsetY = m_CurrentROI.top;
    f7is.width = m_CurrentROI.Width();
    f7is.height = m_CurrentROI.Height();
    Format7PacketInfo f7pi;
    bool valid;
    m_LastError = ( ( Camera* )m_pCamera )->ValidateFormat7Settings( &f7is , &valid , &f7pi );
    if ( m_LastError != PGRERROR_OK )
    {
      if ( m_pixelFormat == PIXEL_FORMAT_MONO8 )
      {
        f7is.pixelFormat = PIXEL_FORMAT_RAW8 ;
        m_LastError = ( ( Camera* )m_pCamera )->ValidateFormat7Settings( &f7is , &valid , &f7pi );
        if ( m_LastError != PGRERROR_OK )
        {
          PtGreyCamClose() ;
          DEVICESENDERR_1( "ValidateFormat7Settings error: %s" , m_LastError.GetDescription() );
          return 0;
        }
        else
          m_pixelFormat = PIXEL_FORMAT_RAW8 ;
      }
    }
    if ( !bFrameRate )
      m_uiPacketSize = iPacketSize_Or_FPSx10 ;
    else
    {
      // there are 8000 packets per second which are transfers FPS * ImageSize bytes
      double dMult = 1. ;// MONO8 and RAW8
      switch ( m_pixelFormat )
      {
        case PIXEL_FORMAT_RAW12:
        case PIXEL_FORMAT_MONO12:
        case PIXEL_FORMAT_411YUV8: dMult = 1.5 ; break ;
        case PIXEL_FORMAT_S_MONO16:
        case PIXEL_FORMAT_MONO16:
        case PIXEL_FORMAT_RAW16:
        case PIXEL_FORMAT_422YUV8: dMult = 2. ; break ;
        case PIXEL_FORMAT_BGR:
        case PIXEL_FORMAT_444YUV8:
        case PIXEL_FORMAT_RGB8: dMult = 3. ; break ;
        case PIXEL_FORMAT_BGR16:
        case PIXEL_FORMAT_S_RGB16:
        case PIXEL_FORMAT_RGB16: dMult = 6. ; break ;
        case PIXEL_FORMAT_BGRU:
        case PIXEL_FORMAT_RGBU: dMult = 6. ; break ;
      }
      double dBandwidth_BytesPerSecond = ( double )iPacketSize_Or_FPSx10 * 0.1
        * f7is.width * f7is.height * dMult ;
      if ( !m_bMaxPacketSize )
      {
      m_uiPacketSize = ( m_IntfType == INTERFACE_IEEE1394 ) ?
        ROUND( dBandwidth_BytesPerSecond / 8000. ) : f7pi.recommendedBytesPerPacket ;
    }
      else
        m_uiPacketSize = f7pi.maxBytesPerPacket ;
    }
    if ( m_uiPacketSize < f7pi.unitBytesPerPacket )
      m_uiPacketSize = f7pi.unitBytesPerPacket ;
    if ( m_uiPacketSize > f7pi.maxBytesPerPacket )
      m_uiPacketSize = f7pi.maxBytesPerPacket ;
    else
    {
      // packet size should be multiple of 
      int iRest = m_uiPacketSize % f7pi.unitBytesPerPacket ;
      m_uiPacketSize = f7pi.unitBytesPerPacket * ( m_uiPacketSize / f7pi.unitBytesPerPacket ) ;
      if ( iRest )
        m_uiPacketSize += f7pi.unitBytesPerPacket ;
    }
    m_f7is = f7is ;
    m_f7pi = f7pi ;
    return m_uiPacketSize ;
  }
  else
  {

  }
  return -1 ;
}

int PtGrey2_0::SetMaxPacketSize()
{
  if ( !m_pCamera )
    return 0;
  if ( m_IntfType != INTERFACE_GIGE )
  {
    Format7ImageSettings f7is;
    f7is.mode = m_Mode;
    f7is.pixelFormat = m_pixelFormat;
    f7is.offsetX = m_CurrentROI.left;
    f7is.offsetY = m_CurrentROI.top;
    f7is.width = m_CurrentROI.Width();
    f7is.height = m_CurrentROI.Height();
    Format7PacketInfo f7pi;
    bool valid;
    m_LastError = ( (Camera*) m_pCamera )->ValidateFormat7Settings( 
      &f7is , &valid , &f7pi );
    if ( m_LastError != PGRERROR_OK )
    {
      if ( m_pixelFormat == PIXEL_FORMAT_MONO8 )
      {
        f7is.pixelFormat = PIXEL_FORMAT_RAW8;
        m_LastError = ( (Camera*) m_pCamera )->ValidateFormat7Settings(
          &f7is , &valid , &f7pi );
        if ( m_LastError != PGRERROR_OK )
        {
          PtGreyCamClose();
          DEVICESENDERR_1( "SetMaxPacketSize : ValidateFormat7Settings error: %s" , m_LastError.GetDescription() );
          return 0;
        }
        else
          m_pixelFormat = PIXEL_FORMAT_RAW8;
}
    }
    m_uiPacketSize = f7pi.maxBytesPerPacket ;
    m_LastError = ( (Camera*) m_pCamera )->SetFormat7Configuration( 
      &m_f7is , m_uiPacketSize );
    if ( m_LastError != PGRERROR_OK )
    {
      PtGreyCamClose();
      DEVICESENDERR_1( "SetMaxPacketSize : Set packet size error: %s" , m_LastError.GetDescription() );
      return 0;
    }
    m_bMaxPacketSize = TRUE ;
    return m_uiPacketSize ;
  }
  return 0;
}

int PtGrey2_0::GetMaxPacketSize()
{
  if ( !m_pCamera )
    return 0;

  if ( m_IntfType != INTERFACE_GIGE )
  {
    Format7ImageSettings f7is;
    f7is.mode = m_Mode;
    f7is.pixelFormat = m_pixelFormat;
    f7is.offsetX = m_CurrentROI.left;
    f7is.offsetY = m_CurrentROI.top;
    f7is.width = m_CurrentROI.Width();
    f7is.height = m_CurrentROI.Height();
    Format7PacketInfo f7pi;
    bool valid;
    m_LastError = ( (Camera*) m_pCamera )->ValidateFormat7Settings( &f7is , &valid , &f7pi );
    if ( m_LastError != PGRERROR_OK )
    {
      if ( m_pixelFormat == PIXEL_FORMAT_MONO8 )
      {
        f7is.pixelFormat = PIXEL_FORMAT_RAW8;
        m_LastError = ( (Camera*) m_pCamera )->ValidateFormat7Settings( &f7is , &valid , &f7pi );
        if ( m_LastError != PGRERROR_OK )
        {
          PtGreyCamClose();
          DEVICESENDERR_1( "ValidateFormat7Settings error: %s" , m_LastError.GetDescription() );
          return 0;
        }
        else
          m_pixelFormat = PIXEL_FORMAT_RAW8;
      }
    }
    return f7pi.maxBytesPerPacket;
  }
  else
  {
  }
  return 0;
}
bool PtGrey2_0::SetFrameRate( double dFrameRate )
{
  UINT CSR_Offset; 
  if ( _tcsstr( m_CamInfo->m_sGivenName , _T("FFMV") ) )
  {  // this code is for FireFy MV cameras
    // Read ABS_CSR_HI_INQ_15 (offset of abs frame rate register)

    Error Err = m_pCamera->ReadRegister( 0x73C, &CSR_Offset );
    if (Err == PGRERROR_OK)
    {        // convert to offset in bytes (was in registers)
      CSR_Offset = (CSR_Offset * 4) & 0xfffff;
      float fMinFPS;
      Err = m_pCamera->ReadRegister( CSR_Offset, (UINT*)&fMinFPS );
      if (Err == PGRERROR_OK)
      {
        float fMaxFPS = 0.0f;
        float fOldFPS = 0.0f;
        Err = m_pCamera->ReadRegister( CSR_Offset + 4, (UINT*)&fMaxFPS );
        Err = m_pCamera->ReadRegister( CSR_Offset + 8, (UINT*)&fOldFPS );
        float fWriteValue = (float)dFrameRate;
        if (fWriteValue < fMinFPS)
          fWriteValue = fMinFPS;
        else if (fWriteValue > fMaxFPS)
          fWriteValue = fMaxFPS;
        Err = m_pCamera->WriteRegister( CSR_Offset + 8, *((UINT*)&fWriteValue) );
        float fReadValue;
        Err = m_pCamera->ReadRegister( CSR_Offset + 8, (UINT*)&fReadValue );
        if (Err == PGRERROR_OK)
        {
          m_fLastRealFrameRate = fReadValue;
          return true;
        }
        return (Err == PGRERROR_OK);
      }
    }
  }
  else
  { // for other cameras
    if ((int)m_TriggerMode < (int)TriggerOn)
    {
      m_iFPSx10 = ROUND(dFrameRate * 10.) ;
      int iOldPacketSize = m_uiPacketSize;
      int iNewPacketSize = SetPacketSize( m_iFPSx10, true ); // adjust packet size
      bool bPacketChanged = (iOldPacketSize != iNewPacketSize);
      if (bPacketChanged)
      {
        if (m_IntfType != INTERFACE_GIGE)
        {
          ASSERT( iNewPacketSize > 0 );
          PtGreyCheckAndStopCapture();
          if (m_bInScanProperties && m_bWasStopped)
            m_bShouldBeReprogrammed = true;
          Format7Info f7i;
          bool bSupported = false;
          ((FlyCapture2::Camera*)m_pCamera)->GetFormat7Info( &f7i, &bSupported );
          Format7ImageSettings f7is;
          UINT uiCurrentPacketSize;
          float fPercentage;
          ((FlyCapture2::Camera*)m_pCamera)->GetFormat7Configuration(
            &f7is, &uiCurrentPacketSize, &fPercentage );
          m_LastError = ((FlyCapture2::Camera*)m_pCamera)->
            SetFormat7Configuration( &f7is, (unsigned)iNewPacketSize );
          if (m_LastError == PGRERROR_OK)
          {
            m_uiPacketSize = iNewPacketSize;
            m_LastError = ((FlyCapture2::Camera*)m_pCamera)->
              GetFormat7Configuration( &m_f7is, &m_uiPacketSize, &m_fPercentage );
            TRACE( " Cam #%u Packet=%d Bus Percentage=%g\n", m_dwSerialNumber,
              m_uiPacketSize, (double)m_fPercentage );
          }
          else
            DEVICESENDERR_1( "Set Format 7 for FPS error: %s", m_LastError.GetDescription() );
        }
        else
        {
        }
      }
      PropertyInfo  prpI;
      memset( &prpI, 0, sizeof( prpI ) );
      prpI.type = FRAME_RATE;
      m_LastError = m_pCamera->GetPropertyInfo( &prpI );
      double dFPS = dFrameRate ;
      if (dFPS < prpI.absMin)
        dFPS = prpI.absMin;
      if (dFPS > prpI.absMax)
        dFPS = prpI.absMax;
      FlyCapture2::Property prp;
      prp.type = FRAME_RATE;
      m_LastError = m_pCamera->GetProperty( &prp );
      prp.absControl = true;
      prp.absValue = (float)dFPS;
      prp.onOff = true;
      prp.autoManualMode = false;
      m_LastError = m_pCamera->SetProperty( &prp );
      if (m_LastError == PGRERROR_OK)
      {
        bool bRes = true;
        if (bPacketChanged)
        {
          if (m_IntfType != INTERFACE_GIGE)
          {
            if (!m_bInScanProperties && m_continueGrabThread)
              bRes = PtGreyInitAndConditialStart();
          }
          else
          {

          }
        }
        m_pCamera->GetProperty( &prp );
        TRACE( "+++ Set Property '%s' to %f (ordered %f), read %f\n",
          _T("FrameRate"), dFPS * 10., dFrameRate * 10.
          , (double)prp.absValue * 10. );
        return bRes;
      }
      else
      {
        TRACE( "+++ PgrSetProp '%s' Error %s\n", _T( "FrameRate" ), m_LastError.GetDescription() );
        DEVICESENDERR_2( " PgrSetProp %s error %s", _T( "FrameRate" ), m_LastError.GetDescription() );
      }
    }

  }
  
  return false;
}

bool PtGrey2_0::GetFrameRate( double& dFrameRate )
{
  UINT CSR_Offset;
  // Read ABS_CSR_HI_INQ_15 (offset of abs frame rate register)
  Error Err = m_pCamera->ReadRegister( 0x73C , &CSR_Offset );
  if ( Err == PGRERROR_OK )
  {        // convert to offset in bytes (was in registers)
    CSR_Offset = ( CSR_Offset * 4 ) & 0xfffff;
    float fReadValue;
    Err = m_pCamera->ReadRegister( CSR_Offset + 8 , (UINT*) &fReadValue );
    if ( Err == PGRERROR_OK )
    {
      dFrameRate = m_fLastRealFrameRate = fReadValue;
      return true ;
    }
  }
  return false;
}


