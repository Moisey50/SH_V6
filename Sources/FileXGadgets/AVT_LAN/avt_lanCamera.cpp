// avt_lanCamera.cpp: implementation of the avt_lan class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "fxfc\fxstrings.h"
#include "avt_lan.h"
#include "avt_lanCamera.h"
#include <video\yuv9.h>
#include <math\intf_sup.h>
#include <gadgets\stdsetup.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define THIS_MODULENAME "avt_lanCamera"
#define MARLINCAMERAMAXNAMELENGTH 128

#pragma comment( lib, "PvAPI.lib" )

IMPLEMENT_RUNTIME_GADGET_EX(avt_lan, C1394Camera, "Video.capture", TVDB400_PLUGIN_NAME);

#define LENARR(x) (sizeof(x)/sizeof(x[0]))
#define N_FRAMES_FOR_ALLOCATE (10)

enum FG_COLORMODE  // declared for compatibility with AVT FireGrab
{
  CM_UNKNOWN = -1 ,
  CM_Y8=0,
  CM_YUV411,
  CM_YUV422,
  CM_YUV444,
  CM_RGB8,
  CM_Y16,
  CM_RGB16,
  CM_SY16,
  CM_SRGB16,
  CM_RAW8,
  CM_RAW16,
  CM_LAST
};


typedef struct tagVideoformats
{
  tPvImageFormat      vm;
  const char *        name; 
  bool                supported;
  double              dBytesPerPixel ;
  FG_COLORMODE        AVT1394_ColorMode ;
  unsigned            BMP_OutCompression ;

} Videoformats;

typedef CArray<tPvAttributeInfo> AvailAttrib ;

Videoformats vFormats[]=
{
  {ePvFmtMono8,"Mono8",true , 1. , CM_Y8 , BI_Y8 },
  {ePvFmtYuv411,"YUV411", true , 1.5 , CM_YUV411 , BI_YUV9 },
  {ePvFmtYuv422, "YUV422", false , 2.0 , CM_YUV422 , BI_YUV12 },
  {ePvFmtYuv444, "YUV444",false , 3.0 , CM_YUV444 ,BI_YUV12 },
  {ePvFmtRgb24, "RGB24",false , 3.0 , CM_UNKNOWN , BI_YUV12 },
  {ePvFmtMono16,"Mono16",true , 2.0 , CM_Y16 , BI_Y16 },
  {ePvFmtRgb48,"RGB48",false , 6.0 , CM_UNKNOWN , BI_YUV12 },
  {ePvFmtBgr24, "BGR24",false , 3.0 , CM_UNKNOWN , BI_YUV12 },
  {ePvFmtRgba32, "RGBA32",false , 4.0 , CM_UNKNOWN , BI_YUV12 },
  {ePvFmtBgra32, "BGRA32",false , 4.0 , CM_UNKNOWN , BI_YUV12},
  {ePvFmtMono12Packed, "Mono12Packed",true , 1.5, CM_UNKNOWN , BI_Y16 },
  {ePvFmtBayer12Packed, "Bayer12Packed",false , 1.5 , CM_UNKNOWN , BI_Y16 },
  {ePvFmtBayer8, "RAW8",true , 1.0 , CM_RAW8 , BI_Y8 },
  {ePvFmtBayer16, "RAW16",true , 2.0 , CM_RAW16 , BI_Y16 },
  {ePvFmtBayer8, "Bayer8",true , 1.0 , CM_RAW8 , BI_Y8 },
  {ePvFmtBayer16, "Bayer16",true , 1.0 , CM_RAW16 , BI_Y16 },
};


#define FGP_FRAME_RATE  ((FG_PARAMETER)(FGP_IMAGEFORMAT-13))
#define FGP_RESOLUTION ((FG_PARAMETER)(FGP_IMAGEFORMAT-2))
#define FGP_ROI        ((FG_PARAMETER)(FGP_IMAGEFORMAT-3))
#define FGP_TRIGGERONOFF ((FG_PARAMETER)(FGP_IMAGEFORMAT-4))
#define FGP_EXTSHUTTER ((FG_PARAMETER)(FGP_IMAGEFORMAT-5))
#define FGP_SEND_FINFO ((FG_PARAMETER)(FGP_IMAGEFORMAT-6))
#define FGP_TRIGGERDELAY ((FG_PARAMETER)(FGP_IMAGEFORMAT-7))
#define FGP_GRAB       ((FG_PARAMETER)(FGP_IMAGEFORMAT-8))
#define FGP_PACKET_SIZE ((FG_PARAMETER)(FGP_IMAGEFORMAT-9))                               // Packet size
#define FGP_TRIG_EVENT ((FG_PARAMETER)(FGP_IMAGEFORMAT-10)) 
#define FGP_ACQU_MODE ((FG_PARAMETER)(FGP_IMAGEFORMAT-11)) 
#define FGP_BANDWIDTH ((FG_PARAMETER)(FGP_IMAGEFORMAT-12)) 

#define FGP_EXP_AUTO_OUT ((FG_PARAMETER)(FGP_IMAGEFORMAT-14))
#define FGP_EXP_AUTO_ALG ((FG_PARAMETER)(FGP_IMAGEFORMAT-15))
#define FGP_GAIN_AUTO_TARG ((FG_PARAMETER)(FGP_IMAGEFORMAT-16))
#define FGP_GAIN_AUTO_OUT ((FG_PARAMETER)(FGP_IMAGEFORMAT-17))
#define FGP_GAIN_PV ((FG_PARAMETER)(FGP_IMAGEFORMAT-18))

typedef struct tagCamProperty
{
  FG_PARAMETER        pr;
  const char *        name;
  const char *        CamPropertyName ;
  const char *        AutoControl ;
  CameraAttribute *   pInCamera ;
  bool                m_bSupported ;
} CamProperty;


CamProperty cProperties[] =
{
  {FGP_IMAGEFORMAT, "Format" , "PixelFormat" , NULL  , NULL , true },
  {FGP_ACQU_MODE, "AcquMode" , "AcquisitionMode" , NULL , NULL , true },
  {FGP_FRAME_RATE, "FrameRate" , "FrameRate" , NULL  , NULL },
  {FGP_WHITEBALCB,"HWWhitebalanceB" , "WhitebalValueBlue" , "WhitebalMode Manual-Auto"  , NULL , true },
  {FGP_WHITEBALCR,"HWWhitebalanceR" , "WhitebalValueRed" , NULL  , NULL , true },
  {FGP_GAMMA , "Gamma" , NULL , NULL  , NULL , true },
  {FGP_IRIS , "Iris" , NULL , NULL  , NULL , true },
  {FGP_FOCUS,"Focus" , NULL , NULL  , NULL , true },
  {FGP_ZOOM,"Zoom" , NULL , NULL  , NULL, true  },
  {FGP_PAN,"Pan" , NULL , NULL  , NULL , true },
  {FGP_TILT,"Tilt" , NULL , NULL  , NULL , true },
  {FGP_EXTSHUTTER, "Shutt_uS" , "ExposureValue" , "ExposureMode Manual-Auto"  , NULL , true },
  {FGP_AUTOEXPOSURE , "ExpTarget" , "ExposureAutoTarget" , NULL  , NULL , true },
  {FGP_GAIN_PV , "Gain" , "GainValue" , "GainMode Manual-Auto" , NULL , true  },
  {FGP_ROI , "ROI" , NULL , NULL  , NULL, true  },
  {FGP_TRIGGERONOFF, "Trigger" , "FrameStartTriggerMode" , NULL , NULL  , true },
  {FGP_TRIG_EVENT , "TrigEvent" , "FrameStartTriggerEvent" , NULL , NULL , true },
  {FGP_TRIGGERDELAY , "TrigDelay_uS" , "FrameStartTriggerDelay" , NULL  , NULL, true  } ,
  {FGP_GRAB , "Grab" , "AcquisitionFrameCount" , NULL  , NULL , true } ,
  {FGP_BANDWIDTH , "LAN_Capacity" , "StreamBytesPerSecond" , NULL , NULL , true  } ,
  {FGP_EXP_AUTO_OUT , "EXP_Outliers" , "ExposureAutoOutliers" , NULL , NULL , true  } ,
  {FGP_EXP_AUTO_ALG , "EXP_Algorithm" , "ExposureAutoAlg" , NULL , NULL , true  } ,
  {FGP_GAIN_AUTO_TARG , "GAIN_Target" , "GainAutoTarget" , NULL , NULL , true  } ,
  {FGP_GAIN_AUTO_OUT , "GAIN_Outliers" , "GainAutoOutliers" , NULL , NULL , true  } ,
};


FXLockObject avt_lan::m_avtLANInitLock;
long avt_lan::m_lAvtLanInstCntr=0;
bool avt_lan::m_bAvtLanDriverReady=false;
bool avt_lan::m_bCamLANStatusChanged = false ;

static LPCTSTR GetErrorMessage(UINT32 errCode)
{
  static TCHAR buf[100] ;
  switch (errCode)
  {
  case ePvErrSuccess:       return "No Error";
  case ePvErrCameraFault:   return "Unexpected camera fault";
  case ePvErrInternalFault: return "Unexpected fault in PvApi or driver";
  case ePvErrBadHandle:     return "Camera handle is invalid";
  case ePvErrBadParameter : return "Bad parameter to API call";
  case ePvErrBadSequence:   return "Sequence of API calls is incorrect";
  case ePvErrNotFound:      return "Camera or attribute not found";
  case ePvErrAccessDenied:  return "Camera cannot be opened in the specified mode";
  case ePvErrUnplugged:     return "Camera was unplugged";
  case ePvErrInvalidSetup:  return "Setup is invalid (an attribute is invalid)";
  case ePvErrResources    : return "System/network resources or memory not available";
  case ePvErrBandwidth:     return "1394 bandwidth not available";
  case ePvErrQueueFull:     return "Too many frames on queue";
  case ePvErrBufferTooSmall:return "Frame buffer is too small";
  case ePvErrCancelled:     return "Frame canceled by user";
  case ePvErrDataLost:      return "The data for the frame was lost";
  case ePvErrDataMissing:   return "Some data in the frame is missing";
  case ePvErrTimeout:       return "Timeout during wait";
  case ePvErrOutOfRange:    return "Attribute value is out of the expected range";
  case ePvErrWrongType:     return "Attribute is not this type (wrong access function)";
  case ePvErrForbidden:     return "Attribute write forbidden at this time";
  case ePvErrUnavailable:   return "Attribute is not available at this time";
  case ePvErrFirewall:      return "A firewall is blocking the traffic (Windows only)";
  case __ePvErr_force_32:   return "Unknown error -1";
  }
  _stprintf_s( buf , _T("Unknown error %d") , errCode ) ;
  return buf ;
}



__forceinline int GetFormatId(tPvImageFormat format)
{
  for (int i=0; i<(sizeof(vFormats)/sizeof(Videoformats)); i++)
  {
    if (format==vFormats[i].vm) 
      return i;
  }
  return -1;
}

__forceinline tPvImageFormat GetFormatId(char * format)
{
  FXString Format , Pattern ;
  for (int i=0; i<(sizeof(vFormats)/sizeof(Videoformats)); i++)
  {
    Format = format ;
    Pattern = vFormats[i].name ;

    if ( !Format.CompareNoCase( Pattern ) )
      return vFormats[i].vm;
  }
  return (tPvImageFormat) -1;
}

CamProperty * GetPropertyByName( FXString& Name )
{
  for ( int i = 0 ; i < LENARR(cProperties) ; i++ )
  {
    if ( Name == cProperties[i].name )
      return &cProperties[i] ;
  }
  ASSERT(0) ;
  return 0 ;
}

CamProperty * GetPropertyByType( FG_PARAMETER Type )
{
  for ( int i = 0 ; i < LENARR(cProperties) ; i++ )
  {
    if ( Type == cProperties[i].pr )
      return &cProperties[i] ;
  }
  ASSERT(0) ;
  return 0 ;
}

// Frame completed callback executes on separate driver thread.
// One callback thread per camera. If a frame callback function has not 
// completed, and the next frame returns, the next frame's callback function is queued. 
// This situation is best avoided (camera running faster than host can process frames). 
// Spend as little time in this thread as possible and offload processing
// to other threads or save processing until later.
//
// Note: If a camera is unplugged, this callback will not get called until PvCaptureQueueClear.
// i.e. callback with pFrame->Status = ePvErrUnplugged doesn't happen -- so don't rely
// on this as a test for a missing camera. 
void __stdcall FrameDoneCB(tPvFrame* pFrame)  
{
  avt_lan * pCamClass = (avt_lan *)pFrame->Context[0] ;

  //Do something with the frame.
  //E.g. display to screen, shift pFrame->ImageBuffer location for later usage , etc
  //Here we display FrameCount and Status
  if (pFrame->Status == ePvErrSuccess)
  {
    //TRACE("Frame: %u returned successfully\n", pFrame->FrameCount);
    pCamClass->m_Lock.Lock() ;
    pCamClass->m_Readyframes.AddTail( pFrame ) ;
    pCamClass->m_Lock.Unlock() ;
    SetEvent( pCamClass->m_hFrameReady ) ;
    return ;
  }
  else if (pFrame->Status == ePvErrDataMissing)
    //Possible improper network card settings. See GigE Installation Guide.
    TRACE("Frame: %u dropped packets\n", pFrame->FrameCount);
  else if (pFrame->Status == ePvErrCancelled)
    TRACE("Frame cancelled %u\n", pFrame->FrameCount);
  else
    TRACE("Frame: %u Error: %u\n", pFrame->FrameCount, pFrame->Status);

  if ( pCamClass->m_Handle && pCamClass->m_bRun )
  {
    // if frame hasn't been cancelled, requeue frame
    if(pFrame->Status != ePvErrCancelled)
    {
      tPvErr Error = PvCaptureQueueFrame( pCamClass->m_Handle , pFrame , FrameDoneCB ) ;
      if ( Error == ePvErrSuccess )
        return ;
      SENDERR_1("Error: Can't requeue frame %s", 
        GetErrorMessage(Error));
    }
    else if ( pCamClass->ReallocFrame( pFrame , pCamClass->m_ulImageSize ) )
    {
      tPvErr Error = PvCaptureQueueFrame( pCamClass->m_Handle , pFrame , FrameDoneCB ) ;
      if ( Error == ePvErrSuccess )
        return ;
      SENDERR_1("Error: Can't requeue frame %s", GetErrorMessage(Error));
    }
  }
  pCamClass->m_Lock.Lock() ;
  pCamClass->m_FreeFrames.AddTail( pFrame ) ;
  pCamClass->m_Lock.Unlock() ;
  TRACE("Frame %u Added to free list (queue clear)\n", pFrame->FrameCount);
}

// callback called when the camera is plugged/unplugged
void __stdcall CameraLinkEventCB(void* Context,
                                 tPvInterface Interface,
                                 tPvLinkEvent Event,
                                 unsigned long UniqueId)
{
  avt_lan * pCamClass = (avt_lan*)Context ;

  switch(Event)
  {
  case ePvLinkAdd:
    {
      {
        TRACE("camera %lu plugged\n",UniqueId);
        int i = 0 ;
        pCamClass->m_Lock.Lock() ;
        for ( ; i < pCamClass->m_AvailableCameras.GetCount() ; i++ )
        {
          if ( pCamClass->m_AvailableCameras[i] == UniqueId )
          {
            ASSERT(0) ;
            break ;
          }
        }
        if ( i >= pCamClass->m_AvailableCameras.GetCount() )
          pCamClass->m_AvailableCameras.Add( UniqueId ) ;
        pCamClass->m_Lock.Unlock() ;
        pCamClass->m_bCamLANStatusChanged = true ;
      }
      break;
    }
  case ePvLinkRemove:
    {
      TRACE("camera %lu unplugged\n",UniqueId);
      int i = 0 ;
      pCamClass->m_Lock.Lock() ;
      for ( ; i < pCamClass->m_AvailableCameras.GetCount() ; i++ )
      {
        if ( pCamClass->m_AvailableCameras[i] == UniqueId )
        {
          pCamClass->m_AvailableCameras.RemoveAt( i ) ;
          i-- ;
          break ;
        }
      }
      pCamClass->m_Lock.Unlock() ;
      if ( i >= pCamClass->m_AvailableCameras.GetCount()  )
        ASSERT(0) ;
      pCamClass->m_bCamLANStatusChanged = true ;
      break;
    }
  default:
    break;
  }
}

void __stdcall CameraEventCB(void* Context,
                             tPvHandle hCamera,
                             const tPvCameraEvent* EventList,
                             unsigned long EventListLength)
{
  avt_lan * pCamClass = (avt_lan*)Context ;
  for ( ULONG i = 0 ; i < EventListLength ; i++ )
  {
    switch( EventList[i].EventId )
    {
    case EventAcquisitionEnd:
      {
        pCamClass->m_bAcqusitionStoped = true ; 
        break ;
      }
    default: break ;
    }
  }
}

__forceinline unsigned GetXSize(tPvHandle CamHandle)
{
  tPvUint32 lValue;

  if(PvAttrUint32Get(CamHandle,"Width",&lValue) == ePvErrSuccess)
    return (unsigned) lValue;
  return 0;
}

__forceinline unsigned GetYSize(tPvHandle CamHandle)
{
  tPvUint32 lValue;

  if(PvAttrUint32Get(CamHandle,"Height",&lValue) == ePvErrSuccess)
    return (unsigned) lValue;
  return 0;
}

__forceinline unsigned GetXOffset(tPvHandle CamHandle)
{
  tPvUint32 lValue;

  if(PvAttrUint32Get(CamHandle,"RegionX",&lValue) == ePvErrSuccess)
    return (unsigned) lValue;
  return 0;
}

__forceinline unsigned GetYOffset(tPvHandle CamHandle)
{
  tPvUint32 lValue;

  if(PvAttrUint32Get(CamHandle,"RegionY",&lValue) == ePvErrSuccess)
    return (unsigned) lValue;
  return 0;
}

__forceinline unsigned GetMaxWidth( tPvHandle CamHandle)
{
  tPvUint32 lValue;

  if(PvAttrUint32Get(CamHandle,"SensorWidth",&lValue) == ePvErrSuccess)
    return (unsigned) lValue;
  return 0;
}

__forceinline unsigned GetMaxHeight(tPvHandle CamHandle)
{
  tPvUint32 lValue;

  if( PvAttrUint32Get(CamHandle,"SensorHeight",&lValue) == ePvErrSuccess )
    return (unsigned) lValue;
  return 0;
}

__forceinline bool IsFormatSupported(tPvHandle CamHandle, char * pFormat )
{
  char buf[2000] ;
  ULONG iAnswerLen = 0 ;
  if( PvAttrRangeEnum( CamHandle , "PixelFormat" , buf , sizeof(buf) , &iAnswerLen ) 
    == ePvErrSuccess 
    && (iAnswerLen != 0) )
  {
    FXParser Parser( buf ) ;
    int iPos = 0 ;
    FXString wrd ;
    while ( Parser.GetWord( iPos , wrd ) )
    {
      if ( ! wrd.CompareNoCase( pFormat ) )
      {
        for ( int i = 0 ; i < LENARR(vFormats) ; i++ )
        {
          if ( ! wrd.CompareNoCase( vFormats[i].name ) ) // format is supported by camera
            return vFormats[i].supported ;
        }
      }
    }
  }
  return false;
}

__forceinline bool IsFormatSupported(tPvHandle CamHandle, tPvImageFormat Format )
{
  for ( int i = 0 ; i < LENARR(vFormats) ; i++ )
  {
    if ( vFormats[i].vm == Format )
    {
      if ( ! vFormats[i].supported )
        return false ; // gadget doesn't support this format
      else // gadget supports format, lets check camera for support
      {
        char buf[2000] ;
        ULONG iAnswerLen = 0 ;
        if( PvAttrRangeEnum( CamHandle , "PixelFormat" , buf , 
          sizeof(buf) , &iAnswerLen ) == ePvErrSuccess 
          && (iAnswerLen != 0) )
        {
          FXParser Parser( buf ) ;
          int iPos = 0 ;
          FXString wrd ;
          while ( Parser.GetWord( iPos , wrd ) )
          {
            if ( ! wrd.CompareNoCase( vFormats[i].name ) )
              return true ; // OK camera also supports
          }
        }
      }
    }
  }
  return false ; // format is not found in gadget table
}

__forceinline bool GetAutoControlPropName( const char * pFullName , FXString& Name )
{
  if ( pFullName )
  {
    char * pSpacepos = strchr( (char*) pFullName , ' ' ) ;
    if ( pSpacepos )
    {
      int iLen = pSpacepos - pFullName ;
      Name = FXString(pFullName).Left(iLen) ;
      return true ;
    }
  }
  return false ;
}

bool avt_lan::Init()
{
  FXAutolock al(m_avtLANInitLock);
  LONG res;
  InterlockedExchange(&res,m_lAvtLanInstCntr);
  if (res==0)
  {
    m_bCamLANStatusChanged = false ;
    tPvErr Result = PvInitialize() ;
    if( Result != ePvErrSuccess)
    {
      TRACE("!!! PvInitialize error\n");
      SENDERR_1("Error: Avt_LAN Driver Initialization error %s", 
        GetErrorMessage(Result));
      m_bAvtLanDriverReady = false ;
    }
    else
    {
      Result = PvLinkCallbackRegister(CameraLinkEventCB,ePvLinkAdd,this);
      if( Result != ePvErrSuccess)
      {
        TRACE("!!! Call back register error\n");
        SENDERR_1("Error: Call back register %s", 
          GetErrorMessage(Result));
      }
      Result = PvLinkCallbackRegister(CameraLinkEventCB,ePvLinkRemove,this);
      if( Result != ePvErrSuccess)
      {
        TRACE("!!! PvInitialize error\n");
        SENDERR_1("Error:  Call back register %s", 
          GetErrorMessage(Result));
      }
      InterlockedIncrement(&m_lAvtLanInstCntr);
      m_bAvtLanDriverReady = true ;
    }
  }
  else
    InterlockedIncrement(&m_lAvtLanInstCntr);
  return m_bAvtLanDriverReady ;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

avt_lan::avt_lan():
  FrameInfoOn(false)
  , m_iAllocatedFrames(0)
  , m_Handle(NULL)
  , m_SensorSize( CSize(0,0) )
  , m_hFrameReady( NULL )
{
  Init();
  memset(&m_BMIH,0,sizeof(BITMAPINFOHEADER));
}

int avt_lan::DiscoverCameras( double dTimeout )
{
  m_CamerasOnBus = m_AvailableCameras.GetCount() ;
  ULONG ulNInfos = 0 ;
  double dStartTime = GetHRTickCount() ;
  while ( ulNInfos == 0   &&  (GetHRTickCount() - dStartTime) < dTimeout )
  {
    ulNInfos = PvCameraListEx( 
      m_nodesinfo , MAX_CAMERASNMB , (ULONG*) &m_CamerasOnBus , 
      sizeof(tPvCameraInfoEx) ) ;
  }

  CAMERA1394::CameraInfo Info ;
  if ( m_CurrentCamera != -1 ) 
    Info = m_CamerasInfo[m_CurrentCamera] ;

  for ( ULONG i = 0 ; i < ulNInfos ; i++ )
  {
    m_CamerasInfo[i].name = m_nodesinfo[i].CameraName ;
    m_CamerasInfo[i].serialnmb = m_nodesinfo[i].UniqueId ;
    if ( m_CamerasInfo[i].serialnmb == Info.serialnmb )
      m_CurrentCamera = i ;
  }
  m_bCamLANStatusChanged = false ;
  return m_CamerasOnBus ;
}

bool avt_lan::DriverInit()
{
  if (!m_bAvtLanDriverReady)
  {
    if ( !Init() )
      return false;
  }

  if ( !DiscoverCameras(5000.) )
    return false;

  if ( !m_hFrameReady)
    m_hFrameReady = CreateEvent( NULL , FALSE , FALSE , NULL ) ;
  return (m_bAvtLanDriverReady && (m_CamerasOnBus!=0));
}

bool avt_lan::CameraInit()
{
  if (m_CamerasOnBus==0) 
  {
    SENDERR_0("Error: No AVT_LAN Cameras found on a bus");
    return false;
  }
  ASSERT(m_CurrentCamera!=-1);
  if ( CameraConnected() )
    CameraClose();
  m_LastError = PvCameraOpen( m_nodesinfo[m_CurrentCamera].UniqueId ,
    ePvAccessMaster , &m_Handle ) ;
  if ( m_LastError != ePvErrSuccess )
  {
    SENDERR_1("Error: AVT_LAN Open Camera error %s", GetErrorMessage(m_LastError) );
    return false;
  }
  m_CameraID.Format( "%s_%u" , m_nodesinfo[m_CurrentCamera].CameraName ,
    m_nodesinfo[m_CurrentCamera].UniqueId );

#ifdef _DEBUG
  SetAttrULONG( "HeartbeatTimeout" , 900000 ) ;
#endif

  tPvUint32 MaxSize = 1504;

  // then we adjust the packet size (max is returned in MaxSize value)
  m_LastError = PvCaptureAdjustPacketSize( m_Handle , MaxSize );
  if ( m_LastError != ePvErrSuccess )
  {
    SENDERR_1("Error: AVT_LAN Adjust Packet Size - %s", GetErrorMessage(m_LastError) );
    return false;
  }
  char buf[100] ;
  ULONG ulReceived = 0 ;
  GetMaxWidth() ; // for sensor size obtaining
  GetMaxHeight() ;
  if ( GetAttrEnum( "PixelFormat" , buf , 100 , ulReceived ) )
  {
    for ( int i = 0 ; i < LENARR(vFormats) ; i++ )
    {
      if ( !strcmp( vFormats[i].name , buf ) ) // matching
      {
        m_Format = vFormats[i].vm ;
        break ;
      }
    }
  }


  return BuildPropertyList();;    
}

void avt_lan::CameraClose()
{
  if ( m_Handle )
  {
    m_bAcqusitionStoped = false ;
    RunCamCommand( "AcquisitionAbort" ) ;
    Sleep( 50 ) ;
    PvCaptureQueueClear( m_Handle ) ;
    Sleep(50) ;
    if ( m_hFrameReady )
      SetEvent( m_hFrameReady ) ;
#ifdef _DEBUG
    SetAttrULONG( "HeartbeatTimeout" , 6000 ) ;
#endif
    PvCaptureEnd( m_Handle ) ;
    PvCameraClose( m_Handle ) ;
    m_Handle = NULL ;
  }
  if ( m_hFrameReady )
    SetEvent( m_hFrameReady ) ;
}

bool avt_lan::CloseDriver()
{
  FXAutolock al(m_avtLANInitLock);
  LONG res = InterlockedDecrement(&m_lAvtLanInstCntr);

  if (res==0)
  {
    PvLinkCallbackUnRegister(CameraLinkEventCB,ePvLinkAdd);
    PvLinkCallbackUnRegister(CameraLinkEventCB,ePvLinkRemove);
    PvUnInitialize();
    m_bAvtLanDriverReady=false;
    return true ;
  }
  return false ;
}

void avt_lan::ShutDown()
{
  CameraStop() ;
  Sleep(50) ;

  ReleaseAllFrames() ;

  ASSERT(m_iAllocatedFrames == 0) ;
  m_bRun = false ;
  if ( m_hFrameReady )
    SetEvent( m_hFrameReady ) ;
  C1394Camera::ShutDown();
  CloseDriver();
  if ( m_hFrameReady )
  {
    CloseHandle( m_hFrameReady ) ;
    m_hFrameReady = NULL ;
  }
}


bool avt_lan::IsPropertySupported( const char * PropName ) 
{
  tPvErr Result = PvAttrIsAvailable( m_Handle , PropName ) ;
  return ( Result == ePvErrSuccess ) ;
}
bool avt_lan::GetAttrULONG( const char * PropName , ULONG& ulVal ) 
{
  m_LastError = PvAttrUint32Get( m_Handle , PropName , (tPvUint32*) &ulVal ) ;
  if ( m_LastError != ePvErrSuccess )
  {
    SENDERR_2("Error: Get %s as ULONG - %s", PropName , GetErrorMessage(m_LastError) );
    return false ;
  }
  return true ;
}
bool avt_lan::SetAttrULONG( const char * PropName , ULONG ulVal ) 
{
  m_LastError = PvAttrUint32Set( m_Handle , PropName , (tPvUint32) ulVal ) ;
  if ( m_LastError != ePvErrSuccess )
  {
    SENDERR_3("Error: Set %s to %u - %s", PropName , ulVal , 
      GetErrorMessage(m_LastError) );
    return false ;
  }
  return true ;
}

bool avt_lan:: GetRangeULONG( const char * PropName , ULONG& ulMin , ULONG& ulMax ) 
{
  m_LastError = PvAttrRangeUint32( m_Handle , PropName , 
    (tPvUint32*) &ulMin , (tPvUint32*) &ulMax ) ;
  if ( m_LastError != ePvErrSuccess )
  {
    SENDERR_2("Error: Get Range for %s - %s", PropName , GetErrorMessage(m_LastError) );
    return false ;
  }
  return true ;
}
bool avt_lan::GetAttrInt64( const char * PropName , tPvInt64& i64Val ) 
{
  m_LastError = PvAttrInt64Get( m_Handle , PropName ,  &i64Val ) ;
  if ( m_LastError != ePvErrSuccess )
  {
    SENDERR_2("Error: Get %s as i64 - %s", PropName , GetErrorMessage(m_LastError) );
    return false ;
  }
  return true ;
}
bool avt_lan::SetAttrInt64( const char * PropName , tPvInt64 i64Val ) 
{
  m_LastError = PvAttrInt64Set( m_Handle , PropName ,  i64Val ) ;
  if ( m_LastError != ePvErrSuccess )
  {
    SENDERR_3("Error: Set %s to %I64d - %s", PropName , i64Val ,
      GetErrorMessage(m_LastError) );
    return false ;
  }
  return true ;
}

bool avt_lan::GetRangeInt64( const char * PropName , tPvInt64& i64Min , tPvInt64& i64Max ) 
{
  m_LastError = PvAttrRangeInt64( m_Handle , PropName , 
    &i64Min , &i64Max ) ;
  if ( m_LastError != ePvErrSuccess )
  {
    SENDERR_2("Error: Get Range i64 for %s - %s", PropName , 
      GetErrorMessage(m_LastError) );
    return false ;
  }
  return true ;
}


bool avt_lan::GetAttrBool( const char * PropName , tPvBoolean& bVal ) 
{
  m_LastError = PvAttrBooleanGet( m_Handle , PropName , &bVal ) ;
  if ( m_LastError != ePvErrSuccess )
  {
    SENDERR_2("Error: Get %s as bool - %s", PropName , GetErrorMessage(m_LastError) );
    return false ;
  }
  return true ;
}
bool avt_lan::SetAttrBool( const char * PropName ,tPvBoolean bVal ) 
{
  m_LastError = PvAttrBooleanSet( m_Handle , PropName , bVal ) ;
  if ( m_LastError != ePvErrSuccess )
  {
    SENDERR_3("Error: Set bool %s to %s - %s", PropName ,  
      bVal ? "true" : "false" , GetErrorMessage(m_LastError) );
    return false ;
  }
  return true ;
}

bool avt_lan::GetAttrFloat( const char * PropName , float& fVal ) 
{
  m_LastError = PvAttrFloat32Get( m_Handle , PropName , &fVal ) ;
  if ( m_LastError != ePvErrSuccess )
  {
    SENDERR_2("Error: Get %s as float - %s", PropName , GetErrorMessage(m_LastError) );
    return false ;
  }
  return true ;
}
bool avt_lan::SetAttrFloat( const char * PropName , float fVal ) 
{
  m_LastError = PvAttrFloat32Set( m_Handle , PropName , fVal ) ;
  if ( m_LastError != ePvErrSuccess )
  {
    SENDERR_3("Error: Set float %s to %8.3f - %s", PropName ,  
      fVal , GetErrorMessage(m_LastError) );
    return false ;
  }
  return true ;
}
bool avt_lan::GetRangeFloat( const char * PropName , float& fMin , float& fMax ) 
{
  m_LastError = PvAttrRangeFloat32( m_Handle , PropName , 
    &fMin , &fMax ) ;
  if ( m_LastError != ePvErrSuccess )
  {
    SENDERR_2("Error: Get Range float for %s - %s", PropName , 
      GetErrorMessage(m_LastError) );
    return false ;
  }
  return true ;
}

bool avt_lan::GetEnumsForAttr( const char * PropName , 
                              char * csv , ULONG ulLen , ULONG& WrittenLen )
{
  m_LastError = PvAttrRangeEnum( m_Handle , PropName , csv , ulLen , &WrittenLen ) ;
  if ( m_LastError != ePvErrSuccess )
  {
    SENDERR_2("Error: Get %s enumerators - %s", PropName , GetErrorMessage(m_LastError) );
    return false ;
  }
  return true ;
}
bool avt_lan::GetAttrString( const char * PropName , 
                            char * ValString , ULONG ulLen , ULONG& WrittenLen )
{
  m_LastError = PvAttrStringGet( m_Handle , PropName , ValString , ulLen , &WrittenLen ) ;
  if ( m_LastError != ePvErrSuccess )
  {
    SENDERR_2("Error: Get %s as string - %s", PropName , GetErrorMessage(m_LastError) );
    return false ;
  }
  return true ;
}
bool avt_lan::GetAttrEnum( const char * PropName , 
                          char * EnumString , ULONG ulLen , ULONG& WrittenLen )
{
  m_LastError = PvAttrEnumGet( m_Handle , PropName , EnumString , ulLen , &WrittenLen ) ;
  if ( m_LastError != ePvErrSuccess )
  {
    SENDERR_2("Error: Get %s as enum - %s", PropName , GetErrorMessage(m_LastError) );
    return false ;
  }
  return true ;
}

bool avt_lan::GetAttrEnum( const char * PropName , FXString& EnumString )
{
  char buf[200] ;
  ULONG ulWrittenLen ;
  m_LastError = PvAttrEnumGet( m_Handle , PropName , buf , sizeof(buf) , 
    &ulWrittenLen ) ;
  if ( m_LastError != ePvErrSuccess )
  {
    SENDERR_2("Error: Get %s as enum - %s", PropName , GetErrorMessage(m_LastError) );
    return false ;
  }
  EnumString = buf ;
  return true ;
}


bool avt_lan::SetAttrEnum( const char * PropName , char * EnumString )
{
  m_LastError = PvAttrEnumSet( m_Handle , PropName , EnumString ) ;
  if ( m_LastError != ePvErrSuccess )
  {
    m_LastError = PvAttrEnumSet( m_Handle , PropName , EnumString ) ;
    SENDERR_3("Error: Set %s to %s - %s", PropName , 
      EnumString , GetErrorMessage(m_LastError) );
    return false ;
  }
  return true ;
}
bool avt_lan::RunCamCommand( const char * CmdName ) 
{
  if ( ! m_Handle )
    return false ;
  m_LastError = PvCommandRun( m_Handle , CmdName ) ;
  if ( m_LastError != ePvErrSuccess )
  {
    SENDERR_2("Error: Run %s command - %s", CmdName ,GetErrorMessage(m_LastError) );
    return false ;
  }
  return true ;
}

bool avt_lan::BuildPropertyList()
{
  m_Properties.RemoveAll();
  if ( !CameraConnected() )
    return false ;
  tPvAttrListPtr   pAttributesList ; // will be received from camera driver
  tPvUint32        ulNAttributes = NULL ;

  tPvErr Err = PvAttrList( m_Handle , &pAttributesList , &ulNAttributes ) ;
  if ( Err == ePvErrSuccess )
  {
    m_CamAttribs.RemoveAll() ;
    char Buffer[2000] ;
    ULONG ulRetLen = 0 ;
    for ( ULONG i = 0 ; i < ulNAttributes ; i++ )
    {
      CameraAttribute Attr ;
      Err = PvAttrInfo( m_Handle , pAttributesList[i] , &Attr.m_Info ) ;
      if ( Err == ePvErrSuccess && (Attr.m_Info.Datatype != ePvDatatypeCommand)
        && (Attr.m_Info.Datatype != ePvDatatypeUnknown) 
        && (Attr.m_Info.Datatype != ePvDatatypeRaw ) )
      {
        Attr.m_Name = pAttributesList[i] ;
        switch ( Attr.m_Info.Datatype )
        {
        case ePvDatatypeEnum:
          {
            if ( GetEnumsForAttr( pAttributesList[i] , Buffer , 
              sizeof(Buffer) , ulRetLen ) && (ulRetLen != 0) )
            {
              FXParser Parser( Buffer ) ;
              int iPos = 0 ;
              FXString NextEnum ;
              while ( Parser.GetWord( iPos , NextEnum ) )
              {
                Attr.m_EnumRange.Add( NextEnum ) ;
              }
            }
            if ( GetAttrEnum( pAttributesList[i] , Buffer , sizeof(Buffer) , ulRetLen ) )
              Attr.m_enumVal = Buffer ;
            break;
          }
        case ePvDatatypeBoolean:
          {
            GetAttrBool( pAttributesList[i] , Attr.m_boolVal ) ;
            break;
          }
        case ePvDatatypeUint32:
          {
            GetRangeULONG( pAttributesList[i] , Attr.m_ulRange[0] , 
              Attr.m_ulRange[1] ) ;
            GetAttrULONG( pAttributesList[i] , Attr.m_uintVal ) ;
            break;
          }
        case ePvDatatypeInt64:
          {
            GetRangeInt64( pAttributesList[i] , Attr.m_i64Range[0] , 
              Attr.m_i64Range[1] ) ;
            GetAttrInt64( pAttributesList[i] , Attr.m_int64Val ) ;
            break;
          }
        case ePvDatatypeFloat32:
          {
            GetRangeFloat( pAttributesList[i] , Attr.m_fRange[0] , 
              Attr.m_fRange[1] ) ;
            GetAttrFloat( pAttributesList[i] , Attr.m_floatVal ) ;
            break;
          }
        case ePvDatatypeCommand:
          {
            break;
          }
        case ePvDatatypeString:
          {
            if ( GetAttrString( pAttributesList[i] , Buffer , sizeof(Buffer) , ulRetLen ) )
              Attr.m_stringVal = Buffer ;
            break;
          }
        default:
          break;
        }
        m_CamAttribs.Add( Attr ) ;
      }
    }
  }

  for ( int j = 0 ; j < LENARR(cProperties) ;j++ )
  {
    cProperties[j].m_bSupported = false ; 
  }
  CamProperty * pExpMode = NULL , 
    * pPixelFormat = NULL , * pTrigMode = NULL ,
    * pAcqMode = NULL , * pTrigEvent = NULL ,
    * pGainMode = NULL ;

  for ( int i = 0 ; i < m_CamAttribs.GetCount() ; i++ )
  {
    for ( int j = 0 ; j < LENARR(cProperties) ;j++ )
    {
      if ( !cProperties[j].CamPropertyName )
        continue ;
      if ( m_CamAttribs[i].m_Name == cProperties[j].CamPropertyName )
      {
        cProperties[j].m_bSupported = true ;
        cProperties[j].pInCamera = &m_CamAttribs[i] ;
        if ( m_CamAttribs[i].m_Name == "ExposureMode" )
        {
          pExpMode = &cProperties[j] ;
          m_bExpAuto =  (pExpMode->pInCamera->m_enumVal == "Auto") ;
        }        
        if ( m_CamAttribs[i].m_Name == "GainMode" )
        {
          pGainMode = &cProperties[j] ;
          m_bGainAuto =  (pGainMode->pInCamera->m_enumVal == "Auto") ;
        }
        else if ( m_CamAttribs[i].m_Name == "PixelFormat" )
        {
          pPixelFormat = &cProperties[j] ;
          m_bMonoPrev = m_bMono =  
            (pPixelFormat->pInCamera->m_uintVal == ePvFmtMono8 ) 
            || (pPixelFormat->pInCamera->m_uintVal == ePvFmtMono16 )
            || (pPixelFormat->pInCamera->m_uintVal == ePvFmtBayer8 )
            || (pPixelFormat->pInCamera->m_uintVal == ePvFmtBayer16 )
            || (pPixelFormat->pInCamera->m_uintVal == ePvFmtMono12Packed )
            || (pPixelFormat->pInCamera->m_uintVal == ePvFmtBayer12Packed ) ;
        }
        else if ( m_CamAttribs[i].m_Name == "FrameStartTriggerMode" )
        {
          pTrigMode = &cProperties[j] ;
          m_bFixedRate = m_bPrevFixedRate = 
            (pTrigMode->pInCamera->m_enumVal == "FixedRate") ;
        }
        else if ( m_CamAttribs[i].m_Name == "FrameStartTriggerEvent" )
        {
          pTrigEvent = &cProperties[j] ;
        }
        else if ( m_CamAttribs[i].m_Name == "AcquisitionMode" )
        {
          pAcqMode = &cProperties[j] ;
          m_bContinuous = 
            (pAcqMode->pInCamera->m_enumVal == "Continuous") ;
        }
        break ;
      }
    }
  }
  if ( pTrigMode && pTrigEvent )
  {
    if ( pTrigMode->pInCamera->m_enumVal.Find("SyncIn") >= 0 )
    {
      m_iHardTrigger = atoi( (LPCTSTR)pTrigMode->pInCamera->m_enumVal + 6 ) ;
    }
    else
      m_iHardTrigger = 0 ;

  }

  CAMERA1394::Property P;

  for (int i=0; i<LENARR(cProperties) ; i++)
  {
    if (cProperties[i].pr<=FGP_IMAGEFORMAT)
    {
      switch (cProperties[i].pr)
      {
      case FGP_ACQU_MODE:
        {
          P.name = cProperties[i].name ;
          P.id=(unsigned)cProperties[i].pr;
          FXString Modes ;
          int iCurrIndex = -1 ;
          for ( int i = 0 ; i < pAcqMode->pInCamera->m_EnumRange.GetCount() ; i++ )
          {
            FXString Mode ;
            Mode.Format( "%s(%d)" , (LPCTSTR) pAcqMode->pInCamera->m_EnumRange[i] , i );
            Modes += Mode ;
            if ( i != pAcqMode->pInCamera->m_EnumRange.GetUpperBound() )
              Modes += ',' ;
            if ( Mode == pAcqMode->pInCamera->m_enumVal )
              iCurrIndex = i ;
          }
          P.property.Format("ComboBox(%s(%s))",
            (LPCTSTR) P.name , (LPCTSTR)Modes );
          m_Properties.Add(P);
        }
        break ;
      case FGP_PACKET_SIZE:
        {
          P.name = cProperties[i].name ;
          P.id=(unsigned)cProperties[i].pr;
          ULONG ulVal = 1504 ;
          const char * PropName = "PacketSize" ;
          if ( IsPropertySupported( PropName ) )
          {
            P.property.Format( "Spin(%s,%u,%u)" , P.name , 1504 , 9088 ) ;
            m_Properties.Add(P) ;
          }
        }
        break ;
      case FGP_BANDWIDTH:
        {
          P.name = cProperties[i].name ;
          P.id=(unsigned)cProperties[i].pr;
          if ( IsPropertySupported( cProperties[i].CamPropertyName )  )
          {
            P.property.Format( "Spin(%s,%d,%d)" , P.name , 
              cProperties[i].pInCamera->m_ulRange[0]/1000000 , cProperties[i].pInCamera->m_ulRange[1]/1000000) ;
            m_Properties.Add(P) ;
          }
        }
        break ;
      case FGP_IMAGEFORMAT:
        {
          FXString items,tmpS;
          int formatCount=sizeof(vFormats)/sizeof(Videoformats);
          m_SupportedFormats.RemoveAll() ;
          for (int j=0; j<formatCount; j++)
          {
            if ( vFormats[j].supported && (IsFormatSupported( m_Handle , vFormats[j].vm )))
            {
              tmpS.Format("%s(%d),",vFormats[j].name,vFormats[j].vm);
              items+=tmpS;
              m_SupportedFormats.Add( vFormats[j].name ) ;
            }
          }
          if (items.GetLength())
          {
            items.Delete( items.GetLength() - 1 ) ;
            P.name=cProperties[i].name;
            P.id=(unsigned)cProperties[i].pr;
            P.property.Format("ComboBox(%s(%s))",P.name,items);
            m_Properties.Add(P);
          }
        }
        break;
      case FGP_FRAME_RATE:
      case FGP_RESOLUTION:
        break ;
      case FGP_ROI:
        {
          P.name=cProperties[i].name;
          P.id=(unsigned)cProperties[i].pr;
          unsigned uXOrig = GetXOffset( m_Handle ) ;
          unsigned uYOrig = GetYOffset( m_Handle ) ;
          unsigned uWidth = GetXSize( m_Handle ) ;
          unsigned uHeight = GetYSize( m_Handle ) ;
          P.property.Format("EditBox(%s([%u,%u,%u,%u]))", P.name ,
            uXOrig , uYOrig , uWidth , uHeight );
          m_Properties.Add(P);
          cProperties[i].m_bSupported = true ;
        }
        break;
      case FGP_TRIGGERONOFF:
        {
          if ( pTrigMode )
          {
            P.name=cProperties[i].name;
            P.id=(unsigned)cProperties[i].pr;
            ULONG ulLen = 0 ;
            FXString Modes ;
            int iCurrIndex = -1 ;
            for ( int i = 0 ; i < pTrigMode->pInCamera->m_EnumRange.GetCount() ; i++ )
            {
              FXString Mode ;
              Mode.Format( "%s(%d)" , (LPCTSTR) pTrigMode->pInCamera->m_EnumRange[i] , i );
              Modes += Mode ;
              if ( i != pTrigMode->pInCamera->m_EnumRange.GetUpperBound() )
                Modes += ',' ;
              if ( Mode == pTrigMode->pInCamera->m_enumVal )
                iCurrIndex = i ;
            }
            P.property.Format("ComboBox(%s(%s))",
              (LPCTSTR) P.name , (LPCTSTR)Modes );
            m_Properties.Add(P);
            if ( m_iHardTrigger && pTrigEvent )
            {
              P.name = pTrigEvent->name ;
              P.id = pTrigEvent->pr ;
              ULONG ulLen = 0 ;
              FXString Modes ;
              int iCurrIndex = -1 ;
              for ( int i = 0 ; i < pTrigEvent->pInCamera->m_EnumRange.GetCount() ; i++ )
              {
                FXString Mode ;
                Mode.Format( "%s(%d)" , (LPCTSTR)pTrigEvent->pInCamera->m_EnumRange[i] , i );
                Modes += Mode ;
                if ( i != pTrigEvent->pInCamera->m_EnumRange.GetUpperBound() )
                  Modes += ',' ;
                if ( Mode == pTrigEvent->pInCamera->m_enumVal )
                  iCurrIndex = i ;
              }
              P.property.Format("ComboBox(%s(%s))",
                (LPCTSTR) P.name , (LPCTSTR)Modes );
              m_Properties.Add(P);
            }
            else if ( m_bFixedRate )
            {
              CamProperty * pFps = GetPropertyByType( FGP_FRAME_RATE ) ;
              if ( pFps )
              {
                tPvFloat32 fFPS = 0. ;
                if ( GetAttrFloat( pFps->CamPropertyName , fFPS ) )
                {
                  P.name = pFps->name;
                  P.id = pFps->pr;
                  P.property.Format("EditBox(%s)", P.name );
                  m_Properties.Add(P);
                }
              }
            }
          }
          break;
        }
      case FGP_TRIG_EVENT:
        {
        }
        break ;
      case FGP_EXTSHUTTER:
        {
          P.name=cProperties[i].name;
          P.id=(unsigned)cProperties[i].pr;
          P.property.Format("Spin&Bool(%s,%d,%d)", P.name , 
            cProperties[i].pInCamera->m_ulRange[0] , 
            cProperties[i].pInCamera->m_ulRange[1] );
          m_Properties.Add(P);
          //           SENDINFO_1("Auto Exposure is %s", m_bExpAuto ? "true" : "false"  );
          if ( m_bExpAuto )
          {
            CamProperty * pExpTarget = GetPropertyByType( FGP_AUTOEXPOSURE ) ;
            if ( pExpTarget )
            {
              P.name = "ExpTarget" ;
              P.id = FGP_AUTOEXPOSURE ;
              P.property.Format( "Spin(%s,%d,%d)" , P.name , 
                pExpTarget->pInCamera->m_ulRange[0] , 
                pExpTarget->pInCamera->m_ulRange[1] ) ;
              m_Properties.Add(P);
            }
            CamProperty * pExpOutLiers = GetPropertyByType( FGP_EXP_AUTO_OUT ) ;
            if ( pExpOutLiers )
            {
              P.name = "ExpOutliers" ;
              P.id = FGP_EXP_AUTO_OUT ;
              P.property.Format( "Spin(%s,%d,%d)" , P.name , 
                pExpOutLiers->pInCamera->m_ulRange[0] , 
                pExpOutLiers->pInCamera->m_ulRange[1] ) ;
              m_Properties.Add(P);
            }
            CamProperty * pExpAutoAlgorithm = GetPropertyByType( FGP_EXP_AUTO_ALG ) ;
            if ( pExpAutoAlgorithm )
            {
              P.name = "ExpAutoAlg" ;
              P.id = FGP_EXP_AUTO_ALG ;
              FXString Algorithms ;
              int iCurrIndex = -1 ;
              for ( int i = 0 ; i < pExpAutoAlgorithm->pInCamera->m_EnumRange.GetCount() ; i++ )
              {
                FXString Mode ;
                Mode.Format( "%s(%d)" , 
                  (LPCTSTR) pExpAutoAlgorithm->pInCamera->m_EnumRange[i] , i );
                Algorithms += Mode ;
                if ( i != pExpAutoAlgorithm->pInCamera->m_EnumRange.GetUpperBound() )
                  Algorithms += ',' ;
                if ( Mode == pExpAutoAlgorithm->pInCamera->m_enumVal )
                  iCurrIndex = i ;
              }
              P.property.Format("ComboBox(%s(%s))",
                (LPCTSTR) P.name , (LPCTSTR)Algorithms );
              m_Properties.Add(P);
            }
          }
        }
        break;
      case FGP_EXP_AUTO_OUT:
      case FGP_EXP_AUTO_ALG:
      case FGP_SEND_FINFO:
        break ;
      case FGP_GAIN_PV:
        {
          P.name=cProperties[i].name;
          P.id=(unsigned)cProperties[i].pr;
          P.property.Format("Spin&Bool(%s,%d,%d)", P.name , 
            cProperties[i].pInCamera->m_ulRange[0] , 
            cProperties[i].pInCamera->m_ulRange[1] );
          m_Properties.Add(P);
          if ( m_bGainAuto )
          {
            CamProperty * pGainTarget = GetPropertyByType( FGP_GAIN_AUTO_TARG ) ;
            if ( pGainTarget )
            {
              P.name = "GainTarget" ;
              P.id = FGP_GAIN_AUTO_TARG ;
              P.property.Format( "Spin(%s,%d,%d)" , P.name , 
                pGainTarget->pInCamera->m_ulRange[0] , 
                pGainTarget->pInCamera->m_ulRange[1] ) ;
              m_Properties.Add(P);
            }
            CamProperty * pGainOutLiers = GetPropertyByType( FGP_GAIN_AUTO_OUT ) ;
            if ( pGainOutLiers )
            {
              P.name = "GainOutliers" ;
              P.id = FGP_GAIN_AUTO_OUT ;
              P.property.Format( "Spin(%s,%d,%d)" , P.name , 
                pGainOutLiers->pInCamera->m_ulRange[0] , 
                pGainOutLiers->pInCamera->m_ulRange[1] ) ;
              m_Properties.Add(P);
            }
          }
        }
        break ;
      case FGP_GAIN_AUTO_TARG:
      case FGP_GAIN_AUTO_OUT:
        break ;

      case FGP_TRIGGERDELAY:
        {
          P.name=cProperties[i].name;
          P.id=(unsigned)cProperties[i].pr;
          P.property.Format("Spin(%s,%d,%d)", P.name ,  
            cProperties[i].pInCamera->m_ulRange[0] , cProperties[i].pInCamera->m_ulRange[1] );
          m_Properties.Add(P);
          break;
        }
      case FGP_GRAB:
        P.name=cProperties[i].name;
        P.id=(unsigned)cProperties[i].pr;
        P.property.Format("Spin(%s,%d,%d)", P.name , -1 , 30000000 );
        m_Properties.Add(P);
        break ; // not shown property
      default:
        {
          SENDERR_1("Error: unsupported property %s", cProperties[i].name);
          break;
        }
      }
    }
    else // std property
    {
      if ( cProperties[i].CamPropertyName 
        && cProperties[i].pr != FGP_AUTOEXPOSURE )
      {
        if (  IsPropertySupported( cProperties[i].CamPropertyName ) )
        {
          bool autocap = (cProperties[i].AutoControl != NULL) ;
          P.name=cProperties[i].name;
          P.id = (unsigned)cProperties[i].pr ;
          P.property.Format("%s(%s,%d,%d)",
            (autocap)?SETUP_SPINABOOL:SETUP_SPIN, P.name , 
            cProperties[i].pInCamera->m_ulRange[0] , cProperties[i].pInCamera->m_ulRange[1] );
          m_Properties.Add(P);
        }
      }
    }
  }  
  if (m_LastError!=ePvErrSuccess)
    m_BMIH.biSize=0;
  else
  {
    m_BMIH.biSize=sizeof(BITMAPINFOHEADER);
    m_BMIH.biWidth=GetXSize(m_Handle);
    m_BMIH.biHeight=GetYSize(m_Handle);
    unsigned XOff=GetXOffset(m_Handle);
    unsigned YOff=GetYOffset(m_Handle);
    m_CurrentROI=CRect(XOff,YOff,XOff+m_BMIH.biWidth,YOff+m_BMIH.biHeight);
    m_BMIH.biPlanes=1;
    switch(m_Format)
    {
    case ePvFmtMono8    : 
    case ePvFmtBayer8    : 
      m_BMIH.biCompression=BI_Y8;
      m_BMIH.biBitCount=8;
      m_BMIH.biSizeImage=m_BMIH.biWidth*m_BMIH.biHeight;
      break;
    case ePvFmtYuv411:
      m_BMIH.biCompression=BI_YUV411;
      m_BMIH.biBitCount=12;
      m_BMIH.biSizeImage=3*m_BMIH.biWidth*m_BMIH.biHeight/2;
      break;
    case ePvFmtMono16   : 
    case ePvFmtBayer16   : 
      m_BMIH.biCompression=BI_Y16;
      m_BMIH.biBitCount=16;
      m_BMIH.biSizeImage=2*m_BMIH.biWidth*m_BMIH.biHeight;
      break;

    default: 
      m_BMIH.biSize=0;
      TRACE("!!! Unsupported format #%d\n", m_Format);
      SENDERR_1("!!! Unsupported format #%d",m_Format);
      return false;
    }
  }
  return true;
}

bool avt_lan::GetCameraProperty(unsigned i, int &value, bool& bauto)
{
  bauto=false;
  static char res[32] ;
  CamProperty * pProp = GetPropertyByType( (FG_PARAMETER) i ) ;
  if ( !pProp  ||  !pProp->m_bSupported )
    return false ;

  // 	if ((int)i<=FGP_IMAGEFORMAT)
  // 	{
  FXString EnumString ;
  switch (i)
  {
  case FGP_ACQU_MODE:
  case FGP_IMAGEFORMAT:
  case FGP_TRIGGERONOFF:
  case FGP_TRIG_EVENT:
  case FGP_EXP_AUTO_ALG:
    {
      if ( GetAttrEnum( pProp->CamPropertyName , EnumString ) )
      {
        for ( int i = 0 ; i < pProp->pInCamera->m_EnumRange.GetCount() ; i++ )
        {
          if ( pProp->pInCamera->m_EnumRange[i].Find( EnumString ) >= 0 )
          {
            value =  i ;
            return true;
          }
        }
      }
      return false ;
    }
  case FGP_FRAME_RATE:
    {
      tPvFloat32 fFPS = 0. ;
      if ( GetAttrFloat( "FrameRate" , fFPS ) )
      {
        sprintf_s( res , "%7.2f" , fFPS ) ;
        value = (int) &res[0] ;
        return true;
      }
      return false ;
    }
  case FGP_RESOLUTION:
    {
      return false ;
    }
  case FGP_ROI:
    {
      static FXString sROI;
      CRect rc;
      if ( GetROIFromCamera(rc) )
      {
        sROI.Format("%d,%d,%d,%d",rc.left,rc.top,rc.right,rc.bottom);
        value=(int)(LPCTSTR)sROI;
        return true;
      }
      return false ;
    }
    // 		case FGP_EXTSHUTTER:
    // 			{
    // 				value = GetLongExposure() ;
    // 				return true ;
    // 			}
  case FGP_SEND_FINFO:
    {
      value=FrameInfoOn;
      //value = (IsFrameInfoAvailable() != 0) ;
      return true ;
    }
  case FGP_TRIGGERDELAY:
    {
      value = GetTriggerDelay() ;
      return true ;
    }
  case FGP_GRAB:
    {
      bool bContinuous ;
      bool bRes = GetGrabConditions( bContinuous , value ) ;
      return true;
    }
  case FGP_AUTOEXPOSURE:
    {
      ULONG ulVal ;
      if ( GetAttrULONG( pProp->CamPropertyName , ulVal) )
      {
        value = ulVal ;
        return true ;
      }
      return false ;
    }
  default:
    {
      if ( pProp->CamPropertyName )
      {
        ULONG ulVal ;
        if ( GetAttrULONG( pProp->CamPropertyName , ulVal) )
        {
          if ( pProp->pr == FGP_BANDWIDTH )
            ulVal /= 1000000 ;
          value = ulVal ;
          FXString AutoPropName ;
          bauto = false ;
          if ( GetAutoControlPropName( pProp->AutoControl , AutoPropName ) )
          {
            FXString AutoPropVal ;
            if ( GetAttrEnum( (char*)(LPCTSTR)AutoPropName , AutoPropVal ) )
              bauto = (AutoPropVal == "Auto") ;
            if ( AutoPropName == "ExposureMode" )
              m_bExpAuto = bauto ;
          }
          return true ;
        }
      }
    }
    break;
  }
  return false;
}

bool avt_lan::SetCameraProperty(unsigned i, int &value, bool& bauto, bool& Invalidate)
{
  CamProperty * pProp = GetPropertyByType( (FG_PARAMETER) i ) ;
  if ( !pProp  ||  !pProp->m_bSupported )
    return false ;

  switch (i)
  {
  case FGP_PACKET_SIZE:
    {
      bool wasRunning=IsRunning();
      if (wasRunning)
        OnStop();
      SetAttrULONG( pProp->CamPropertyName , value ) ;
      CameraInit() ;
      if (wasRunning)
        OnStart();
      BuildPropertyList() ;
    }
    break ;
  case FGP_BANDWIDTH:
    {
      bool wasRunning=IsRunning();
      if (wasRunning)
        OnStop();
      SetAttrULONG( pProp->CamPropertyName , value * 1000000 ) ;
      if (wasRunning)
        OnStart();
      BuildPropertyList() ;
    }
    break ;
  case FGP_ACQU_MODE:
    {
      bool wasRunning=IsRunning();
      if (wasRunning)
        OnStop();
      FXString Mode = pProp->pInCamera->m_EnumRange[value] ;
      bool bRes = SetAttrEnum( pProp->CamPropertyName , (char*) (LPCTSTR)Mode ) ;
      if (wasRunning)
        OnStart();
      return bRes ;
    }
  case FGP_IMAGEFORMAT:
    {
      bool wasRunning=IsRunning();
      if (wasRunning)
        OnStop();
      FXString Format = m_SupportedFormats[value] ;
      m_Format =  GetFormatId( (char*) ((LPCTSTR)Format) ) ;
      bool res = SetAttrEnum( "PixelFormat" ,  (char*) ((LPCTSTR)Format) ) ;
      ULONG ulNewImageSize = GetImageSize( m_Format ) ;
      if ( m_ulImageSize != ulNewImageSize )
      {
        m_ulImageSize = ulNewImageSize ;
        PvCaptureQueueClear( m_Handle ) ;
      }
      if (wasRunning)
        OnStart();
      BuildPropertyList();
      Invalidate=true;
      return res;
    }
  case FGP_FRAME_RATE:
    {
      m_fFrameRate =(float)(value/100.);
      bool res = SetAttrULONG( "FrameRate" , value ) ;
      return res;
    }
  case FGP_RESOLUTION:
    {
      return false;
    }
  case FGP_ROI:
    {
      CRect rc;
      if (sscanf((LPCTSTR)value,"%d,%d,%d,%d",&rc.left,&rc.top,&rc.right,&rc.bottom)==4)
      {
        bool res=SetROI(rc);
        return res;
      }
      return false;
    }
  case FGP_TRIGGERONOFF:
    {
      bool wasRunning=IsRunning();
      if (wasRunning)
        OnStop();
      FXString Mode = pProp->pInCamera->m_EnumRange[value] ;
      bool bRes = SetAttrEnum( pProp->CamPropertyName , (char*) (LPCTSTR)Mode ) ;
      if (wasRunning)
        OnStart();
      BuildPropertyList();
      Invalidate = true ;
      return bRes ;
    }
    // 	case FGP_EXTSHUTTER:
    // 		{
    // 			SetLongExposure( value ) ;
    // 			return true ;
    // 		}
  case FGP_SEND_FINFO:
    {
      return true ;
    }
    // 	case FGP_TRIGGERDELAY:
    // 		{
    // 			SetTriggerDelay( value );
    // 			return true;
    // 		}
  case FGP_GRAB:
    {
      SetGrab( value ) ;
      return true;
    }
  case FGP_EXP_AUTO_ALG:
    {
      bool wasRunning=IsRunning();
      if (wasRunning)
        OnStop();
      FXString Mode = pProp->pInCamera->m_EnumRange[value] ;
      bool bRes = SetAttrEnum( pProp->CamPropertyName , (char*) (LPCTSTR)Mode ) ;
      if (wasRunning)
        OnStart();
      return bRes ;
    }

  default:
    {
      if ( pProp->AutoControl )
      {
        FXString AutoPropName ;
        if ( GetAutoControlPropName( pProp->AutoControl , AutoPropName ) )
        {
          SetAttrEnum( (char*)(LPCTSTR)AutoPropName , (bauto)? "Auto" : "Manual" ) ;
          if ( bauto )
            return true ;
        }
      }
      return SetAttrULONG( pProp->CamPropertyName , value ) ;
    }
    break ;
  }
  return false;
}


bool avt_lan::CameraStart()
{
  if ( !m_Handle )
    return false ;

  C1394Camera::CameraStart() ;
  ULONG ulCaptureState = 0 ;
  m_LastError = PvCaptureQuery( m_Handle , &ulCaptureState ) ;
  if ( m_LastError != ePvErrSuccess )
  {
    SENDERR_1("Error: Can't Query stream %s", GetErrorMessage(m_LastError) );
    return false ;
  }
  if ( !ulCaptureState )
  {
    m_LastError = PvCaptureStart( m_Handle ) ;
    if ( m_LastError != ePvErrSuccess )
    {
      SENDERR_1("Error: Can't start stream %s", GetErrorMessage(m_LastError) );
      return false ;
    }
  }
  ULONG ulFrameSize = 0 ;
  GetAttrULONG( "TotalBytesPerFrame" , ulFrameSize ) ;
  TRACE( "\nCamera Start Frame Size %u" , ulFrameSize ) ;
  if ( !QueueFreeFrames( ulFrameSize) )
  {
    if ( m_LastError )
      SENDERR_1("Error: Can't realloc frames %s", GetErrorMessage(m_LastError) );
  }
  while ( m_iAllocatedFrames < 5 )
  {
    if ( !AllocateCameraFrame() )
      break ;
  }
  //   SetAttrEnum( "FrameStartTriggerMode" , "Freerun") ;
  //   SetAttrEnum( "AcquisitionMode" , "Continuous" ) ;

  return RunCamCommand( "AcquisitionStart" ) ;
}

void avt_lan::CameraStop()
{
  if( !m_Handle )
    return ;
  ULONG   Capturing;
  m_LastError = PvCaptureQuery(m_Handle, &Capturing) ;
  if( m_LastError == ePvErrSuccess )
  {
    if(Capturing)
    {
      // then force the acquisition mode to stopped to get the camera to stop streaming
      RunCamCommand( "AcquisitionStop" );
      PvCaptureEnd(m_Handle);
    }
    // then dequeue all the frame still in the queue
    // in case there is any left in it and that the camera
    // was unplugged (we will ignore any error as the
    // capture was stopped anyway)
    PvCaptureQueueClear( m_Handle ); 
  }
  else if( m_LastError == ePvErrUnplugged ) // the camera was unplugged while we were streaming
  {
    // we still need to dequeue all queued frames
    PvCaptureQueueClear(m_Handle);
    SENDERR_1("Error: Can't talk with camera - %s", GetErrorMessage(m_LastError) );
  }
  else
    ASSERT(0) ;
  TRACE("CameraStop passed\n") ;
  SetEvent( m_hFrameReady ) ; // release for returning to DoJob
}

CVideoFrame* avt_lan::CameraDoGrab()
{
  double ts=GetHRTickCount();

  if ( m_Readyframes.IsEmpty() && m_bRun )  // wait for next frame
  {
    WaitForSingleObject( m_hFrameReady , INFINITE ) ;
    ts=GetHRTickCount();
  }
  if ( m_Readyframes.IsEmpty() )
    return NULL ;

  m_Lock.Lock() ;
  tPvFrame * pNextFrame = (tPvFrame *) m_Readyframes.RemoveHead() ;
  m_Lock.Unlock() ;
  CVideoFrame * pResult = NULL ;
  if ( m_bRun )
  {
    pResult = ConvertPvToSH( pNextFrame ) ;
    PvCaptureQueueFrame( m_Handle , pNextFrame , FrameDoneCB ) ;
  }
  else
    ReleaseCameraFrame( pNextFrame ) ;
  AddCPUUsage(GetHRTickCount()-ts);    

  return pResult;
}

bool avt_lan::IsTriggerMode()
{
  char buf[100] ;
  buf[0] = 0 ;
  ULONG ulReceivedLen ;
  if (  GetAttrEnum( "FrameStartTriggerMode" , buf , 100 , ulReceivedLen ) )
    return ( strstr( buf , "Synch" ) != NULL) ;
  return false ;
}
int avt_lan::GetTriggerMode( char * bufOut , int bufOutLen )
{
  bufOut[0] = 0 ;
  int iMode = 0 ;
  ULONG ulReceivedLen ;
  if (  GetAttrEnum( "FrameStartTriggerMode" , bufOut , bufOutLen , ulReceivedLen ) )
  {
    char * pChanPos = strstr( bufOut , "Sync" ) ;
    if ( pChanPos )// trigger mode
    {
      int iChannel = atoi(pChanPos + 5) ;  
      strcat(bufOut , "+") ;
      int iPos = strlen(bufOut) ;
      ULONG ulReceivedLen ;
      GetAttrEnum( "FrameStartTriggerEvent" , bufOut + iPos , bufOutLen - iPos , ulReceivedLen ) ;
      char * pEdgePos = strstr( bufOut + iPos , "Edge") ;
      if ( pEdgePos )
      {
        switch( pEdgePos[4] )
        {
        case 'R' : iMode = (iChannel == 1) ? 1 : 3 ; break ; // Rise front
        case 'F' : iMode = (iChannel == 1) ? 2 : 4 ; break ; // Fall front, i.e. inverse
        case 'A' : iMode = (iChannel == 1) ? 5 : 6 ; break ; // All fronts
        }
      }
      else
      {
        char * pEdgePos = strstr( bufOut + iPos , "Level") ;
        if ( pEdgePos )
        {
          switch( pEdgePos[5] )
          {
          case 'H' : iMode = (iChannel == 1) ? 7 : 8 ; break ; // active high level
          case 'L' : iMode = (iChannel == 1) ? 9 : 10 ; break ; // low level, i.e. inverse
          }
        }
      }
    }
    return iMode ;
  }
  return -1 ;
}

void avt_lan::SetTriggerMode(int iMode)
{

  // iMode : 0 - no trigger, 1 - channel 1 no inverse , 2 - channel 2 no inverse
  //                         3 - channel 1 no inverse , 4 - channel 2 inverse
  //                         5 - channel 1 all fronts , 6 - channel 2 all fronts
  //                         7 - channel 1 level high , 8 - channel 2 level high
  //                         9 - channel 1 level low  , 10 - channel 2 level low

  UINT32 nOn=(iMode)?1:0; // 0=ext. trigger off, else ext. trigger on
  if ( !nOn )
  {
    SetAttrEnum( "AcquisitionMode" , "Continuous" ) ;
    SetAttrEnum( "FrameStartTriggerMode" , "Freerun" ) ;
  }
  else
  {
    SetAttrEnum( "AcquisitionMode" , "MultiFrame" ) ;
    SetAttrEnum( "FrameStartTriggerMode" , (iMode & 1) ? "SyncIn1" : "SyncIn2" ) ;
    switch( iMode )
    {
    case 1:
    case 2:
      SetAttrEnum( "FrameStartTriggerEvent" , "EdgeRising" ) ;
      break ;
    case 3:
    case 4:
      SetAttrEnum( "FrameStartTriggerEvent" , "EdgeFalling" ) ;
      break ;
    case 5:
    case 6:
      SetAttrEnum( "FrameStartTriggerEvent" , "EdgeAny" ) ;
      break ;
    case 7:
    case 8:
      SetAttrEnum( "FrameStartTriggerEvent" , "LevelHigh" ) ;
      break ;
    case 9:
    case 10:
      SetAttrEnum( "FrameStartTriggerEvent" , "LevelLow" ) ;
      break ;
    default: SENDERR_1( "Error: Unknown Trigger Mode %d" , iMode ) ;
    }
  }
}

void avt_lan::GetROI(CRect& rc)
{
  rc=m_CurrentROI;
}

bool avt_lan::GetROIFromCamera(CRect& rc)
{
  unsigned uXOrig = GetXOffset( m_Handle ) ;
  unsigned uYOrig = GetYOffset( m_Handle ) ;
  unsigned uWidth = GetXSize( m_Handle ) ;
  unsigned uHeight = GetYSize( m_Handle ) ;
  if ( uWidth && uHeight )
  {
    m_CurrentROI.left = uXOrig ;
    m_CurrentROI.top = uYOrig ;
    m_CurrentROI.right = uXOrig + uWidth ;
    m_CurrentROI.bottom = uYOrig + uHeight ;
    rc=m_CurrentROI;
    return true ;
  }
  return false ;
}

bool avt_lan::SetROI(CRect& rc)
{
  if (rc.left!=-1) //must be divisible by 4 - inheritance from old AVT
  {                // may be not necessary 
    rc.left&=~3;
    rc.right&=~3;
    rc.bottom&=~3;
    rc.top&=~3;
    rc.NormalizeRect() ;

    if ( rc.left < 0 )
    {
      rc.right = rc.Width() ;
      rc.left = 0 ;
    }
    if ( rc.top < 0 )
    {
      rc.bottom = rc.Height() ;
      rc.top = 0 ;
    }
    if ( rc.Width() + rc.left > m_SensorSize.cx )
    {
      if ( rc.Width() <= m_SensorSize.cx )
        rc.left = m_SensorSize.cx - rc.Width() ;
      else
      {
        rc.left = 0 ;
        rc.right = m_SensorSize.cx ;
      }
    }

    if ( rc.Height() + rc.top > m_SensorSize.cy )
    {
      if ( rc.Height() <= m_SensorSize.cy )
        rc.top = m_SensorSize.cy - rc.Height() ;
      else
      {
        rc.top = 0 ;
        rc.bottom = m_SensorSize.cy ;
      }
    }
  }
  else
  {
    rc.left=rc.top=0;
    rc.right = m_SensorSize.cx ;
    rc.bottom = m_SensorSize.cy ;
  }

  bool bNeedRestart = ( m_CurrentROI.Width() != rc.Width() 
    || m_CurrentROI.Height() != rc.Height() ) ;
  if ( bNeedRestart )
  {
    CameraStop() ;
    SetAttrULONG( "RegionX" , rc.left ) ;
    SetAttrULONG( "RegionY" , rc.top ) ;
    SetAttrULONG( "Width" , rc.right-rc.left ) ;
    SetAttrULONG( "Height" , rc.bottom-rc.top ) ;
    CameraStart() ;
  }
  else
  {
    SetAttrULONG( "RegionX" , rc.left ) ;
    SetAttrULONG( "RegionY" , rc.top ) ;
  }

  m_CurrentROI = rc;
  return true;        
}
ULONG avt_lan::GetLongExposure()
{
  ULONG iExp_usec ;
  if ( GetAttrULONG( "ExposureValue" , iExp_usec ) )
    return ( iExp_usec );
  return 1 ;
}

void avt_lan::SetLongExposure( int iExp_usec)
{
  SetAttrULONG( "ExposureValue" , (ULONG) iExp_usec ) ;
}

bool avt_lan::IsFrameInfoAvailable()
{
  // 	DWORD iAvail ;
  // 	m_Camera.ReadRegister(0xF1000630,&iAvail);
  // 	if ( iAvail & 0x80000000 )
  // 		return (iAvail & 0x0200ffff ) ;
  // 	else
  return true ;
}

void avt_lan::SetSendFrameInfo( int iSend)
{
  // 	m_Camera.WriteRegister(0xF1000630 , (iSend) ? 0x02000000 : 0 );
}

ULONG avt_lan::GetTriggerDelay()
{
  ULONG iDelay_usec ;
  GetAttrULONG( "FrameStartTriggerDelay" , iDelay_usec ) ;
  return iDelay_usec ;
}

void avt_lan::SetTriggerDelay( ULONG iDelay_uS)
{
  SetAttrULONG( "FrameStartTriggerDelay" , iDelay_uS ) ;
}

bool avt_lan::SetGrab( int iNFrames )
{
  if (  (iNFrames < -1)  
    ||  (iNFrames > 0xffff) )
    return false ;
  if ( iNFrames == -1 )
  {
    SetAttrEnum( "FrameStartTriggerMode" , "Freerun" ) ;
    SetAttrEnum( "AcqEndTriggerMode" , "Disabled" ) ;
    SetAttrEnum( "AcqStartTriggerMode" , "Disabled" ) ;
    SetAttrEnum( "AcquisitionMode" , "Continuous" ) ;
    RunCamCommand( "AcquisitionStart" ) ;
    return true ;
  }
  else if ( iNFrames == 0 )
  {
    RunCamCommand( "AcquisitionStop" ) ;
    return true ;
  }
  SetAttrEnum( "AcqEndTriggerMode" , "Disabled" ) ;
  SetAttrEnum( "AcqStartTriggerMode" , "Disabled" ) ;
  SetAttrEnum( "AcquisitionMode" , "MultiFrame" ) ;
  SetAttrULONG( "AcquisitionFrameCount" , iNFrames ) ;
  RunCamCommand( "AcquisitionStart" ) ;
  return true ;
}

bool avt_lan::GetGrabConditions( 
  bool& bContinuous , int& iNRestFrames ) 
{
  char buf[100] ;
  ULONG ulLen ;
  if ( GetAttrEnum( "AcquisitionMode" , buf , sizeof(buf) , ulLen ) )
  {
    if ( strstr( buf , "Continuous" ) )
    {
      bContinuous = true ;
      iNRestFrames = -1 ;
      return true ;
    }
    if ( strstr( buf , "SingleFrame" ) )
    {
      bContinuous = false ;
      iNRestFrames = 0 ;  // ?
      return true ;
    }
    if ( strstr( buf , "MultiFrame" ) || strstr( buf , "Recorder" ))
    {
      bContinuous = false ;
      return GetAttrULONG( "AcquisitionFrameCount" , (ULONG&) iNRestFrames ) ;
      iNRestFrames = 0 ;  // ?
      return true ;
    }
  }
  return true ;
}

ULONG avt_lan::GetMaxWidth()
{
  if ( !m_SensorSize.cx )
    m_SensorSize.cx = ::GetMaxWidth( m_Handle ) ;
  return m_SensorSize.cx ;
}

ULONG avt_lan::GetMaxHeight()
{
  if ( !m_SensorSize.cy )
    m_SensorSize.cy = ::GetMaxHeight( m_Handle ) ;
  return m_SensorSize.cy ;
}

tPvFrame * avt_lan::AllocateCameraFrame()
{
  ULONG ulSize = m_SensorSize.cx * m_SensorSize.cy ;
  for ( int i = 0 ; i < LENARR(vFormats) ; i++ )
  {
    if ( vFormats[i].vm == m_Format )
    {
      ulSize = ROUND(ulSize * vFormats[i].dBytesPerPixel) ;
      m_ulImageSize = ulSize ;
      return AllocateCameraFrame( ulSize ) ;
    }
  }

  return NULL ;
}
tPvFrame * avt_lan::AllocateCameraFrame( ULONG ulSize )
{
  tPvFrame * pFrame = new tPvFrame ;
  if ( !pFrame )
  {
    SENDERR_1("Error: Can't allocate descriptor %s", GetErrorMessage(m_LastError) );
    return NULL ;
  }
  memset( pFrame , 0 , sizeof(tPvFrame) ) ;
  pFrame->ImageBuffer = new BYTE[ulSize] ;
  if ( !pFrame->ImageBuffer )
  {
    SENDERR_1("Error: Can't allocate image %s", GetErrorMessage(m_LastError) );
    delete pFrame ;
    return NULL ;
  }
  pFrame->ImageBufferSize = ulSize ;
  // Ancillary will be used in future versions
  pFrame->Context[0] = this ;

  m_LastError = PvCaptureQueueFrame( m_Handle , pFrame , FrameDoneCB ) ;
  if ( m_LastError != ePvErrSuccess )
  {
    SENDERR_1("Error: Can't Queue frame %s", GetErrorMessage(m_LastError) );
    delete pFrame->ImageBuffer ;
    delete pFrame ;
    return NULL ;
  }
  else
  {
    m_Lock.Lock() ;
    m_iAllocatedFrames++ ;
    m_Lock.Unlock() ;
    TRACE("\n%d Frames Queued" , m_iAllocatedFrames ) ;
  }
  return pFrame ;
}

int avt_lan::ReleaseCameraFrame( tPvFrame * pFrame )
{
  POSITION pos ;
  m_Lock.Lock() ;
  for ( pos = m_FreeFrames.GetHeadPosition() ; pos != NULL ;  )
  {
    if ( m_FreeFrames.GetAt( pos ) == pFrame )
    {
      m_FreeFrames.RemoveAt( pos ) ;
      m_iAllocatedFrames-- ;
      delete pFrame->ImageBuffer ;
      delete pFrame ;
      break ;
    }
    m_FreeFrames.GetNext( pos ) ;
  }
  m_Lock.Unlock() ;

  return m_FreeFrames.GetCount() ;
}

bool avt_lan::ReleaseAllFrames()
{
  m_Lock.Lock() ;
  while ( m_FreeFrames.GetCount() )
  {
    tPvFrame * pFrame = (tPvFrame *) m_FreeFrames.GetHead() ;
    m_FreeFrames.RemoveHead() ;
    if ( pFrame )
    {
      m_iAllocatedFrames-- ;
      delete pFrame->ImageBuffer ;
      delete pFrame ;
    }
  }
  while ( m_Readyframes.GetCount() )
  {
    tPvFrame * pFrame = (tPvFrame *) m_Readyframes.GetHead() ;
    m_Readyframes.RemoveHead() ;
    if ( pFrame )
    {
      m_iAllocatedFrames-- ;
      delete pFrame->ImageBuffer ;
      delete pFrame ;
    }
  }
  m_Lock.Unlock() ;
  return true ;
}

ULONG avt_lan::GetImageSize( 
  tPvImageFormat PixelFormat , CRect ROI )
{
  int iFormatIndex = GetFormatId( PixelFormat) ;
  if ( iFormatIndex >= 0 )
  {
    ULONG ulWidth ;
    ULONG ulHeight ;
    ULONG ulSize ;
    if ( ! ROI.Width() ) // No ROI, sensor size will be used
    {
      ulWidth = GetMaxWidth() ;
      ulHeight = GetMaxHeight() ;
    }
    else
    {
      if ( ROI.PtInRect( ROI.TopLeft() )
        && ROI.PtInRect( ROI.BottomRight() - CSize(1,1) ) )
      {
        ulWidth = ROI.Width() ;
        ulHeight = ROI.Height() ;
      }
      else
        return 0 ;

    }
    ulSize = ulWidth * ulHeight ;
    ulSize = ROUND(ulSize * vFormats[iFormatIndex].dBytesPerPixel) ;
    return ulSize ;
  }

  return 0 ;
}


bool avt_lan::QueueFreeFrames( ULONG ulImageSize )
{
  bool bRes = false ;
  m_Lock.Lock() ;
  while ( m_FreeFrames.GetCount())
  {
    tPvFrame * pFrame = (tPvFrame*) m_FreeFrames.GetHead() ;
    m_FreeFrames.RemoveHead() ;
    if ( pFrame )
    {

      if ( ReallocFrame( pFrame , m_ulImageSize ) )
      {
        tPvErr Error = PvCaptureQueueFrame( m_Handle ,
          pFrame , FrameDoneCB ) ;
        if ( Error == ePvErrSuccess )
        {
          pFrame = NULL ;
          bRes = true ;
        }
        else
        {
          SENDERR_1("Error: Can't requeue frame %s", 
            GetErrorMessage(Error));
        }
      }
      if ( pFrame )
      {
        delete pFrame ;
        m_iAllocatedFrames-- ;
      }
    }
  }
  m_Lock.Unlock() ;
  return bRes ;
}


int avt_lan::ReallocFramesIfNecessary()
{
  ULONG ulImageSize = GetImageSize( m_Format ) ;
  if ( !ulImageSize )
    return -1 ;
  m_ulImageSize = ulImageSize ;
  POSITION pos ;
  m_Lock.Lock() ;
  for ( pos = m_FreeFrames.GetHeadPosition() ; pos != NULL ;  )
  {
    tPvFrame * pFrame = (tPvFrame*) m_FreeFrames.GetAt( pos ) ;

    if ( !ReallocFrame( pFrame , m_ulImageSize ) )
    {
      delete pFrame ;
      m_iAllocatedFrames-- ;
      m_FreeFrames.RemoveAt( pos ) ;
    }
    m_FreeFrames.GetNext( pos ) ;
  }
  m_Lock.Unlock() ;

  return m_FreeFrames.GetCount() ;
}

CVideoFrame * avt_lan::ConvertPvToSH( tPvFrame * pFrame )
{
  //TRACE("Frame: %u passed to conversion\n", pFrame->FrameCount);
  tPvImageFormat Format = pFrame->Format ;
  bool bReformatBMI = false ;
  int iFormatIndex = GetFormatId( Format ) ;
  if ( m_Format != Format ) // there is new format
  {
    if ( iFormatIndex >= 0  &&  vFormats[iFormatIndex].supported )
    {
      m_iLast1394Format = vFormats[iFormatIndex].AVT1394_ColorMode ;
      m_Format = Format ;
      bReformatBMI = true ;
    }
    else
    {
      if ( iFormatIndex == -1 )
        SENDERR_1( "Error: Received unknown image format %d" , Format ) ;
      else
        SENDERR_1( "Error: Received unsupported image format %s" , 
        vFormats[iFormatIndex].name ) ;
      iFormatIndex = -1 ;
    }
  }
  if ( iFormatIndex >= 0 )
  {
    int iWidth = (pFrame->Width / 4) * 4 ;
    int iHeight = (pFrame->Height / 4) * 4 ;
    int iNewSize = ( iWidth * iHeight ) ;
    switch ( vFormats[iFormatIndex].BMP_OutCompression )
    {
    case BI_Y8: break ;
    case BI_Y16: iNewSize *= 2 ; break ;
    case BI_YUV9: iNewSize = ( iNewSize * 9)/8 ;
    case BI_YUV12: iNewSize = (iNewSize * 3)/2 ;
    }
    if ( iNewSize != m_BMIH.biSizeImage ) // image size is changed
      bReformatBMI = true ;
    if ( bReformatBMI )
    {
      m_BMIH.biWidth = iWidth ;
      m_BMIH.biHeight = iHeight ;
      m_BMIH.biSizeImage = iNewSize ;
      m_BMIH.biCompression = vFormats[iFormatIndex].BMP_OutCompression ;

      m_BMIH.biSize = sizeof(BITMAPINFOHEADER) ;
      m_BMIH.biPlanes = 1 ;
      m_BMIH.biBitCount = ROUND( 8 * vFormats[iFormatIndex].dBytesPerPixel ) ;
      m_BMIH.biXPelsPerMeter = 72 ;
      m_BMIH.biYPelsPerMeter = 72 ;
      m_BMIH.biClrUsed = 0 ;
      m_BMIH.biClrImportant = 0 ;
      memcpy( &m_RealBMIH , 
        &m_BMIH , m_BMIH.biSize );
      m_RealBMIH.biSizeImage = pFrame->Width * pFrame->Height ;
    }
    pTVFrame frame = (pTVFrame)malloc(sizeof(TVFrame));
    frame->lpData=NULL;
    if ( pFrame->Format != ePvFmtMono12Packed
      &&  pFrame->Format != ePvFmtBayer12Packed )
    {
      switch( m_BMIH.biCompression )
      {
      case BI_Y8:
        frame->lpBMIH = (LPBITMAPINFOHEADER)malloc( iNewSize + sizeof(BITMAPINFOHEADER) ) ;
        memcpy(frame->lpBMIH , &m_BMIH , sizeof(BITMAPINFOHEADER) ) ;
        memcpy( &frame->lpBMIH[1], pFrame->ImageBuffer , iNewSize ) ;
        break ;
      case BI_Y16:
        {
          frame->lpBMIH = (LPBITMAPINFOHEADER)malloc( iNewSize + sizeof(BITMAPINFOHEADER) ) ;
          memcpy(frame->lpBMIH , &m_BMIH , sizeof(BITMAPINFOHEADER) ) ;
          USHORT * pSrc = (USHORT*)pFrame->ImageBuffer ;
          USHORT * pDst = (USHORT*)&frame->lpBMIH[1] ;
          USHORT * pEndDst = pDst + (m_BMIH.biSizeImage/2) ;
          while ( pDst < pEndDst )
            *(pDst++) = *(pSrc++) << 2 ;// 14 bits ADC resolution
        }
        break ;
      case BI_YUV9:
        frame->lpBMIH = y8yuv9( &m_BMIH , (LPBYTE) pFrame->ImageBuffer ) ;
        break ;
      case BI_YUV12:
        frame->lpBMIH = yuv411yuv9( &m_BMIH , (LPBYTE) pFrame->ImageBuffer ) ;
        break ;
      }
    }
    else
    {  // unpack Packed 12 bits to 16 bits format
      ASSERT(m_BMIH.biCompression == BI_Y16 ) ;
      frame->lpBMIH = (LPBITMAPINFOHEADER)malloc( iNewSize + sizeof(BITMAPINFOHEADER) ) ;
      memcpy(frame->lpBMIH , &m_BMIH , sizeof(BITMAPINFOHEADER) ) ;
      for ( int iY = 0 ; iY < iHeight ; iY++ )
      {
        BYTE * pSrc = (BYTE*)pFrame->ImageBuffer + ((iY * pFrame->Width)*3)/2 ;
        USHORT * pDst = (USHORT*)(frame->lpBMIH + 1) ;
        for ( int iX = 0 ; iX < iWidth ; iX += 4 )
        {
          *(pDst++) = (((USHORT)(*pSrc)) << 4) + (((USHORT) ((*(pSrc+1)) & 0x0f)) << 12);
          *(pDst+1) = (((USHORT) (*(pSrc+1) & 0xf0)) << 4) | (((USHORT) (*(pSrc+2))) << 8) ;
          pSrc += 3 ;
          pDst += 2 ;
        }
      }
    }
    if ((frame)&&(frame->lpBMIH))
    {
      CVideoFrame* vf=CVideoFrame::Create(frame);
      vf->SetLabel(m_CameraID);
      vf->SetTime( GetGraphTime() ) ;
      vf->ChangeId( ++(m_FrameCounter) ) ;
      return vf ;
    }
    else if (frame)
      free(frame);
  }
  return NULL ;
}

bool avt_lan::ScanSettings(FXString& text)
{
  // Prepare cameras list, if changes on bus are recognized
  if ( m_bCamLANStatusChanged )
    DiscoverCameras() ;

  return C1394Camera::ScanSettings( text ) ;
}

