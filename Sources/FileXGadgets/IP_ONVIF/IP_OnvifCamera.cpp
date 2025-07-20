// IP_OnvifCam.cpp: implementation of the IP_OnvifCam class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <gadgets\gadbase.h>
#include <gadgets\stdsetup.h>
#include <gadgets\textframe.h>
#include <math\Intf_sup.h>
#include "IP_Onvif.h"
#include "IP_OnvifCamera.h"
#include <video\shvideo.h>


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#define THIS_MODULENAME "IP_OnvifCamera.cpp"



IMPLEMENT_RUNTIME_GADGET_EX( IP_OnvifCam , C1394Camera , "Video.capture" , TVDB400_PLUGIN_NAME );

typedef struct tagCamProperties
{
  PropertyType        pr;
  const char *        name;
} CamProperties;

typedef struct tagVideoformat
{
  PixelFormat vm;
  const char *        name;
}Videoformat;

int __stdcall RealDataCallBack( long lRealHandle , const PACKET_INFO_EX *pFrame , UINT_PTR dwUser )
{
  IP_OnvifCam* pThis = (IP_OnvifCam*) dwUser;
  pThis->CopyBuffer( (unsigned char*) pFrame->pPacketBuffer , pFrame->dwPacketSize );
  CVideoFrame * pOut = NULL ;
  pOut = pThis->DoH264VideoFrame( (unsigned char*) pFrame->pPacketBuffer , pFrame->dwPacketSize );
  pThis->GetOutputConnector( 0 )->Put( pOut );
  return 1;
}

FXLockObject                IP_OnvifCam::m_ConfigLock ;
SDK_CONFIG_NET_COMMON       IP_OnvifCam::m_CamSearchData[ MAX_CAMERASNMB ] ;
DEV_INFO					IP_OnvifCam::m_CamInfo[ MAX_CAMERASNMB ] ;
FXLockObject                IP_OnvifCam::m_IPCamArrLock ;
IP_OnvifCam*				IP_OnvifCam::m_pIPCamGadgets[ MAX_CAMERASNMB ] = {NULL};
unsigned int                IP_OnvifCam::m_iCamNum ;
unsigned int                IP_OnvifCam::m_iNCam = 0; ;
FXArray<DWORD>              IP_OnvifCam::m_BusyCameras ;
FXLockObject				IP_OnvifCam::m_GrabLock;
FXLockObject				IP_OnvifCam::m_ConnectionLock;


#define FORMAT_MODE ((PropertyType)(BRIGHTNESS-1))
#define SETROI ((PropertyType)(BRIGHTNESS-2))
#define SETSTROBE0 ((PropertyType)(BRIGHTNESS-3))
#define SETSTROBE1 ((PropertyType)(BRIGHTNESS-4))
#define SETSTROBE2 ((PropertyType)(BRIGHTNESS-5))
#define SETSTROBE3 ((PropertyType)(BRIGHTNESS-6))
#define PACKETSIZE ((PropertyType)(BRIGHTNESS-7))
#define WHITE_BAL_RED  ((PropertyType)(BRIGHTNESS-8))
#define WHITE_BAL_BLUE ((PropertyType)(BRIGHTNESS-9))

CamProperties cProperties[] =
{
  {SETROI, "ROI"},
  {FORMAT_MODE,"Format"},
  {BRIGHTNESS,"Brightness"},
  {AUTO_EXPOSURE,"Auto_exposure"},
  {SHARPNESS,"Sharpness"},
  {WHITE_BALANCE,"White_balance"},
  {HUE,"Hue"},
  {SATURATION,"Saturation"},
  {GAMMA,"Gamma"},
  {IRIS,"Iris"},
  {FOCUS,"Focus"},
  {ZOOM,"Zoom"},
  {PAN,"Pan"},
  {TILT,"Tilt"},
  {SHUTTER,"Shutter_us"},
  {GAIN,"Gain_dBx10"},
  {TRIGGER_MODE,"Trigger_mode"},
  {TRIGGER_DELAY,"Trigger_delay"},
  {FRAME_RATE,"Frame_rate_x10"},
  {PACKETSIZE,"PacketSize"},
  {SETSTROBE0,"Strobe0"},
  {SETSTROBE1,"Strobe1"},
  {SETSTROBE2,"Strobe2"},
  {SETSTROBE3,"Strobe3"},
  {WHITE_BAL_RED,"W.B.Red"},
  {WHITE_BAL_BLUE,"W.B.Blue"},
  {TEMPERATURE,"Temperature"}
};

__forceinline int _getPropertyCount()
{
  return sizeof( cProperties ) / sizeof( CamProperties );
}

__forceinline LPCTSTR _getPropertyname( int id )
{
  for ( int i = 0; i < _getPropertyCount(); i++ )
  {
    if ( cProperties[ i ].pr == id )
      return cProperties[ i ].name;
  }
  return "Unknown property";
}

Videoformat vFormats[] =
{
  {PIXEL_FORMAT_MONO8, "Mono8"},
  {PIXEL_FORMAT_MONO16, "Mono16"},
  {PIXEL_FORMAT_411YUV8, "YUV411"},
  //   {PIXEL_FORMAT_411YUV8,"YUV422"},
  {PIXEL_FORMAT_RGB , "RGB24" } ,
  {PIXEL_FORMAT_RAW8, "RAW8"}//,
  //{PIXEL_FORMAT_RAW16,"RAW16"},
  //{PIXEL_FORMAT_RAW12,"RAW12"}
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IP_OnvifCam::IP_OnvifCam()
{
  double dStart = GetHRTickCount() ;
  m_pStreamInput = new CInputConnector( userdata , InputStreamProcessor , this ) ;
  m_pStreamInput->SetName( "StreamInput" ) ;
  m_pStreamOutput = new COutputConnector( userdata ) ;
  m_pStreamOutput->SetName( "StreamOutput" ) ;
  m_bGUIReconnect = false ;
  m_uiSerialNumber = m_uiConnectedSerialNumber = 0 ;
  m_CurrentROI = CRect( 0 , 0 , 640 , 480 );
  m_pixelFormat = PIXEL_FORMAT_MONO8;
  m_uiPacketSize = 2048 ;
  m_FormatNotSupportedDispayed = false;
  m_GadgetInfo = "IP_OnvifCam";
  m_WaitEventFrameArray[ 0 ] = m_evFrameReady = CreateEvent( NULL , FALSE , FALSE , NULL ) ;
  m_WaitEventBusChangeArr[ 1 ] = m_WaitEventFrameArray[ 1 ] = m_evExit ;
  m_WaitEventBusChangeArr[ 0 ] = m_evBusChange = CreateEvent( NULL , FALSE , FALSE , NULL ) ;
  m_evGrabbed = CreateEvent( NULL , FALSE , FALSE , NULL ) ;
  m_evReconnect = CreateEvent( NULL , FALSE , FALSE , NULL ) ;
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
  m_iNoCameraStatus = 0 ;
  m_bInitialized = false ;
  m_bStopInitialized = false ;
  m_bRescanCameras = true ;
  m_iNNoSerNumberErrors = 0 ;
  m_iWBRed = m_iWBlue = 512 ;
  m_bLocalStopped = FALSE ;
  m_TriggerMode = TrigNotSupported ;
  m_dLastInCallbackTime = 0. ;
  m_dwNArrivedEvents = 0 ;
  m_iFPSx10 = 300 ; // 30 FPS
  m_pImage = NULL ;
  m_nPlaydecHandle = -1;
  m_iPlayhandle = -1;
  //m_nIndex = -1;
  m_bReconnect = false;
  memset( &m_BMIH , 0 , sizeof( BITMAPINFOHEADER ) );
  memset( &m_RealBMIH , 0 , sizeof( BITMAPINFOHEADER ) );

  m_pIPCamGadgets[ m_iNCam ] = this;
  m_nIndex = m_iNCam;
  m_iNCam++;


#ifdef _DEBUG
  m_iMutexTakeCntr = 0 ;
#endif
  if ( m_iNCam == 1 )
  {
    DriverInit() ;
  }
  HANDLE ghSemaphore = OpenSemaphore( SEMAPHORE_ALL_ACCESS , false , _T( H264SemaphoreName ) );
  if ( ghSemaphore == NULL )
  {
    ghSemaphore =
      CreateSemaphore(
      NULL ,
      0 ,
      MAX_SEM_COUNT ,
      _T( "H264Semaphore" ) );
  }
  else
  {
    DWORD err;
    if ( !ReleaseSemaphore(
      ghSemaphore ,  // handle to semaphore
      1 ,            // increase count by one
      NULL ) )       // not interested in previous count
    {
      err = GetLastError();
    }

  }


  double dBusyTime = GetHRTickCount() - dStart ;
  TRACE( "\nIP_OnvifCam::IP_OnvifCam: Start %g , Busy %g" , dStart , dBusyTime ) ;

}

//device disconnect callback
unsigned int ip_to_hex( const char* address )
{
  unsigned int ip = 0;
  char* str = _strdup( address );
  char dels[] = ".";
  char* token = strtok( str , dels );
  int tokencount = 0;
  while ( token != NULL )
  {
    if ( tokencount < 4 )
    {
      int n = atoi( token );
      ip = ip | n << 8 * (3 - tokencount);
    }
    tokencount++;
    token = strtok( NULL , dels );
  }
  free( str );

  if ( tokencount != 4 )
    ip = 0;

  return ip;
}
FXString hex_to_ip( unsigned int ip )
{
  struct in_addr addr;
  addr.s_addr = htonl( ip );
  char *s = inet_ntoa( addr );
  FXString fxIP( s );
  return fxIP;
}
void __stdcall DisConnectBackCallFunc( LONG lLoginID , char *pchDVRIP , LONG nDVRPort , FXSIZE dwUser )
{
  IP_OnvifCam* pThis = (IP_OnvifCam*) dwUser;
  if ( pThis == NULL )
  {
    ASSERT( FALSE );
    return ;
  }
  pThis->SetRelevantCamera( lLoginID , pchDVRIP );
  //pThis->ReConnect(lLoginID, pchDVRIP, nDVRPort);

}

IP_OnvifCam::~IP_OnvifCam()
{
  if ( m_hGrabThreadHandle )
  {
    DWORD dwRes = WaitForSingleObject( m_hGrabThreadHandle , 1000 ) ;
    ASSERT( dwRes == WAIT_OBJECT_0 ) ;
    dwRes = WaitForSingleObject( m_hCamAccessMutex , 1000 ) ;
    ASSERT( dwRes == WAIT_OBJECT_0 ) ;
    CloseHandle( m_hCamAccessMutex ) ;
  }
}
void IP_OnvifCam::SetRelevantCamera( LONG lLoginID , char *pchDVRIP )
{
  m_IPCamArrLock.Lock();
  FXString ip;
  ip = pchDVRIP;
  //ip.Remove('.');
     //unsigned int uiSN = (unsigned int)atoi(ip);
  unsigned int uiSN = ip_to_hex( ip );
  for ( unsigned i = 0; i < m_iNCam; i++ )
  {
    if ( m_pIPCamGadgets[ i ]->m_uiSerialNumber == uiSN && m_pIPCamGadgets[ i ]->m_LogIn == lLoginID )
    {
      m_pIPCamGadgets[ i ]->m_bReconnect = true;
      SEND_GADGET_TRACE( "Camera IP:'%s' - connect callback recieved" , hex_to_ip( uiSN ) );
      break;
    }
  }
  m_IPCamArrLock.Unlock();
}
bool IP_OnvifCam::CopyBuffer( unsigned char* data , DWORD size )
{
  m_GrabLock.Lock() ;
  if ( m_pImage )
  {
    delete m_pImage ;
    m_pImage = NULL ;
  }
  m_pImage = new unsigned char[ size ]();
  memcpy( m_pImage , (unsigned char*) data , size );
  m_dwPacketSize = size;
  m_GrabLock.Unlock() ;
  SetEvent( m_evGrabbed ) ;
  return true;
}
bool IP_OnvifCam::DriverInit()
{
  BOOL bResult = H264_DVR_Init( (fDisConnect) DisConnectBackCallFunc , (DWORD_PTR) this );
  H264_DVR_SetConnectTime( 5000 , 3 );//5000
  return EnumCameras();
}

bool IP_OnvifCam::EnumCameras()
{
  FXAutolock al( m_ConfigLock ) ;
  //if ( m_bCamerasEnumerated && !m_bRescanCameras )
  //  return true ;
  if ( m_iCamNum > 0 )
  {
    return true;
  }
  int nRetLength = 0;
  bool bRet = H264_DVR_SearchDevice( (char *) &m_CamSearchData , sizeof( m_CamSearchData ) , &nRetLength , 5000 );

  if ( bRet && nRetLength > 0 )
  {
    m_iCamNum = nRetLength / sizeof( SDK_CONFIG_NET_COMMON );
    m_bCamerasEnumerated = true;
  }

  ASSERT( m_iCamNum < MAX_CAMERASNMB );
  if ( m_iCamNum == 0 )
  {
    return false;
  }
  else
  {
    for ( unsigned int i = 0; i < m_iCamNum; i++ )
    {
      if ( m_CamSearchData[ i ].TCPPort == 0 || m_CamSearchData[ i ].HostIP.l == 0 )
      {
        continue;
      }

      FXString strTemp( _T( "" ) );
      strTemp.Format( _T( "%d.%d.%d.%d" ) , m_CamSearchData[ i ].HostIP.c[ 0 ] , m_CamSearchData[ i ].HostIP.c[ 1 ] , m_CamSearchData[ i ].HostIP.c[ 2 ] , m_CamSearchData[ i ].HostIP.c[ 3 ] );
      pData = new DEV_INFO;
      memset( pData , 0 , sizeof( DEV_INFO ) );
      memcpy( &pData->NetComm , &m_CamSearchData[ i ] , sizeof( SDK_CONFIG_NET_COMMON ) );
      pData->nTotalChannel = 1;
      pData->nPort = m_CamSearchData[ i ].TCPPort;
      FXString camName;
      camName.Format( "Cam%d" , i + 1 );
      strcpy( pData->szDevName , camName );
      strcpy( pData->szUserName , "admin" );
      strcpy( pData->szPsw , "" );
      strcpy( pData->szIpaddress , strTemp.GetBuffer( 0 ) );
      unsigned ip = ip_to_hex( pData->szIpaddress );
      //strTemp.Remove('.');
      pData->nserialnmb = ip;//atoi(strTemp);
      m_CamInfo[ i ] = *pData;
      delete pData;
    }
  }

  m_bCamerasEnumerated = (m_iCamNum > 0) ;
  m_bRescanCameras = false ;
  return true;
}

void IP_OnvifCam::ShutDown()
{
  C1394Camera::ShutDown() ;
  if ( m_hGrabThreadHandle )
  {
    bool bRes = IP_Onvif_DoAndWait( PGR_EVT_SHUTDOWN , 1000 ) ;
    DWORD dwRes = WaitForSingleObject( m_hGrabThreadHandle , 2000 ) ;
    ASSERT( dwRes == WAIT_OBJECT_0 ) ;
  }
  HANDLE ghSemaphore = OpenSemaphore( SEMAPHORE_ALL_ACCESS , false , _T( H264SemaphoreName ) );
  DWORD dwWaitResult = WaitForSingleObject( ghSemaphore , 0L );
  if ( dwWaitResult == WAIT_TIMEOUT )
  {
    H264_DVR_Cleanup();
  }

  FxReleaseHandle( m_evFrameReady ) ;
  FxReleaseHandle( m_evBusChange ) ;
  FxReleaseHandle( m_evGrabbed ) ;
  FxReleaseHandle( m_evReconnect ) ;

}

void IP_OnvifCam::AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame )
{
  if ( !pParamFrame )
    return;


  CTextFrame* tf = pParamFrame->GetTextFrame( DEFAULT_LABEL );
  if ( tf )
  {
    FXParser pk = (LPCTSTR) tf->GetString();
    FXString cmd;
    FXString param;
    FXSIZE pos = 0;
    pk.GetWord( pos , cmd );

    if ( cmd.CompareNoCase( "list" ) == 0 )
    {
      pk.Empty();
      for ( int i = 0; i < m_Properties.GetSize(); i++ )
      {
        //FXParser
        pk += m_Properties[ i ].property; pk += "\r\n";
      }
    }
    else if ( (cmd.CompareNoCase( "get" ) == 0) && (pk.GetWord( pos , cmd )) )
    {
      unsigned id = GetPropertyId( cmd );
      if ( id != WRONG_PROPERTY )
      {
        FXSIZE value;
        bool bauto;
        m_SettingsLock.Lock() ;
        bool bRes = GetCameraProperty( id , value , bauto ) ;
        m_SettingsLock.Unlock() ;
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
            pk = (LPCTSTR) value;
        }
        else
          pk = "error";
      }
      else
        pk = "error";
    }
    else if ( (cmd.CompareNoCase( "set" ) == 0) && (pk.GetWord( pos , cmd )) && (pk.GetParamString( pos , param )) )
    {
      unsigned id = GetPropertyId( cmd );
      if ( id != WRONG_PROPERTY )
      {
        FXSIZE value = 0;
        bool bauto = false , Invalidate = false;
        if ( IsDigitField( id ) )
        {
          if ( param.CompareNoCase( "auto" ) == 0 )
            bauto = true;
          else
            value = atoi( param );
        }
        else
          value = (FXSIZE) (LPCTSTR) param;
        m_SettingsLock.Lock() ;
        bool bWasStopped = m_bWasStopped ;
        m_bWasStopped = false ;
        bool bRes = SetCameraProperty( id , value , bauto , Invalidate ) ;
        if ( !bWasStopped  && m_bWasStopped )
          IP_Onvif_DoAndWait( PGR_EVT_INIT | PGR_EVT_START_GRAB ) ;
        m_bWasStopped = bWasStopped ;
        m_SettingsLock.Unlock() ;
        pk = (bRes) ? "OK" : "error" ;
      }
      else
        pk = "error";
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

bool IP_OnvifCam::CameraInit()
{
#ifdef _DEBUG
  if ( IP_Onvif_DoAndWait( PGR_EVT_INIT , 150000 ) )
  #else
  if ( IP_Onvif_DoAndWait( PGR_EVT_INIT , 5000 ) )
  #endif
  {
    //CloseStream();
//     if ( m_pCamera )
//       return BuildPropertyList();
  }
  return false ;
}

bool IP_OnvifCam::IP_Onvif_CamInit()
{
  if ( m_uiConnectedSerialNumber && (m_uiConnectedSerialNumber == m_uiSerialNumber)
    && !m_bShouldBeReprogrammed )
  {
    //     if ( m_pCamera && CheckCameraPower( m_pCamera ) ) // already connected to the same camera
    return true ;
  }

  //   if ( m_pCamera )
  //     IP_Onvif_CamClose();

    //FXAutolock al( m_LocalConfigLock ) ;

  if ( !m_uiSerialNumber || !m_iCamNum
    || ((m_CurrentCamera != 0xffffffff) && (m_CurrentCamera >= m_iCamNum)) )
  {  // nothing to connect
    //     if ( bMain )
    //       C1394_SENDERR_0("Fatal error: No IP_Onvif_ cameras found on a bus.");
    return false;
  }

  //   m_LastError = m_BusManager.GetCameraFromSerialNumber(
  //     m_dwSerialNumber , &m_CurrentCameraGUID);
  //  if (m_LastError!=0)
  //  {
  //     if ( ++m_iNNoSerNumberErrors <= 2 ) // Commented for Onvif
  //       SENDERR("Fatal error GetCameraFromSerialNumber: %s",
  //       m_LastError.GetDescription( ));
  //    return false;
  //  }
  m_iNNoSerNumberErrors = 0 ;

  //   m_LastError=m_BusManager.GetInterfaceTypeFromGuid( &m_CurrentCameraGUID, &m_IntfType );
  //  if ( m_LastError != 0 )
  //  {   
  //     SENDERR("Fatal error GetInterfaceTypeFromGuid: %s", // Commented for Onvif
  //       m_LastError.GetDescription( ));
  //    return FALSE;
  //  }    

  m_Status.Empty() ;

  //   FlyCapture2::CameraBase*&  pCam = m_pCamera ;
  //   if ( m_IntfType == INTERFACE_GIGE )
  //     pCam = new GigECamera ;   // GIGE interface
  //   else
  //     pCam = new Camera ; // USB or 1394 interface

  //   if ( !pCam ) // Commented for Onvif
  //     return FALSE ;
  // 
  //   m_LastError=pCam->Connect(&m_CurrentCameraGUID);
  //   if (m_LastError!=0)
  //   {
  //     m_CurrentCameraGUID = PGRGuid() ;
  //     SENDERR("Fatal error Connect: %s",m_LastError.GetDescription( ));
  //     return false;
  //   }
  // 
  //   FC2Config   Config;
  //   m_pCamera->GetConfiguration(&Config);
  //   Config.grabTimeout = 1000 ;
  //   Config.grabMode = DROP_FRAMES ;
  //   Config.numImageNotifications = 1 ;
  //   if ( m_IntfType == INTERFACE_IEEE1394  )
  //   {
  //     Config.isochBusSpeed = BUSSPEED_S_FASTEST ;
  //     Config.asyncBusSpeed = BUSSPEED_S_FASTEST ;
  //     Config.bandwidthAllocation = BANDWIDTH_ALLOCATION_ON ;
  //     Config.numBuffers = 5 ;
  //   }
  //   m_LastError = pCam->SetConfiguration(&Config);
  //   if (m_LastError!=0)
  //   {
  //     IP_Onvif_CamClose() ;
  //     SENDERR("SetConfiguration Fatal error: %s",m_LastError.GetDescription( ));
  //     return false;
  //   }
  // 
  //   m_LastError = pCam->GetCameraInfo( &m_CameraInfo );
   // if( m_LastError != 0 )
   // {
    //  return false;
    //}
    //bool bColorCam = m_CameraInfo.isColorCamera /*(m_CameraInfo.bayerTileFormat != BayerTileFormat::NONE)*/ ;
  //   if ( bColorCam )
  //   {
  //     if ( m_pixelFormat == PIXEL_FORMAT_MONO8 )
  //       m_pixelFormat = PIXEL_FORMAT_RAW8 ;
  //     if ( m_pixelFormat == PIXEL_FORMAT_MONO16 )
  //       m_pixelFormat = PIXEL_FORMAT_RAW16 ;
  //   }
  //   else
  //   {
  //     if ( m_pixelFormat == PIXEL_FORMAT_RAW8 )
  //       m_pixelFormat = PIXEL_FORMAT_MONO8 ;
  //     if ( m_pixelFormat == PIXEL_FORMAT_RAW16 )
  //       m_pixelFormat = PIXEL_FORMAT_MONO16 ;
  //   }

  //   if ( m_IntfType != INTERFACE_GIGE )
  //   {
  //     int iPacketSize = SetPacketSize( 
  //       (m_TriggerMode == TriggerOn ) ? m_uiPacketSize : m_iFPSx10 ,
  //       !(m_TriggerMode == TriggerOn ) ) ;
  //     Format7ImageSettings f7is;
  //     f7is.mode=m_Mode;
  //     f7is.pixelFormat=m_pixelFormat;
  //     f7is.offsetX=m_CurrentROI.left;
  //     f7is.offsetY=m_CurrentROI.top;
  //     f7is.width=m_CurrentROI.Width();
  //     f7is.height=m_CurrentROI.Height();
  //     Format7PacketInfo f7pi;
  //     bool valid;
  //     m_LastError = ((Camera*)m_pCamera)->ValidateFormat7Settings(&f7is,&valid,&f7pi);
  //     if ( m_LastError != 0 )
  //     {
  //       if ( m_pixelFormat == PIXEL_FORMAT_MONO8 )
  //       {
  //         f7is.pixelFormat = PIXEL_FORMAT_RAW8 ;
  //         m_LastError = ((Camera*)m_pCamera)->ValidateFormat7Settings(&f7is,&valid,&f7pi);
  //         if ( m_LastError != 0 )
  //         {
  //           IP_Onvif_CamClose() ;
  //           SENDERR("ValidateFormat7Settings error: %s",m_LastError.GetDescription( ));
  //           return false;
  //         }
  //         else
  //           m_pixelFormat = PIXEL_FORMAT_RAW8 ;
  //       }
  //     }
  //     if ( m_uiPacketSize < f7pi.unitBytesPerPacket )
  //       m_uiPacketSize = f7pi.unitBytesPerPacket ;
  //     if ( m_uiPacketSize > f7pi.maxBytesPerPacket )
  //       m_uiPacketSize = f7pi.maxBytesPerPacket ;
  //     else
  //     {
  //       // packet size should be multiple of 
  //       m_uiPacketSize = f7pi.unitBytesPerPacket * ( m_uiPacketSize / f7pi.unitBytesPerPacket ) ;
  //     }
  //     m_LastError = ((Camera*)m_pCamera)->SetFormat7Configuration( &m_f7is , m_uiPacketSize );
  //     if (m_LastError!=0)
  //     {
  //       IP_Onvif_CamClose() ;
  //       SENDERR("SetFormat7Configuration Fatal error: %s",m_LastError.GetDescription( ));
  //       return false;
  //     }
  //     else
  //     {
  //       m_LastError = ((Camera*)m_pCamera)->
  //         GetFormat7Configuration( &m_f7is , &m_uiPacketSize , &m_fPercentage);   
  //     }
  //   }
  //   else  // GIGE
  //   {
  // 
  //   }

  //   if ( m_LastError != 0 )
  //     m_Status.Format("CameraInit(): %s", m_LastError.GetDescription( ));
  //   int iLastChannel = -1 ;
  //   for ( int i = 0 ; i < 5 ; i++ )
  //   {
  //     StrobeInfo NewStrobe ;
  //     NewStrobe.source = i ;
  //     m_LastError = m_pCamera->GetStrobeInfo( &NewStrobe ) ;
  //     if ( m_LastError == 0 )
  //     {
  //       iLastChannel = i ;
  //       m_StrobeInfo.Add( NewStrobe ) ;
  //     }
  //   }
  //   m_CameraID.Format("%d_%s",m_dwSerialNumber , m_CameraInfo.modelName  );
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
    C1394_SENDINFO_0( m_Status );
    m_Status.Empty() ;
  }
  m_bInitialized = true ;
  m_bGUIReconnect = true ;
  FXAutolock al( m_ConfigLock ) ;
  m_uiConnectedSerialNumber = m_uiSerialNumber ;
  for ( int i = 0 ; i < m_BusyCameras.GetCount() ; i++ )
  {
    if ( m_BusyCameras[ i ] == m_uiSerialNumber )
      return true ;
  }
  m_BusyCameras.Add( m_uiSerialNumber ) ;
  //   TRACE("\nIP_OnvifCam::IP_Onvif_CamInit - Camera %u initialized" , m_CameraInfo.serialNumber ) ;
  return true;
}

// void IP_OnvifCam::IP_Onvif_DeleteCam( FlyCapture2::CameraBase* pCam )
// {
//   if ( pCam )
//   {
//     if(m_disableEmbeddedTimeStamp)
//     {
//       DisableEmbeddedTimeStamp( pCam );
//     }
//     //     UnregisterCallbacks() ;
//     if ( pCam->IsConnected() )
//       m_LastError = pCam->Disconnect();
//     delete pCam ;
//     FXAutolock al( m_ConfigLock ) ;
//     m_dwConnectedSerialNumber = 0 ;
//     for ( int i = 0 ; i < m_BusyCameras.GetCount() ; i++ )
//     {
//       if ( m_BusyCameras[i] == m_dwSerialNumber )
//       {
//         m_BusyCameras.RemoveAt( i ) ;
//       }
//     }
//   }
//   m_bInitialized = false ;
//   TRACE("\nIP_OnvifCam::IP_Onvif_DeleteCam - Camera %u deleted" , m_CameraInfo.serialNumber ) ;
// }
// 

void IP_OnvifCam::IP_Onvif_CamClose()
{
  if ( m_iPlayhandle > 0 )
  {
    H264_DVR_DelRealDataCallBack_V2( m_iPlayhandle , RealDataCallBack , (FXSIZE) this );
    H264_DVR_StopRealPlay( m_iPlayhandle );
    m_iPlayhandle = -1;
    m_nPlaydecHandle = -1;
  }

  //   if ( m_pCamera )
  //   {
  //     IP_Onvif_CameraStop() ;
  // 
  //     IP_OnvifCam_DeleteCam( m_pCamera );    
  //     m_CallbackThreadName.Empty() ;
  //     //       m_Status.Format("CameraClose() %s: %s", bMain ? "Main" : "GUI" , m_LastError.GetDescription( ));
  //     //       C1394_SENDINFO_0(m_Status);
  //     m_CameraID = "-IP_Onvif";
  //     m_CurrentCameraGUID = PGRGuid() ;
  //     m_pCamera = NULL ;
  //   }
  //   if ( m_pGUICamera )
  //   {
  //     m_pGUICamera->Disconnect() ;
  //     m_pGUICamera = NULL ;
  //   }
  //   TRACE("\nIP_OnvifCam::IP_Onvif_CamClose - Camera %u closed" , m_CameraInfo.serialNumber ) ;
}
void IP_OnvifCam::IP_Onvif_LocalStreamStart()
{
  //   if ( m_pCamera )
  //   {
  //     UINT uiRead ;
  //     m_LastError = m_pCamera->ReadRegister( 0x614 , &uiRead ) ;
  //     if ( m_LastError == 0 )
  //     {
  //       if ( uiRead == 0 ) // stopped
  //       {
  //         m_LastError = m_pCamera->WriteRegister( 0x614 , 0x80000000 ) ;
  //         TRACE("\nIP_OnvifCam::IP_Onvif_LocalStreamStart - Camera %u stream started" , m_CameraInfo.serialNumber ) ;
  //       }
  //       m_bLocalStopped = false ;
  //       return ;
  //     }
  //   }
  //   TRACE("\nIP_OnvifCam::IP_Onvif_LocalStreamStart - Camera %u stream not started" , m_CameraInfo.serialNumber ) ;
}
void IP_OnvifCam::IP_Onvif_LocalStreamStop()
{
  //   if ( m_pCamera )
  //   {
  //     UINT uiRead ;
  //     m_LastError = m_pCamera->ReadRegister( 0x614 , &uiRead ) ;
  //     if ( m_LastError == 0 )
  //     {
  //       if ( uiRead != 0 ) // not stopped
  //       {
  //         m_LastError = m_pCamera->WriteRegister( 0x614 , 0 ) ;
  //         TRACE("\nIP_OnvifCam::IP_Onvif_LocalStreamStart - Camera %u stream stopped" , m_CameraInfo.serialNumber ) ;
  //       }
  //       m_bLocalStopped = true ;
  //       return ;
  //     }
  //   }
  //   TRACE("\nIP_OnvifCam::IP_Onvif_LocalStreamStart - Camera %u stream not stopped" , m_CameraInfo.serialNumber ) ;
}


void IP_OnvifCam::CameraClose()
{
  IP_Onvif_DoAndWait( PGR_EVT_RELEASE , 2000 ) ;
}

bool IP_OnvifCam::DriverValid()
{

  return  (m_iCamNum != 0) /*&& (m_pCamera && m_pCamera->IsConnected())*/;
}

bool IP_OnvifCam::CameraStart()
{
  m_bRun = TRUE ;
  m_ConnectionLock.Lock();
  Connect();
  //OpenStream();
  m_ConnectionLock.Unlock();
  /*
  if (m_ControlThreadName.IsEmpty() && m_continueGrabThread && m_dwGrabThreadId )
  {
    GetGadgetName( m_ControlThreadName ) ;
    m_ControlThreadName += _T("_CTRL") ;
    ::SetThreadName( (LPCSTR)((LPCTSTR)m_ControlThreadName) , m_dwGrabThreadId ) ;
  }
  */

  IP_Onvif_DoAndWait( PGR_EVT_START_GRAB , 1000 )  ;

  return true ;
}

DEV_INFO * IP_OnvifCam::GetSelectedCamData()
{
  for ( unsigned int i = 0; i < m_iCamNum; i++ )
  {
    if ( m_CamInfo[ i ].nserialnmb == m_uiSerialNumber )
      return &m_CamInfo[ i ];
  }
  return nullptr;
}

int IP_OnvifCam::SerialToNmb( int nserial )
{
  for ( unsigned int i = 0; i < m_iCamNum; i++ )
  {
    if ( m_CamInfo[ i ].nserialnmb == nserial )
      return i;
  }
  return -1;
}
void IP_OnvifCam::ReConnect( LONG lLoginID , char *pchDVRIP , LONG nDVRPort )
{
  //m_lostLogID = lLoginID;
  //m_lostIP = pchDVRIP;
  //m_bReconnect = true;
  //SetEvent(m_evReconnect);

  /*
  DEV_INFO * pDev = GetSelectedCamData();
  if(!pDev)
    return;
  if(lLoginID == pDev->lLoginID)
  {
    CameraStop();
    Connect();
  }
  */

}
int IP_OnvifCam::Connect()
{
  if ( m_uiSerialNumber <= 0 )
    return -1;
  int nRet = 0;
  DEV_INFO * pDev = GetSelectedCamData();
  if ( !pDev )
  {
    int port = 34567;


    //struct in_addr addr;
    //addr.s_addr = htonl(m_uiSerialNumber); // s_addr must be in network byte order 
    //char *s = inet_ntoa(addr);




    pData = new DEV_INFO;
    memset( pData , 0 , sizeof( DEV_INFO ) );
    strcpy( pData->szIpaddress , hex_to_ip( m_uiSerialNumber )/*s*/ );
    pData->nTotalChannel = 1;
    pData->nPort = port;
    FXString camName;
    camName.Format( "Cam%d" , m_iCamNum + 1 );
    strcpy( pData->szDevName , camName );
    strcpy( pData->szUserName , "admin" );
    strcpy( pData->szPsw , "" );
    unsigned ip = ip_to_hex( pData->szIpaddress );
    pData->nserialnmb = ip;
    m_CamInfo[ m_iCamNum ] = *pData;
    m_iCamNum++;
    delete pData;
    pDev = GetSelectedCamData();
    if ( !pDev )
      return -1;
  }
  if ( pDev->lLoginID <= 0 )
  {
    H264_DVR_DEVICEINFO OutDev;
    int nError = 0;
    H264_DVR_SetConnectTime( 3000 , 1 );
    long lLogin = H264_DVR_Login( pDev->szIpaddress , pDev->nPort , pDev->szUserName , pDev->szPsw , &OutDev , &nError );
    int nAttemps = 0;
    while ( lLogin <= 0 && nAttemps++ < 3 )
    {
      int nErr = H264_DVR_GetLastError();
      SEND_GADGET_TRACE( "Camera IP:'%s' Attemp- %d ,Login error - %d" , pDev->szIpaddress , nAttemps , nErr );
      Sleep( 500 );
      lLogin = H264_DVR_Login( pDev->szIpaddress , pDev->nPort , pDev->szUserName , pDev->szPsw , &OutDev , &nError );
    }
    if ( lLogin <= 0 )
    {
      int nErr = H264_DVR_GetLastError();
      SEND_GADGET_TRACE( "Camera IP:'%s' Login error - %d" , pDev->szIpaddress , nErr );
      return nRet;
    }
    SEND_GADGET_TRACE( "Camera IP:'%s' Login done " , pDev->szIpaddress );
    m_LogIn = lLogin;
    pDev->lLoginID = lLogin;
    H264_DVR_SetupAlarmChan( lLogin );
  }
  return Stream( pDev , 0 );
}

int IP_OnvifCam::Stream( DEV_INFO *pDev , int nChannel )
{
  H264_DVR_CLIENTINFO playstru;
  playstru.nChannel = nChannel;
  playstru.nStream = 0;
  playstru.nMode = 0;
  m_iPlayhandle = H264_DVR_RealPlay( pDev->lLoginID , &playstru );
  int counter = 5;
  while ( m_iPlayhandle <= 0 && counter-- > 0 )
  {
    Sleep( 10 );
    m_iPlayhandle = H264_DVR_RealPlay( pDev->lLoginID , &playstru );
    SEND_GADGET_TRACE( "Camera IP:'%s' was connected" , pDev->szIpaddress );
  }
  if ( m_iPlayhandle <= 0 )
  {
    DWORD dwErr = H264_DVR_GetLastError();
    CString sTemp( "" );
    sTemp.Format( "access %s channel%d fail, dwErr = %d" , pDev->szDevName , nChannel , dwErr );
    SEND_GADGET_TRACE( "Camera IP:'%s' connection failed, Error = %d" , pDev->szIpaddress , dwErr );
  }
  else
  {
    //set callback to decode receiving data
    H264_DVR_MakeKeyFrame( pDev->lLoginID , nChannel , 0 );
    H264_DVR_SetRealDataCallBack_V2( m_iPlayhandle , RealDataCallBack , (FXSIZE) this );
  }
  //m_lLogin = pDev->lLoginID;
  //m_iChannel = nChannel;
  return m_iPlayhandle;
}

void IP_OnvifCam::CameraStop()
{
  m_ConnectionLock.Lock();
  m_bRun = FALSE ;
  if ( m_iPlayhandle > 0 )
  {
    H264_DVR_DelRealDataCallBack_V2( m_iPlayhandle , RealDataCallBack , (FXSIZE) this );
    Sleep( 200 );
    H264_DVR_StopRealPlay( m_iPlayhandle );
    Sleep( 200 );
    H264_DVR_Logout( m_LogIn );
    Sleep( 200 );
    GetSelectedCamData()->lLoginID = -1;
    m_LogIn = -1;
    m_iPlayhandle = -1;
    m_nPlaydecHandle = -1;
  }
  //CloseStream();
  m_ConnectionLock.Unlock();
  /*
  //close decoder
  if (m_nPlaydecHandle >= 0)
  {
    H264_PLAY_CloseStream(m_nPlaydecHandle);
    H264_PLAY_Stop(m_nPlaydecHandle);
    m_nPlaydecHandle = -1;
  }
  */
  //   if ( !m_pCamera || !m_continueGrabThread) // Commented for Onvif
  //     return;

  IP_Onvif_DoAndWait( PGR_EVT_STOP_GRAB , 1000 )  ;
  C1394Camera::CameraStop();

  SetEvent( m_evFrameReady ) ;
  TRACE( "+++ IP_OnvifCam::CameraStop() for #%u\n " , m_uiSerialNumber );
  SEND_GADGET_TRACE( "Camera IP:'%s' was stopped" , hex_to_ip( m_uiSerialNumber ) );
}

void IP_OnvifCam::IP_Onvif_CameraStop() // for calling from DoGrabLoop
{
  m_GrabLock.Lock() ;
  if ( m_pImage )
  {
    delete m_pImage ;
    m_pImage = NULL ;
  }
  m_GrabLock.Unlock();

  //   if (!m_pCamera  ||  !m_continueGrabThread) 
  //     return;
  // 
  //   TriggerMode triggerMode;
  //   bool bTriggerWasOn = false ;
  //   if ( m_TriggerMode != TrigNotSupported )
  //   {
  //     m_LastError = m_pCamera->GetTriggerMode( &triggerMode );
  //     if (triggerMode.onOff)
  //     {
  //       triggerMode.onOff=false;
  //       m_pCamera->SetTriggerMode(&triggerMode );
  //       bTriggerWasOn = true ;
  //     }
  //   }
  //   m_bStopInitialized = true ;
  //   m_LastError = m_pCamera->StopCapture();
  //   m_GrabLock.Lock() ;
  //   if ( m_pNewImage )
  //   {
  //     delete m_pNewImage ;
  //     m_pNewImage = NULL ;
  //   }
  //   m_GrabLock.Unlock() ;
  // 
  //   if( m_LastError != 0 )
  //   {
  //     SENDERR("Stop Failure: %s", m_LastError.GetDescription());
  //   }
  //   if ( bTriggerWasOn )
  //   {
  //     triggerMode.onOff=true;
  //     m_pCamera->SetTriggerMode(&triggerMode );
  //   }
  // 
  //   TRACE("+++ IP_Onvif_CameraStop()for #%u\n " , m_dwSerialNumber );
}

CVideoFrame * IP_OnvifCam::DoH264VideoFrame( LPBYTE pData , UINT uiDataLen )
{
  CVideoFrame * pvf = CVideoFrame::Create() ;
  if ( pvf && pData )
  {
    //FXString fxLabel;
    //fxLabel.Format("%d",m_iPlayhandle);
    //pvf->SetLabel(fxLabel);
    pvf->lpData = NULL ;
    pvf->lpBMIH = (BITMAPINFOHEADER*) malloc( sizeof( BITMAPINFOHEADER ) + uiDataLen ) ;
    memset( pvf->lpBMIH , 0 , sizeof( BITMAPINFOHEADER ) ) ;
    pvf->lpBMIH->biClrImportant = m_iPlayhandle;
    pvf->lpBMIH->biClrUsed = m_nIndex;
    pvf->lpBMIH->biSize = sizeof( BITMAPINFOHEADER ) ;
    pvf->lpBMIH->biSizeImage = uiDataLen ;
    pvf->lpBMIH->biCompression = BI_H264 ;
    memcpy( &pvf->lpBMIH[ 1 ] , pData , uiDataLen ) ;
    pvf->SetTime( GetHRTickCount() ) ;
  }
  return pvf ;
}
/*
void IP_OnvifCam::UpdatePlayhandle()
{
  if(m_nPlaydecHandle == -1)
  {
    int sz = m_IPCamShared.GetBoxSize();
    for (int i = 0; i < m_IPCamShared.GetNBoxes(); i++)
    {
      m_IPCamShared.GetBoxContent(i,(BYTE*)&m_IPCamBusInfo,sz);
      if (m_IPCamBusInfo.m_iIndex == m_iPlayhandle)
      {
        m_nPlaydecHandle = m_IPCamBusInfo.m_iPlayDecIndex;
      }
    }
  }
}
*/
CVideoFrame* IP_OnvifCam::CameraDoGrab( double* StartTime )
{

  if ( m_bReconnect )
  {

    m_bReconnect = false;
    DEV_INFO * pDev = GetSelectedCamData();
    pDev->lLoginID = -1;
    Sleep( 1000 );
    CameraStop();
    //OpenStream();
    Sleep( 1000 );
    CameraStart();
    SEND_GADGET_TRACE( "Camera IP:'%s' was reconnected" , pDev->szIpaddress );
    CVideoFrame * pOut = NULL ;
    return pOut;
  }

  Sleep( 100 );
  //UpdatePlayhandle();

  CVideoFrame * pOut = NULL ;
  /*
  DWORD dwRes1 =  WaitForSingleObject( m_evGrabbed , 1000 ) ;
  if (dwRes1  == WAIT_OBJECT_0)
  {
    m_GrabLock.Lock();
    pOut = DoH264VideoFrame(m_pImage,m_dwPacketSize);
    m_GrabLock.Unlock();
    return pOut;
  }
  return pOut;
  */



  DWORD dwRes = WaitForMultipleObjects( 2 , m_WaitEventFrameArray , FALSE , /*INFINITE*/ 1000 ) ;
  if ( dwRes == WAIT_OBJECT_0 )
  {
    m_dLastStartTime = ::GetHRTickCount() ;
    *StartTime = m_dLastStartTime - m_dLastInCallbackTime ;
    //FXAutolock al( m_GrabLock );
//     FlyCapture2::Image * pImage = m_pNewImage ;
//     m_pNewImage = NULL ;
// 
//     if ( !m_pCamera && pImage )
//     {
//       delete pImage ;
//     }
//     else if ( pImage )
//     {
//       unsigned int rows,cols,stride;
//       PixelFormat format;
//       BayerTileFormat BayerFormat ;
// 
//       pImage->GetDimensions( &rows, &cols, &stride, &format , &BayerFormat );    
//       switch (format)
//       {
//       case PIXEL_FORMAT_RAW8:
//         {
//           //           Image * pCopied = new Image( rows , cols , PIXEL_FORMAT_RAW8 , RGGB ) ;
//           //           m_LastError = pImage->DeepCopy( pCopied ) ;
// 
//           Image * pRGB = new Image( rows , cols , PIXEL_FORMAT_RGB ) ;
// 
//           //pImage->SetDimensions(rows , cols , stride , format , (BayerTileFormat)NewBayer ) ;
//           pImage->SetDefaultColorProcessing( DIRECTIONAL_FILTER ) ;
//           m_LastError = pImage->Convert( /*PIXEL_FORMAT_411YUV8 , */pRGB ) ;
//           //          delete pCopied ;
//           if ( m_LastError != 0 )
//           {
//             SENDERR("Can't convert color image: %s", m_LastError.GetDescription());
//           }
//           else
//           {
//             //unsigned iSize = (3 * rows * cols)/2 ;
//             unsigned iSize = (3 * rows * cols) ;
//             LPBYTE data = (LPBYTE)pRGB->GetData() ;
// 
//             BITMAPINFOHEADER BMIH ;
// 
//             memset( &BMIH , 0 , sizeof(BITMAPINFOHEADER) );
//             BMIH.biSize=sizeof(BITMAPINFOHEADER);
//             BMIH.biHeight=rows;
//             BMIH.biWidth=cols;
//             BMIH.biSizeImage=iSize;
//             BMIH.biCompression = BI_RGB ;
//             BMIH.biBitCount = 24 ;
//             BMIH.biPlanes=1;
// 
//             LPBITMAPINFOHEADER pYUV9 = rgb24yuv9( &BMIH , data ) ;
//             CVideoFrame* vf=CVideoFrame::Create();
//             vf->lpBMIH = pYUV9 ;
//             vf->lpData = NULL ;
//             vf->SetLabel(m_CameraID);
//             vf->SetTime( GetHRTickCount() ) ;
// 
//             pOut = vf ;
//           }
//           delete pRGB ;
//         }
//         break ;
//       case PIXEL_FORMAT_411YUV8:
//         {
//           unsigned iSize = (3 * rows * cols)/2 ;
//           BITMAPINFOHEADER BMIH ;
// 
//           memset( &BMIH , 0 , sizeof(BITMAPINFOHEADER) );
//           BMIH.biSize=sizeof(BITMAPINFOHEADER);
//           BMIH.biHeight=rows;
//           BMIH.biWidth=cols;
//           BMIH.biSizeImage=iSize;
//           BMIH.biCompression = BI_YUV411 ;
//           BMIH.biBitCount = 12 ;
//           BMIH.biPlanes=1;
//           LPBYTE data = pImage->GetData() ;
// 
//           LPBITMAPINFOHEADER lpYUV9 = yuv411yuv9( &BMIH , data ) ;
//           if ( lpYUV9 )
//           {
//             CVideoFrame* vf=CVideoFrame::Create();
//             vf->lpBMIH = lpYUV9 ;
//             vf->lpData = NULL ;
//             vf->SetLabel(m_CameraID);
//             vf->SetTime( GetHRTickCount() ) ;
//             pOut = vf ;
//           }
//         }
//         break ;
//         //       case PIXEL_FORMAT_422YUV8:
//         //         {
//         // 
//         //         }
//         //         break ;
//       case PIXEL_FORMAT_RGB8: // the same with PIXEL_FORMAT_RGB
//         {
//           unsigned iSize = (3 * rows * cols) ;
//           LPBITMAPINFOHEADER lpBMIH=(LPBITMAPINFOHEADER)malloc(sizeof(BITMAPINFOHEADER) + iSize);
//           LPBYTE data = pImage->GetData() ;
// 
//           memset( lpBMIH , 0 , sizeof(BITMAPINFOHEADER) );
//           lpBMIH->biSize=sizeof(BITMAPINFOHEADER);
//           lpBMIH->biHeight=rows;
//           lpBMIH->biWidth=cols;
//           lpBMIH->biSizeImage=iSize;
//           lpBMIH->biCompression = BI_RGB ;
//           lpBMIH->biBitCount = 24 ;
//           lpBMIH->biPlanes=1;
// 
//           LPBITMAPINFOHEADER pYUV9 = rgb24yuv9( lpBMIH , data ) ;
//           CVideoFrame* vf=CVideoFrame::Create();
//           vf->lpBMIH = pYUV9 ;
//           vf->lpData = NULL ;
//           vf->SetLabel(m_CameraID);
//           vf->SetTime( GetHRTickCount() ) ;
// 
//           pOut = vf ;
//           delete lpBMIH ;
//         }
//         break ;
// 
//       case PIXEL_FORMAT_MONO8:
//         {
//           unsigned iSize=rows*cols;
// 
//           LPBITMAPINFOHEADER lpBMIH=(LPBITMAPINFOHEADER)malloc(sizeof(BITMAPINFOHEADER) + iSize);
//           LPBYTE data = (LPBYTE)&lpBMIH[1] ;
// 
//           memset( lpBMIH , 0 , sizeof(BITMAPINFOHEADER) );
//           lpBMIH->biSize=sizeof(BITMAPINFOHEADER);
//           lpBMIH->biHeight=rows;
//           lpBMIH->biWidth=cols;
//           lpBMIH->biSizeImage=iSize;
//           lpBMIH->biCompression=BI_Y8;
//           lpBMIH->biBitCount=8;
//           lpBMIH->biPlanes=1;
// 
//           memcpy( data , pImage->GetData() , iSize );
//           CVideoFrame* vf=CVideoFrame::Create();
//           vf->lpBMIH = lpBMIH ;
//           vf->lpData = NULL ;
//           vf->SetLabel(m_CameraID);
//           vf->SetTime( GetHRTickCount() ) ;
// 
//           pOut = vf ;
//         }
//         break ;
//       case PIXEL_FORMAT_MONO16:
//         {
//           unsigned iSize=2*rows*cols;
//           CVideoFrame* vf=CVideoFrame::Create();
//           vf->SetLabel(m_CameraID);
//           vf->lpBMIH=(LPBITMAPINFOHEADER)malloc(sizeof(BITMAPINFOHEADER)+iSize);
//           vf->lpData=NULL;
//           LPBYTE data=((LPBYTE)vf->lpBMIH)+sizeof(BITMAPINFOHEADER);
// 
//           memset(vf->lpBMIH,0,sizeof(BITMAPINFOHEADER));
//           vf->lpBMIH->biSize=sizeof(BITMAPINFOHEADER);
//           vf->lpBMIH->biHeight=rows;
//           vf->lpBMIH->biWidth=cols;
//           vf->lpBMIH->biSizeImage=iSize;
//           vf->lpBMIH->biCompression=BI_Y16;
//           vf->lpBMIH->biBitCount=16;
//           vf->lpBMIH->biPlanes=1;
//           //_swab((char*)rawImage.GetData(),(char*)data,iSize);
//           memcpy(data,pImage->GetData(),iSize);
// 
//           pOut = vf ;
//         }
//         break ;
//       default:
//         if ( !m_FormatNotSupportedDispayed )
//         {
//           SENDERR("Format not supported: 0x%x", format);
//           m_FormatNotSupportedDispayed=true;
//         }
//         break ;
//       }
//     }
//     delete pImage ;
// 
//   }
//   else if ( dwRes == WAIT_TIMEOUT )
//   {
//     if ( !m_pCamera || !m_pCamera->IsConnected() )
//     {
//       if ( m_dwSerialNumber && (m_dwSerialNumber != (0-1)) )
//       {
//         m_BusEvents |= PGR_EVT_START_GRAB ;
//         SetEvent( m_evBusChange ) ;
//         SetEvent( m_evFrameReady ) ;
//       }
//     }
    *StartTime = GetHRTickCount() ;
  }


  //m_evGrabbed
  return pOut ;
}
/*
void IP_OnvifCam::OpenStream()
{
    BYTE byFileHeadBuf;
  if (H264_PLAY_OpenStream(m_nIndex, &byFileHeadBuf, 1, SOURCE_BUF_MIN*50))
  {
    H264_PLAY_SetStreamOpenMode(m_nIndex, STREAME_REALTIME);
    int sz = m_IPCamShared.GetBoxSize();
    m_IPCamBusInfo.m_ID = m_nIndex;
    m_IPCamBusInfo.m_iIndex = m_iPlayhandle;
    int ret = m_IPCamShared.SetBoxContent(m_nIndex,(BYTE*)&m_IPCamBusInfo,sz);
  }
}
void IP_OnvifCam::CloseStream()
{
    H264_PLAY_CloseStream(m_nIndex);
    H264_PLAY_Stop(m_nIndex);
    m_IPCamShared.BoxClear(m_nIndex);//clear Shared Memory
}
*/
bool IP_OnvifCam::BuildPropertyList()
{
  m_Properties.RemoveAll();
  CAMERA1394::Property P;

  //   if ( !CheckAndAllocCamera() )
  //     return false ;

  //   int iWhiteBalanceMode = GetColorMode() ; // 0 - no color, 1 - by program, 2 - by camera
  // 
  //   FlyCapture2::CameraBase*& pCam = m_pGUICamera ; 
  for ( int i = 0; i < _getPropertyCount(); i++ )
  {
    if ( (int) cProperties[ i ].pr < BRIGHTNESS ) // virtual property
    {
      switch ( (int) cProperties[ i ].pr )
      {
        case FORMAT_MODE:
        {
          //           if ( m_IntfType != INTERFACE_GIGE )
          //           {
          int iNmb = sizeof( vFormats ) / sizeof( Videoformat );
          FXString items , tmpS;
          //            bool supported;
          //             Format7Info f7i;
          //             f7i.mode=MODE_0;
          //             ((Camera*)pCam)->GetFormat7Info(&f7i,&supported);
          //             if (!supported)
          //             {
          //               C1394_SENDERR_0("Camera doesn't support Format7 mode 0");
          //               break;
          //             }
          //             for (int j=0; j<iNmb; j++)
          //             {
          //               if (vFormats[j].vm & f7i.pixelFormatBitField)
          //               {
          //                 tmpS.Format("%s(%d),",vFormats[j].name,vFormats[j].vm);
          //                 items+=tmpS;
          //               }
          //             }
          P.name = cProperties[ i ].name;
          P.id = (unsigned) cProperties[ i ].pr;
          P.property.Format( "ComboBox(%s(%s))" , P.name , items );
          m_Properties.Add( P );
          //           }
          break;
        }
        case SETROI:
        {
          P.name = cProperties[ i ].name;
          P.id = (unsigned) cProperties[ i ].pr;
          P.property.Format( "EditBox(%s)" , P.name );
          m_Properties.Add( P );
          break;
        }
        case SETSTROBE0:
        case SETSTROBE1:
        case SETSTROBE2:
        case SETSTROBE3:
        {
          //           int iIndex = SETSTROBE0 - (unsigned)cProperties[i].pr ;
          //           if ( iIndex < m_StrobeInfo.GetCount() )
          //           {
          //             if ( m_StrobeInfo[iIndex].present )
          //             {
          //               P.name=cProperties[i].name;
          //               P.id=(unsigned)cProperties[i].pr;
          //               P.property.Format("EditBox(%s)",P.name);
          //               m_Properties.Add(P);
          //             }
          //           }
          //           break;
        }
        case PACKETSIZE:
          if ( m_TriggerMode == TriggerOn )
          {
            P.name = cProperties[ i ].name;
            P.id = (unsigned) cProperties[ i ].pr;
            P.property.Format( "EditBox(%s)" , P.name );
            m_Properties.Add( P );
          }
          break ;
        case WHITE_BAL_RED:
        case WHITE_BAL_BLUE:
        {
          //           if ( iWhiteBalanceMode == COLOR_BY_SOFTWARE ) // Program white balance
          //           {
          //             P.name = cProperties[i].name ;
          //             P.id=(unsigned)cProperties[i].pr;
          //             P.property.Format("Spin(%s,0,1023)",P.name);
          //             m_Properties.Add(P);
          //           }
        }
        break ;
        default:
          SENDERR( "Undefined property:'%s'" , cProperties[ i ].name );
      }
    }
    else
    {
      //       if ( (cProperties[i].pr == WHITE_BALANCE) && (iWhiteBalanceMode == 0) )
      //       { // no white balance for BW images
      //       }
      //       else
      //       {
      //         PropertyInfo  prpI;
      //         memset(&prpI,0,sizeof(prpI));
      //         prpI.type = cProperties[i].pr;
      //         m_LastError=pCam->GetPropertyInfo(&prpI);
      //         if ((m_LastError==0) && (prpI.present))
      //         {
      //           P.name=cProperties[i].name;
      //           P.id=(unsigned)cProperties[i].pr;
      //           bool bAdd = true ;
      //           switch ( prpI.type )
      //           {
      //           case TRIGGER_MODE:
      //             {
      //               P.property.Format("ComboBox(%s(NoTrigger(0),Trigger0+(1),Trigger0-(2))",cProperties[i].name);
      //               TriggerMode triggerMode;
      //               m_LastError = pCam->GetTriggerMode( &triggerMode );
      //               m_TriggerMode = (triggerMode.onOff) ? TriggerOn : TriggerOff ;
      //              }
      //             break ;
      //           case SHUTTER:
      //             {
      //               if ( prpI.absValSupported )
      //               {
      //                 P.property.Format("%s(%s,%d,%d)",
      //                   (prpI.autoSupported)?SETUP_SPINABOOL:SETUP_SPIN,
      //                   P.name, (int)(prpI.absMin * 1000.) , (int)(prpI.absMax * 1000.) );
      //               }
      //               else
      //               {
      //                 P.property.Format("%s(%s,%d,%d)",
      //                   (prpI.autoSupported)?SETUP_SPINABOOL:SETUP_SPIN,
      //                   P.name,prpI.min,prpI.max);
      //               }
      //             }
      //             break ;
      //           case GAIN:
      //             {
      //               if ( prpI.absValSupported )
      //               {
      //                 P.property.Format("%s(%s,%d,%d)",
      //                   (prpI.autoSupported)?SETUP_SPINABOOL:SETUP_SPIN,
      //                   P.name, (int)(prpI.absMin * 10.) , (int)(prpI.absMax * 10.) );
      //               }
      //               else
      //               {
      //                 P.property.Format("%s(%s,%d,%d)",
      //                   (prpI.autoSupported)?SETUP_SPINABOOL:SETUP_SPIN,
      //                   P.name,prpI.min,prpI.max);
      //               }
      //             }
      //             break ;
      //           case FRAME_RATE:
      //             {
      //               if ( m_TriggerMode != TriggerOn )
      //               {
      //                 if ( prpI.absValSupported )
      //                 {
      //                   P.property.Format("%s(%s,%d,%d)",
      //                     (prpI.autoSupported)?SETUP_SPINABOOL:SETUP_SPIN,
      //                     P.name, (int)(prpI.absMin * 10.) , (int)(prpI.absMax * 10.) );
      //                 }
      //                 else
      //                 {
      //                   P.property.Format("%s(%s,%d,%d)",
      //                     (prpI.autoSupported)?SETUP_SPINABOOL:SETUP_SPIN,
      //                     P.name,prpI.min,prpI.max);
      //                 }
      //               }
      //             }
      //             break ;
      //           case WHITE_BALANCE:
      //           case SATURATION:
      //           case HUE:
      //             {
      //               if ( iWhiteBalanceMode == COLOR_BY_CAMERA ) // Program white balance
      //               {
      //                 P.property.Format("%s(%s,%d,%d)",
      //                   (prpI.autoSupported)?SETUP_SPINABOOL:SETUP_SPIN,
      //                   P.name , prpI.min,prpI.max );
      //               }
      //               else
      //                 bAdd = false ;
      //             }
      //             break ;
      //           default:
      //             {
      //               P.property.Format("%s(%s,%d,%d)",
      //                 (prpI.autoSupported)?SETUP_SPINABOOL:SETUP_SPIN,
      //                 P.name,prpI.min,prpI.max);
      //             }
      //             break ;
      //           }
      //           if ( bAdd )
      //             m_Properties.Add(P);
      //         }
      //         else if ( m_LastError == 0 )
      //         {
      //           switch ( prpI.type )
      //           {
      //             case TRIGGER_MODE:
      //               m_TriggerMode = TrigNotSupported ;
      //             break ;
      // 
      //           default:
      break;
      //           }
      //         }
      //       }
    }
  }
  return true;
}

bool IP_OnvifCam::GetCameraProperty( unsigned i , FXSIZE &value , bool& bauto )
{
  if ( !CheckAndAllocCamera() )
    return false;

  if ( ((int) i) < BRIGHTNESS ) // virtual property
  {
    switch ( i )
    {
      case FORMAT_MODE:
      {
        bauto = false;
        value = m_pixelFormat;
        return true;
      }
      case SETROI:
      {
        static FXString sROI;
        CRect rc;
        GetROI( rc );
        sROI.Format( "%d,%d,%d,%d" , rc.left , rc.top , rc.right , rc.bottom );
        bauto = false;
        value = (FXSIZE) (LPCTSTR) sROI;
        return true;
      }
      case SETSTROBE0:
      case SETSTROBE1:
      case SETSTROBE2:
      case SETSTROBE3:
      {
        //         int iIndex = SETSTROBE0 - i ;
        //         if ( iIndex < m_StrobeInfo.GetCount() )
        //         {
        //           if ( m_StrobeInfo[iIndex].present )
        //           {
        //             static FXString StrobeDataAsText ;
        //             GetStrobe( StrobeDataAsText , iIndex ) ;
        //             value = (int)(LPCTSTR)StrobeDataAsText ;
        //             return true ;
        //           }
        //         }
        return false ;
      }
      case PACKETSIZE:
      {
        //         unsigned int PacketSize;
        //         float Percentage;
        //         Format7ImageSettings f7is;
        //         ASSERT( GetCamera( "GetCameraProperty1" ) ) ;
        //         m_LastError = ((Camera*)m_pGUICamera)->GetFormat7Configuration(&f7is,&PacketSize,&Percentage);
        //         ReleaseCamera() ;
        //         if ( m_LastError == 0 )
        //         {
        //           static FXString sPacketSize ;
        //           sPacketSize.Format( "%d" , PacketSize ) ;
        //           value = (int)(LPCTSTR)sPacketSize ;
        //           m_uiPacketSize = PacketSize ;
        //           return true ;
        //         }
        //         else
        //         {
        //           LPCTSTR pDescr = m_LastError.GetDescription() ;
        //           return false ;
        //         }
      }
      case WHITE_BAL_RED:
        value = m_iWBRed ;
        return true ;
      case WHITE_BAL_BLUE:
        value = m_iWBlue ;
        return true ;

      default:
        SENDERR( "Undefined property:'%s'" , cProperties[ i ].name );
    }
  }
  else
  {
    //     bauto=false;
    //     FlyCapture2::Property prp;
    //     memset(&prp,0,sizeof(prp));
    // 
    //     prp.type=(UINT)i;
    //     m_LastError=m_pGUICamera->GetProperty(&prp);
    //     if ((prp.present) && (m_LastError==0))
    //     {
    //       switch( i )
    //       {
    //       case TRIGGER_MODE:
    //         {
    //           TriggerMode triggerMode;
    //           m_LastError = m_pGUICamera->GetTriggerMode( &triggerMode );
    //           value = (triggerMode.onOff)? 1:0;
    //           if ( value && (triggerMode.polarity != 0) )
    //             value = 2 ;
    //           return true;
    //         }
    //       case SHUTTER:
    //         {
    //           value = (int)( prp.absValue * 1000. ) ;
    //           return true ;
    //         }
    //       case GAIN:
    //       case FRAME_RATE:
    //         value = (int)( prp.absValue * 10. ) ;
    //         return true ;
    //       default:
    //         {
    //           if (prp.autoManualMode)
    //             bauto=true;
    //           value=prp.valueA;
    //           return true;
    //         }
    //       }
    //    }
  }
  return false;
}
bool IP_OnvifCam::SetCameraProperty( unsigned i , FXSIZE &value , bool& bauto , bool& Invalidate )
{
  // #ifdef _DEBUG
  // #ifdef DEBUG_TRIGGER_MODE
  //   TriggerMode triggerModeDebugIn;
  //   if ( (i != TRIGGER_MODE) && m_pGUICamera )
  //     m_LastError = m_pGUICamera->GetTriggerMode( &triggerModeDebugIn );
  // #endif
  // #endif
  bool bRes = SetCameraPropertyEx( i , value , bauto , Invalidate ) ;
  // #ifdef _DEBUG
  // #ifdef DEBUG_TRIGGER_MODE
  //   if ( (i != TRIGGER_MODE) && m_pGUICamera )
  //   {
  //     TriggerMode triggerModeDebugOut;
  //     m_LastError = m_pGUICamera->GetTriggerMode( &triggerModeDebugOut );
  //     ASSERT( triggerModeDebugIn.onOff == triggerModeDebugOut.onOff ) ;
  //   }
  // #endif
  // #endif

  return bRes ;

}

bool IP_OnvifCam::SetCameraPropertyEx( unsigned i , FXSIZE &value , bool& bauto , bool& Invalidate )
{
  if ( !CheckAndAllocCamera() )
    return false;


  if ( ((int) i) < BRIGHTNESS ) // virtual property
  {
    switch ( i )
    {
      case FORMAT_MODE:
      {
        //         if ( m_IntfType != INTERFACE_GIGE )
        {
          CheckAndStopCapture() ;
          m_pixelFormat = (PixelFormat) value;
          Invalidate = true;
          if ( m_bInScanProperties )
            m_bShouldBeReprogrammed = true ;
          else if ( m_continueGrabThread )
          {
            bool bRes = IP_Onvif_DoAndWait( PGR_EVT_INIT , 1000 )  ;
            if ( m_bWasStopped )
              return CameraStart();
          }
          return (true);
        }
        //         else // GIGE
        //         {
        // 
        //         }
      }
      case SETROI:
      {
        CRect rc;
        if ( sscanf( (LPCTSTR) value , "%d,%d,%d,%d" , &rc.left , &rc.top , &rc.right , &rc.bottom ) == 4 )
        {
          rc.right += rc.left ;
          rc.bottom += rc.top ;
          SetROI( rc );
          return true;
        }
        return false;
      }
      case SETSTROBE0:
      case SETSTROBE1:
      case SETSTROBE2:
      case SETSTROBE3:
      {
        //         int iIndex = SETSTROBE0 - i ;
        //         if ( iIndex < m_StrobeInfo.GetCount() )
        //         {
        //           FXString StrobeDataAsText = (LPCTSTR)value ;
        //           return SetStrobe( StrobeDataAsText , iIndex ) ;
        //         }
      }
      case PACKETSIZE:
      {
        //         if ( m_TriggerMode == TriggerOn)
        //         {
        //           if ( m_IntfType != INTERFACE_GIGE )
        //           {
        //             CheckAndStopCapture() ;
        //             int iPacketSize = _tstoi( (LPCTSTR)value ) ;
        // 
        //             iPacketSize = SetPacketSize( iPacketSize , false ) ;
        //             ASSERT( iPacketSize > 0 ) ;
        //             Invalidate=true;
        // 
        //             if ( m_bInScanProperties )
        //               m_bShouldBeReprogrammed = true ;
        //             else if ( m_continueGrabThread )
        //             {
        //               bool bRes = IP_Onvif_DoAndWait( PGR_EVT_INIT , 1000)  ;
        //               if ( m_bWasStopped )
        //                 return CameraStart(); 
        //             }
        //             return true ;
        //           }
        //           else
        //           {
        // 
        //           }
        //         }
      }
      break ;
      case WHITE_BAL_RED:
      {
        //         Property whitebalance;
        //         whitebalance.type = WHITE_BALANCE;
        //         whitebalance.autoManualMode = false;
        // 
        //         m_LastError = m_pGUICamera->GetProperty(&whitebalance);
        //         if (m_LastError == 0)
        //         {
        //           whitebalance.valueA = m_iWBRed = value ;
        //           m_LastError = m_pGUICamera->SetProperty(&whitebalance);
        //           if (m_LastError == 0)
        //             return true ;
        //         }
        break ;
      }
      case WHITE_BAL_BLUE:
      {
        //         Property whitebalance;
        //         whitebalance.type = WHITE_BALANCE;
        //         whitebalance.autoManualMode = false;
        // 
        //         m_LastError = m_pGUICamera->GetProperty(&whitebalance);
        //         if (m_LastError == 0)
        //         {
        //           whitebalance.valueB = m_iWBlue = value ;
        //           m_LastError = m_pGUICamera->SetProperty(&whitebalance);
        //           if (m_LastError == 0)
        //             return true ;
        //         }
      }
      break ;
      default:
        SENDERR( "Undefined property:'%s'" , cProperties[ i ].name );
    }
  }
  else
  {
    //     FlyCapture2::Property prp;
    //     memset(&prp,0,sizeof(prp));
    //     prp.type=(UINT)i;
    // 
    //     m_LastError = m_pGUICamera->GetProperty(&prp);
    //     if ((m_LastError==0) && (prp.present))
    {
      switch ( i )
      {
        case TRIGGER_MODE:
        {
          //           TriggerMode triggerMode;
          //           m_LastError = m_pGUICamera->GetTriggerMode( &triggerMode );
          //           triggerMode.onOff = (value!=0);
          //           if ( value > 1 )
          //             triggerMode.polarity = 1 ;
          //           triggerMode.mode = 0;
          //           triggerMode.parameter = 0;
          //           triggerMode.source = 0;
          //           m_LastError = m_pGUICamera->SetTriggerMode( &triggerMode );
          //           m_TriggerMode = (value!=0) ? TriggerOn : TriggerOff ;
          //           Invalidate = true ;
          return (m_LastError == 0);
        }
        case SHUTTER:
        {
          //           prp.absControl = true;
          //           prp.absValue = (float)(value / 1000.) ;
          //           prp.onOff = true ;
          //           prp.autoManualMode = bauto;
          //           TRACE("+++ Set CameraProperty '%s' to %f\n",_getPropertyname(i),value);
          //           m_LastError=m_pGUICamera->SetProperty(&prp);
          // 
          //           FlyCapture2::Property PropRead ;
          //           PropRead.type = AUTO_EXPOSURE ;
          //           m_LastError = m_pGUICamera->GetProperty( &PropRead ) ;
          //           PropRead.onOff = bauto ;
          //           m_LastError = m_pGUICamera->SetProperty( &PropRead ) ;
          return (m_LastError == 0);
        }
        case GAIN:
          //         prp.absControl = true;
          //         prp.absValue = (float)(value / 10.) ;
          //         prp.onOff = true ;
          //         prp.autoManualMode = bauto;
          //         TRACE("+++ Set CameraProperty '%s' to %f\n",_getPropertyname(i),value);
          //         m_LastError=m_pGUICamera->SetProperty(&prp);
          return true ;
        case FRAME_RATE:
          //         if ( m_TriggerMode != TriggerOn )
          //         {
          //           prp.absControl = true;
          //           prp.absValue = (float)(value / 10.) ;
          //           prp.onOff = true ;
          //           prp.autoManualMode = bauto;
          //           TRACE("+++ Set CameraProperty '%s' to %f\n",_getPropertyname(i),value);
          //           m_LastError = m_pGUICamera->SetProperty(&prp);
          //           if ( m_LastError == 0 )
          //           {
          //             m_iFPSx10 = value ;
          //             int iOldPacketSize = m_uiPacketSize ;
          //             int iNewPacketSize = SetPacketSize( value , true ) ; // adjust packet size
          //             if ( iOldPacketSize != iNewPacketSize )
          //             {
          //               if ( m_IntfType != INTERFACE_GIGE )
          //               {
          //                 ASSERT( iNewPacketSize > 0 ) ;
          //                 CheckAndStopCapture() ;
          //                 if ( m_bInScanProperties )
          //                   m_bShouldBeReprogrammed = true ;
          //                 else if ( m_continueGrabThread )
          //                 {
          //                   bool bRes = IP_Onvif_DoAndWait( PGR_EVT_INIT , 1000)  ;
          //                   if ( m_bWasStopped )
          //                     return CameraStart(); 
          //                 }
          //                 return true ;
          //               }
          //               else
          //               {
          // 
          //               }
          //             }
          //           }
          //         }
          return true ;
        default:
        {
          //           prp.absControl = false;
          //           prp.valueA = value ;
          //           prp.onOff = (prp.type != AUTO_EXPOSURE) ;
          //           prp.autoManualMode=bauto;
          //           TRACE("+++ Set CameraProperty '%s' to %d\n",_getPropertyname(i),value);
          //           m_LastError=m_pGUICamera->SetProperty(&prp);
          return (m_LastError == 0);
        }
      }
    }
  }
  return false;
}

void IP_OnvifCam::GetROI( CRect& rc )
{
  rc = m_CurrentROI;
  rc.right -= rc.left ;
  rc.bottom -= rc.top ;
}

void IP_OnvifCam::SetROI( CRect& rc )
{
  //   if ( !m_pGUICamera )
  //     return ;
  // 
  //   if ( 
  //     (m_CurrentROI.Width() == rc.Width()) &&
  //     (m_CurrentROI.Height() == rc.Height()) 
  //     )
  //   {
  //     ASSERT( GetCamera( "SetROI1" ) ) ;
  // 
  //     // if format 7 and size the same simply do origin shift
  //     // by writing into IMAGE_ORIGIN register
  //     // without grab stop
  //     m_pGUICamera->WriteRegister(0xa08, (rc.left << 16)|rc.top );
  //     ReleaseCamera() ;
  //     m_CurrentROI = rc;
  //     return ;
  //   }
  // 
  //   CheckAndStopCapture() ;
  // 
  //   Format7Info fi;
  //   bool supported;
  // 
  //   ((Camera*)m_pGUICamera)->GetFormat7Info(&fi,&supported);
  //   if (rc.left!=-1)
  //   {
  //     rc.left&=~3;
  //     if ( rc.left < 0 || rc.left > (int)fi.maxWidth )
  //       rc.left = 0 ;
  // 
  //     rc.right&=~3;
  //     if ( rc.right < 0 || rc.right > (int)fi.maxWidth )
  //       rc.right = fi.maxWidth ;
  //     rc.top&=~3;
  //     if ( rc.top < 0 || rc.top > (int)fi.maxHeight)
  //       rc.top = 0 ;
  //     rc.bottom&=~3;
  //     if ( rc.bottom < 0  ||  rc.bottom > (int)fi.maxHeight )
  //       rc.bottom = fi.maxHeight ;
  //   }
  //   else
  //   {
  //     rc.left=rc.top=0;
  //     rc.right=fi.maxWidth ;
  //     rc.bottom=fi.maxHeight;
  //   }
  //   FXAutolock al(m_GrabLock);
  //   UINT iPacketSize = 0 ;
  //   if ( m_TriggerMode != TriggerOn ) // FPS
  //   {
  //     m_CurrentROI = rc ;
  //     iPacketSize = SetPacketSize( m_iFPSx10 , true ) ;
  //   }
  //   else // packet size
  //   {
  //     iPacketSize = SetPacketSize( m_uiPacketSize , false ) ;
  //   }
  // //   unsigned int PacketSize;
  // //   float Percentage;
  // //   Format7ImageSettings f7is;
  // //   ((Camera*)m_pGUICamera)->GetFormat7Configuration(&f7is,&PacketSize,&Percentage);
  // // 
  // //   f7is.offsetX=rc.left;
  // //   f7is.offsetY=rc.top;
  // //   f7is.width=rc.Width();
  // //   f7is.height=rc.Height();
  // //   Format7PacketInfo f7pi;
  // //   bool valid;
  // //   m_LastError = ((Camera*)m_pGUICamera)->ValidateFormat7Settings(&f7is,&valid,&f7pi);				 
  // //   if (m_LastError==0)
  // //   {
  // //     m_LastError = ((Camera*)m_pGUICamera)->
  // //       SetFormat7Configuration(&f7is,f7pi.recommendedBytesPerPacket); 
  // //     if (m_LastError==0)
  // //     {
  // //       m_LastError = ((Camera*)m_pGUICamera)->
  // //         GetFormat7Configuration(&f7is,&PacketSize,&Percentage);   
  // //       if (m_LastError==0)
  // //         m_CurrentROI=rc;
  // //     }
  // //   }
  //   if ( iPacketSize )
  //   {
  //     m_LastError = ((Camera*)m_pGUICamera)->
  //       SetFormat7Configuration(&m_f7is , iPacketSize ); 
  //     if (m_LastError==0)
  //     {
  //       m_LastError = ((Camera*)m_pGUICamera)->
  //         GetFormat7Configuration( &m_f7is , &m_uiPacketSize , &m_fPercentage);   
  //     }
  //   }
}

bool IP_OnvifCam::SetStrobe( const FXString& StrobeDataAsText , int iIndex )
{
  //   if ( !m_pGUICamera )
  //     return false;
  // 
  //   StrobeControl SControl ;
  //   int iDelay , iDuration ;
  //   if ( sscanf( (LPCTSTR)StrobeDataAsText , _T("%d,%d,%d,%d") ,
  //     &SControl.onOff , &SControl.polarity , 
  //     &iDelay , &iDuration ) == 4 )
  //   {
  //     SControl.delay = (float)((double)iDelay * 1.e-6) ;
  //     SControl.duration = (float)((double)iDuration * 1.e-6) ;
  //     SControl.source = iIndex ;
  //     m_LastError = m_pGUICamera->SetStrobe( &SControl ) ;
  //     if ( m_LastError != 0 )
  //     {
  //       C1394_SENDERR_2("Set Strobe %d error: %s",iIndex,m_LastError.GetDescription( ));
  //       return false ;
  //     }
  //     return true ;
  //   }
  // 
  return false ;
}

void IP_OnvifCam::GetStrobe( FXString& StrobeDataAsText , int iIndex )
{
  //   if ( !m_pGUICamera )
  //     return ;
  // 
  //   StrobeControl SControl ;
  //   SControl.source = iIndex ;
  //   m_LastError = m_pGUICamera->GetStrobe( &SControl ) ;
  //   if ( m_LastError != 0 )
  //   {
  //     SENDERR("Get Strobe error: %s",m_LastError.GetDescription( ));
  //   }
  //   else
  //   {
  //     StrobeDataAsText.Format( _T("%d,%d,%d,%d") ,
  //       SControl.onOff , SControl.polarity , 
  //       _ROUND(SControl.delay * 1.e6) , _ROUND( SControl.duration * 1.e6) ) ;
  //   }
}


bool IP_OnvifCam::IsGrabThreadRunning()
{
  return m_continueGrabThread;
}


DWORD WINAPI IP_OnvifCam::DoGrabLoop( LPVOID pParam )
{
  TRACE( "---------Entry to DoGrabLoop\n" ) ;
  IP_OnvifCam * pGadget = (IP_OnvifCam*) pParam ;
  //   Error error;
  FXString csMessage;
  BOOL isCorruptFrame = FALSE;
  unsigned int cols = 0 , rows = 0 , colsPrev = 0 , rowsPrev = 0;
  DWORD dwWaitRes = 0 ;
  pGadget->m_continueGrabThread = true ;
  pGadget->RegisterCallbacks() ;
  // Start of main grab loop
  while ( pGadget->m_continueGrabThread )
  {
    dwWaitRes = WaitForMultipleObjects( 2 , pGadget->m_WaitEventBusChangeArr , FALSE , (!(pGadget->m_bInitialized)) ? 500 : INFINITE ) ;
    if ( dwWaitRes == WAIT_FAILED )  // gadget deleted
    {
      DWORD dwError = GetLastError() ;
      break ;
    }
    if ( dwWaitRes == WAIT_OBJECT_0 ) // some bus or command event message
    {
      while ( pGadget->m_BusEvents )
      {
        if ( !pGadget->m_uiSerialNumber || (pGadget->m_uiSerialNumber == 0xffffffff) )
        {
          if ( !(pGadget->m_BusEvents & PGR_EVT_SHUTDOWN) )
            break ;
        }
        if ( !pGadget->m_bRun )
          pGadget->m_BusEvents &= ~(PGR_EVT_START_GRAB | PGR_EVT_LOCAL_START) ;
        FXAutolock al( pGadget->m_LocalConfigLock , "DoGrabLoop" ) ;
        if ( pGadget->m_BusEvents & PGR_EVT_SHUTDOWN )
        {
          pGadget->m_BusEvents = 0 ;
          pGadget->m_continueGrabThread = false ;
          //           pGadget->m_LocalConfigLock.Unlock() ;
          TRACE( "\nCamera #%u Shut downed" , pGadget->m_uiSerialNumber ) ;
          Sleep( 100 ) ;
          break ;
        }
        if ( pGadget->m_BusEvents & PGR_EVT_RESTART )
        {
          pGadget->m_BusEvents &= ~(PGR_EVT_RESTART) ;
          pGadget->IP_Onvif_CamClose() ;
          TRACE( "\nCamera #%u closed in DoGrabLoop" , pGadget->m_uiSerialNumber ) ;
          pGadget->m_BusEvents |= PGR_EVT_INIT ;
          break ;
        }
        if ( pGadget->m_BusEvents & (BUS_EVT_REMOVED | BUS_EVT_BUS_RESET) )
        {
          pGadget->IP_Onvif_CamClose() ;
          if ( pGadget->m_BusEvents & BUS_EVT_BUS_RESET )
          {
            pGadget->m_BusEvents |= PGR_EVT_INIT ;
            TRACE( "\nCamera #%u Bus Reset" , pGadget->m_uiSerialNumber ) ;
          }
          if ( pGadget->m_BusEvents & BUS_EVT_REMOVED )
            TRACE( "\nCamera #%u Removed" , pGadget->m_uiSerialNumber ) ;

          pGadget->m_BusEvents &= ~((BUS_EVT_REMOVED | BUS_EVT_BUS_RESET)) ;
        }
        if ( (pGadget->m_BusEvents & PGR_EVT_START_GRAB) && !pGadget->m_bInitialized )
        {
          pGadget->m_BusEvents |= PGR_EVT_INIT ;
        }
        if ( pGadget->m_BusEvents & PGR_EVT_INIT )
        {
          if ( pGadget->m_uiSerialNumber )
          {
            pGadget->IP_Onvif_CamInit() ;
            if ( pGadget->m_bInitialized  &&  pGadget->m_bRun )
              pGadget->m_BusEvents |= PGR_EVT_START_GRAB ;
          }
          pGadget->m_BusEvents &= ~(PGR_EVT_INIT) ;
        }
        if ( pGadget->m_BusEvents & PGR_EVT_START_GRAB )
        {
          if ( pGadget->m_uiSerialNumber )
          {
          }
          pGadget->m_BusEvents &= ~(PGR_EVT_START_GRAB) ;
        }
        if ( pGadget->m_BusEvents & PGR_EVT_STOP_GRAB )
        {
          pGadget->IP_Onvif_CameraStop() ;
          pGadget->m_BusEvents &= ~(PGR_EVT_STOP_GRAB) ;
        }
        if ( pGadget->m_BusEvents & PGR_EVT_RELEASE )
        {
          pGadget->IP_Onvif_CamClose() ;
          pGadget->m_CurrentCamera = -1 ;
          //pGadget->m_dwSerialNumber = 0 ;
          pGadget->m_BusEvents &= ~(PGR_EVT_RELEASE) ;
          break ;
        }
        if ( pGadget->m_BusEvents & PGR_EVT_LOCAL_STOP )
        {
          pGadget->IP_Onvif_LocalStreamStop() ;
          pGadget->m_BusEvents &= ~(PGR_EVT_LOCAL_STOP) ;
          break ;
        }
        if ( pGadget->m_BusEvents & PGR_EVT_LOCAL_START )
        {
          pGadget->IP_Onvif_LocalStreamStart() ;
          pGadget->m_BusEvents &= ~(PGR_EVT_LOCAL_START) ;
          break ;
        }
        if ( pGadget->m_BusEvents & BUS_EVT_ARRIVED )
        {
          pGadget->m_BusEvents &= ~(BUS_EVT_ARRIVED) ;
        }
      }
    }

    if ( pGadget->m_bRun  && pGadget->m_continueGrabThread )
    {
      int y = 0;
    }
  }
  pGadget->m_continueGrabThread = false ;
  pGadget->UnregisterCallbacks() ;

  //   if ( pGadget->m_pCamera )                        // Commented for Onvif
  pGadget->IP_Onvif_CamClose() ;
  pGadget->m_ControlThreadName.Empty() ;
  pGadget->m_dwGrabThreadId = NULL ;
  TRACE( "---PGR Normal Exit from DoGrabLoop for #%u\n" , pGadget->m_uiSerialNumber ) ;
  return 0;
}

void IP_OnvifCam::LogError( LPCTSTR Msg )
{
  FXString GadgetName ;
  GetGadgetName( GadgetName ) ;
  FxSendLogMsg( MSG_ERROR_LEVEL , GetDriverInfo() , 0 ,
    _T( "%s - %s" ) , (LPCTSTR) GadgetName , Msg );
}

void IP_OnvifCam::LocalStreamStart()
{
  IP_Onvif_DoAndWait( PGR_EVT_START_GRAB ) ;
  m_bLocalStopped = false ;
}
void IP_OnvifCam::LocalStreamStop()
{
  IP_Onvif_DoAndWait( PGR_EVT_STOP_GRAB ) ;
  m_bLocalStopped = true ;
}


void
IP_OnvifCam::RegisterCallbacks()
{
  //   Error error;

    // Register arrival callback
  //   error = m_BusManager.RegisterCallback( &IP_OnvifCam::OnBusArrival, ARRIVAL, this, &m_cbArrivalHandle );    
  //   if ( error != 0 )
  //   {
  //     SENDERR("Failed to register bus arrival callback: %s",
  //       error.GetDescription( ));
  //     m_cbArrivalHandle = NULL ;
  //   } 
  // 
  //   // Register removal callback
  //   error = m_BusManager.RegisterCallback( &IP_OnvifCam::OnBusRemoval, REMOVAL, this, &m_cbRemovalHandle );    
  //   if ( error != 0 )
  //   {
  //     SENDERR("Failed to register bus removal callback: %s",
  //       error.GetDescription( ));
  //     m_cbRemovalHandle = NULL ;
  //   } 
  // 
  //   // Register reset callback
  //   error = m_BusManager.RegisterCallback( &IP_OnvifCam::OnBusReset, BUS_RESET, this, &m_cbResetHandle );    
  //   if ( error != 0 )
  //   {
  //     SENDERR("Failed to register bus reset callback: %s",
  //       error.GetDescription( ));
  //     m_cbResetHandle = NULL ;
  //   } 
}

void IP_OnvifCam::UnregisterCallbacks()
{
  //   Error error;

    //   if ( m_cbArrivalHandle )
    //   {
    //     // Unregister arrival callback
    //     error = m_BusManager.UnregisterCallback( m_cbArrivalHandle );
    //     if ( error != 0 )
    //     {
    //       //ShowErrorMessageDialog( "Failed to unregister callback", error );     
    //     }
    //     m_cbArrivalHandle = NULL ;
    //   }

  //   if ( m_cbRemovalHandle )
  //   {
  //     // Unregister removal callback
  //     error = m_BusManager.UnregisterCallback( m_cbRemovalHandle );
  //     if ( error != 0 )
  //     {
  //       //ShowErrorMessageDialog( "Failed to unregister callback", error );     
  //     }   
  // 
  //     m_cbRemovalHandle = NULL ;
  //   }
  //   if ( m_cbResetHandle )
  //   {
  //     // Unregister reset callback
  //     error = m_BusManager.UnregisterCallback( m_cbResetHandle );
  //     if ( error != 0 )
  //     {
  //       //ShowErrorMessageDialog( "Failed to unregister callback", error );     
  //     }   
  //     m_cbResetHandle = NULL ;
  //   }
}

bool IP_OnvifCam::ScanProperties( LPCTSTR text , bool& Invalidate )
{
  //FXAutolock al( m_SettingsLock ) ;
  double dStart = GetHRTickCount() ;
  m_bInScanProperties = true ;
  m_bShouldBeReprogrammed = false ;

  if ( !m_continueGrabThread )
  {
    FXString ThreadName ;
    m_hGrabThreadHandle = CreateThread( NULL , 0 ,
      DoGrabLoop , this , CREATE_SUSPENDED , &m_dwGrabThreadId ) ;
    if ( m_hGrabThreadHandle )
    {
      ResumeThread( m_hGrabThreadHandle ) ;
      Sleep( 50 ) ;
      m_continueGrabThread = true ;
    }
    else
    {
      C1394_SENDERR_2( "%s: %s" , (LPCTSTR) ThreadName , _T( "Can't start thread" ) );
      m_bInScanProperties = false ;
      return false ;
    }
  }

  FXString tmpS;
  FXPropertyKit pc( text );
  unsigned camSN = 0 ;
  m_bWasStopped = false ;
  FXString fxcamSN;

  if ( pc.GetString( "Camera" , (FXString&) fxcamSN ) )
  {
    if ( fxcamSN.GetLength() > 5 )
      camSN = ip_to_hex( fxcamSN );
    if ( camSN && (camSN != 0xffffffff) )
    {
      unsigned newCamnmb = SerialToNmb( camSN );
      if ( newCamnmb < m_iCamNum )
      {
        if ( (m_uiSerialNumber != camSN) || (newCamnmb != m_CurrentCamera) )
        {
          m_bWasStopped = IsRunning() ;
          if ( m_uiSerialNumber && (m_uiSerialNumber != 0xffffffff) )
          {
            bool bRes = IP_Onvif_DoAndWait( PGR_EVT_RELEASE , 2000 ) ;
            m_uiSerialNumber = 0 ;
            m_uiConnectedSerialNumber = 0 ;
          }
          //           ASSERT( m_pCamera == NULL ) ;               // Commented for Onvif
          m_uiSerialNumber = camSN ;
          m_CurrentCamera = newCamnmb;
          if ( m_uiSerialNumber )
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
        //FXString fxCamNum ;
        //fxCamNum.Format("%d",camSN);
      //      m_dwSerialNumber = ip_to_hex(fxCamNum);

        m_uiSerialNumber = camSN;

        m_CurrentCamera = -1 ;
      }
      Invalidate |= true; //update setup
    }
    else
    {
      if ( m_uiSerialNumber && (m_uiSerialNumber != 0xffffffff) )
      {
        for ( int i = 0 ; i < m_BusyCameras.GetCount() ; i++ )
        {
          if ( m_BusyCameras[ i ] == m_uiSerialNumber )
          {
            m_BusyCameras.RemoveAt( i ) ;
          }
        }
        bool bRes = IP_Onvif_DoAndWait( PGR_EVT_RELEASE , 2000 ) ;
        m_uiSerialNumber = 0 ;
        m_uiConnectedSerialNumber = 0 ;
        Invalidate = true ;
      }
      //       ASSERT( bRes ) ;
    }
  }
  if ( DriverValid() /*&& m_pCamera*/ )                   // Commented for Onvif
  {
    if ( pc.GetInt( "StreamState" , m_bLocalStopped ) )
    {
      if ( m_bLocalStopped )
        LocalStreamStop() ;
      else
        LocalStreamStart() ;
    };
    for ( int i = 0; i < m_Properties.GetSize(); i++ )
    {
      FXString key;
      FXParser param;
      m_Properties[ i ].property.GetElementNo( 0 , key , param );
      if ( (key == SETUP_COMBOBOX) || (key == SETUP_SPIN) ) // ints result
      {
        int value; 
        bool bauto = false;
        if ( pc.GetInt( m_Properties[ i ].name , value ) )
        {
          FXSIZE Tmp = value ;
          if ( !SetCameraProperty( m_Properties[ i ].id , Tmp , bauto , Invalidate ) )
          {
            SENDERR( "Can't set property %s" , m_Properties[ i ].name );
          }
        }
      }
      else if ( key == SETUP_SPINABOOL )
      {
        FXString tmpS; 
        FXSIZE value; 
        bool bauto = false;
        if ( pc.GetString( m_Properties[ i ].name , tmpS ) )
        {
          bauto = tmpS.CompareNoCase( "auto" ) == 0;
          if ( bauto )
          {
            GetCameraProperty( m_Properties[ i ].id , value , bauto );
            bauto = true;
          }
          else
            value = atoi( tmpS );
          if ( !SetCameraProperty( m_Properties[ i ].id , value , bauto , Invalidate ) )
          {
            SENDERR( "Can't set property %s" , m_Properties[ i ].name );
          }
        }
      }
      else if ( key == SETUP_EDITBOX )
      {
        FXString svalue;
        FXSIZE value; bool bauto = false;
        if ( pc.GetString( m_Properties[ i ].name , svalue ) )
        {
          value = (FXSIZE) ((LPCTSTR) svalue);
          if ( !SetCameraProperty( m_Properties[ i ].id , value , bauto , Invalidate ) )
          {
            C1394_SENDERR_2( "Can't set prop %s to %s" ,
              m_Properties[ i ].name , svalue );
          }
        }
      }
      else
      {
        SENDERR( "Unsupported key '%s'in scanproperty" , key );
      }
    }
  }
  if ( m_bShouldBeReprogrammed )
  {
    if ( m_bWasStopped )
      CameraStart() ;
    else
      IP_Onvif_DoAndWait( PGR_EVT_INIT , 2000 ) ;
  }
  m_bInScanProperties = false ;
  FXString GadgetName ;
  GetGadgetName( GadgetName ) ;
  double dBusyTime = GetHRTickCount() - dStart ;
  TRACE( "\nIP_Onvif::ScanProperties %s: Start %g , Busy %g" , (LPCTSTR) GadgetName ,
    dStart , dBusyTime ) ;
  return true;
}

void IP_OnvifCam::OnBusArrival( void* pParam , unsigned int serialNumber )
{
  IP_OnvifCam* pGadget = static_cast<IP_OnvifCam*>(pParam);
  //   pGadget->m_BusEvents |= BUS_EVT_ARRIVED ;
  //   SetEvent( pGadget->m_evBusChange ) ;
  TRACE( "\nBus Arrival for Camera #%u " , pGadget->m_uiSerialNumber ) ;
  pGadget->m_bRescanCameras = true ;
  pGadget->m_dwNArrivedEvents++ ;
}

void IP_OnvifCam::OnBusRemoval( void* pParam , unsigned int serialNumber )
{
  IP_OnvifCam* pGadget = static_cast<IP_OnvifCam*>(pParam);
  if ( serialNumber == pGadget->m_CamerasInfo[ pGadget->m_CurrentCamera ].serialnmb )
  {
    pGadget->m_BusEvents |= BUS_EVT_REMOVED ;
    SetEvent( pGadget->m_evBusChange ) ;
    FxSendLogMsg( MSG_WARNING_LEVEL ,
      pGadget->GetDriverInfo() , 0 , "Camera %u disconnected" , serialNumber ) ;
    TRACE( "\nCamera #%u Removed\n" , pGadget->m_uiSerialNumber ) ;
  }
  pGadget->m_bCamerasEnumerated = false ;
  pGadget->m_bRescanCameras = true ;
}


void IP_OnvifCam::OnBusReset( void* pParam , unsigned int serialNumber )
{
  IP_OnvifCam* pGadget = static_cast<IP_OnvifCam*>(pParam);
  pGadget->m_bCamerasEnumerated = false ;
  //   bool bRes = pGadget->IP_Onvif_DoAndWait( BUS_EVT_BUS_RESET | PGR_EVT_INIT , 2000)  ;
  pGadget->m_bRescanCameras = true ;
  TRACE( "\nBus Reset for Camera #%u \n" , pGadget->m_uiSerialNumber ) ;
  //   ASSERT( bRes ) ;
}



bool IP_OnvifCam::CheckAndAllocCamera( void )
{
  //   if ( !m_pCamera )                         // Commented for Onvif
  //   {
  //     if ( !m_dwSerialNumber )
  //       return false ;
  //     if ( !CameraInit() )
  //       return false ;
  //   }
  if (/* !m_pGUICamera  ||*/  m_bGUIReconnect )                 // Commented for Onvif
  {
    FXAutolock al( m_LocalConfigLock , "CheckAndAllocCamera" ) ;

    //     if ( m_pGUICamera )                                        // Commented for Onvif
    //     {
    //       m_pGUICamera->Disconnect() ;
    //       m_pGUICamera = NULL ;
    //     }
    //     m_LastError = m_BusManager.GetCameraFromSerialNumber(
    //       m_dwSerialNumber , &m_CurrentCameraGUID);
    //     if (m_LastError!=0)
    //     {
    //       if ( ++m_iNNoSerNumberErrors <= 2 )
    //         SENDERR("Fatal error GetCameraFromSerialNumber: %s",
    //         m_LastError.GetDescription( ));
    //       return false;
    //     }
    //     m_LastError=m_BusManager.GetInterfaceTypeFromGuid( &m_CurrentCameraGUID, &m_IntfType );
    //     if ( m_LastError != 0 )
    //     {   
    //       SENDERR("Fatal error GetInterfaceTypeFromGuid: %s",
    //         m_LastError.GetDescription( ));
    //       return FALSE;
    //     }    
    // 
    //     m_Status.Empty() ;
    // 
    //     if ( m_IntfType == INTERFACE_GIGE )
    //       m_pGUICamera = new GigECamera ;   // GIGE interface
    //     else
    //       m_pGUICamera = new Camera ; // USB or 1394 interface

    m_bGUIReconnect = false ;
    //     if ( m_pGUICamera )
    //     {
    //       m_LastError=m_pGUICamera->Connect(&m_CurrentCameraGUID);
    //       if (m_LastError!=0)
    //       {
    //         delete m_pGUICamera ;
    //         m_pGUICamera = NULL ;
    //         SENDERR("GUI Connect error: %s",m_LastError.GetDescription( ));
    //         return false;
    //       }
    //     }
  }
  return true ;
}




bool IP_OnvifCam::ScanSettings( FXString& text )
{
  EnumCameras() ;
  CheckAndAllocCamera() ;
  // Prepare cameras list
  FXString camlist( "Not Selected(-1)," ) , paramlist , tmpS;
  //  FXAutolock al( m_ConfigLock ) ;
  FXAutolock al( m_SettingsLock ) ;
  int iCurrentCamera = -1 ;
  for ( unsigned i = 0; i < m_iCamNum; i++ )
  {
    TCHAR cMark = _T( '+' ) ; // sign, that camera is free
    if ( m_uiSerialNumber != m_CamInfo[ i ].nserialnmb )
    {
      for ( int j = 0 ; j < m_BusyCameras.GetCount() ; j++ )
      {
        if ( m_uiSerialNumber != m_BusyCameras[ j ] )
        {
          if ( m_CamInfo[ i ].nserialnmb == m_BusyCameras[ j ] )
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
      iCurrentCamera = m_CurrentCamera = i ;
    }
    //tmpS.Format("%c%d:%s(%d)", cMark , m_CamInfo[i].nserialnmb , (LPCTSTR)m_CamInfo[i].szDevName , m_CamInfo[i].nserialnmb );
    tmpS.Format( "%c%s(%s)" , cMark , m_CamInfo[ i ].szIpaddress /*, (LPCTSTR)m_CamInfo[i].szDevName*/ , m_CamInfo[ i ].szIpaddress );
    camlist += tmpS;
    if ( i < m_iCamNum - 1 )
      camlist += _T( ',' ) ;
  }
  if ( iCurrentCamera < 0 && m_uiSerialNumber && (m_uiSerialNumber != 0xffffffff) )
  {
    if ( !camlist.IsEmpty() )
      camlist += _T( ',' ) ;
    tmpS.Format( "?%d(%d)" , m_uiSerialNumber , m_uiSerialNumber ) ;
    camlist += tmpS ;
    iCurrentCamera = m_iCamNum ; // first after real cameras
  }
  tmpS.Format( "ComboBox(Camera(%s))" , camlist );
  paramlist += tmpS;
  /*
  if ( m_dwSerialNumber == m_dwConnectedSerialNumber )
  {
    paramlist += _T(',') ;
    BuildPropertyList() ;
    tmpS.Format("ComboBox(StreamState(Run(0),Idle(1))),");
    paramlist+=tmpS;
    for (int i=0; i<m_Properties.GetSize(); i++)
    {
      paramlist+=m_Properties[i].property ;
      if ( i < m_Properties.GetUpperBound() )
        paramlist += _T(',') ;
    }
  }
  */
  text.Format( "template(%s)" , paramlist );
  return true;
}

bool IP_OnvifCam::PrintProperties( FXString& text )
{
  FXPropertyKit pc;
  if ( DriverValid() && (m_uiSerialNumber != 0)
    && (m_uiSerialNumber != -1) )
  {
    //pc.WriteInt("Camera", m_uiSerialNumber );
    pc.WriteString( "Camera" , hex_to_ip( m_uiSerialNumber ) );
    pc.WriteInt( "StreamState" , m_bLocalStopped ) ;
    if ( m_uiConnectedSerialNumber != 0 )
    {
      for ( int i = 0; i < m_Properties.GetSize(); i++ )
      {
        FXSIZE value; 
        bool bauto;
        if ( GetCameraProperty( m_Properties[ i ].id , value , bauto ) )
        {
          FXString key;
          FXParser param;
          m_Properties[ i ].property.GetElementNo( 0 , key , param );
          if ( (key == SETUP_COMBOBOX) || (key == SETUP_SPIN) ) // ints result
          {
            pc.WriteInt( m_Properties[ i ].name , (int)value );
          }
          else if ( key == SETUP_SPINABOOL )
          {
            FXString tmpS;
            if ( bauto )
              tmpS = "auto";
            else
              tmpS.Format( "%d" , value );
            pc.WriteString( m_Properties[ i ].name , tmpS );
          }
          else if ( key == SETUP_EDITBOX )
          {
            FXString svalue = (LPCTSTR) value;
            pc.WriteString( m_Properties[ i ].name , svalue );
          }
          else
          {
            SENDERR( "Unsupported key '%s'in scanproperty" , key );
          }
        }
      }
    }
  }
  else
  {
    pc.WriteInt( "Camera" , -1 );
  }
  CCaptureGadget::PrintProperties( pc ) ;
  text = pc;
  return true;
}
void IP_OnvifCam::ProcessNextInputStreamChunk( CDataFrame * pFrame )
{

}

