// IDSCamera.cpp: implementation of the IDS class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IDS.h"
#include "IDSCamera.h"
#include <video\stdcodec.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifndef ROUND
#define ROUND(a) ((int)((a) + .5))
#endif


#define THIS_MODULENAME "IDSCamera"
#define MARLINCAMERAMAXNAMELENGTH 128

IMPLEMENT_RUNTIME_GADGET_EX(IDS, C1394Camera, "Video.capture", TVDB400_PLUGIN_NAME);

typedef struct tagVideoformats
{
  DWORD        compression ;
  DWORD        IDSCompression ;
  const char * name; 
  bool         supported;
} Videoformats;

typedef struct tagFramerates
{
  // 	FG_FRAMERATE        fr;
  const char *        name; 
  const char *        m7name;
}Framerates;

typedef struct tagResolutions
{
  // 	FG_RESOLUTION       res;
  const char *        name; 
}Resolutions;

typedef struct tagCamProperties
{
  FG_PARAMETER        pr;
  const char *        name; 
}CamProperties;

static LPCTSTR GetErrorMessage(UINT32 errCode)
{
  switch (errCode)
  {
  case IS_NO_SUCCESS:         return "Function Failed" ;
  case IS_SUCCESS:            return "No Error";
  case IS_INVALID_HANDLE:                return "Invalid camera handle";
  case IS_IO_REQUEST_FAILED:             return "Error, No details";
  case IS_CANT_OPEN_DEVICE:              return "Can't open device";
  case IS_CANT_CLOSE_DEVICE:             return "Can't close device";
  case IS_CANT_SETUP_MEMORY:             return "Can't setup memory";
  case IS_NO_HWND_FOR_ERROR_REPORT:      return "Error creating WinDevice";
  case IS_ERROR_MESSAGE_NOT_CREATED:     return "error msg not created";
  case IS_ERROR_STRING_NOT_FOUND:        return "String not found";
  case IS_HOOK_NOT_CREATED:              return "Hook is not created";
  case IS_TIMER_NOT_CREATED:             return "Timer not created";
  case IS_CANT_OPEN_REGISTRY:            return "Can't open registry";
  case IS_CANT_READ_REGISTRY:            return "can't read registry";
  case IS_CANT_VALIDATE_BOARD:           return "Can't validate board";
  case IS_CANT_GIVE_BOARD_ACCESS:        return "can't give board access";
  case IS_NO_IMAGE_MEM_ALLOCATED:        return "No image memory allocated";
  case IS_CANT_CLEANUP_MEMORY:           return "Can't clean up memory";
  case IS_CANT_COMMUNICATE_WITH_DRIVER:  return "Can't communicate with driver";
  case IS_FUNCTION_NOT_SUPPORTED_YET:    return "Function not supported";
  case IS_OPERATING_SYSTEM_NOT_SUPPORTED:return "OS not supported";
  case IS_INVALID_VIDEO_IN:              return "Invalid video in";
  case IS_INVALID_IMG_SIZE:                  return "Invalid Image Size";
  case IS_INVALID_ADDRESS:                   return "Invalid Address";
  case IS_INVALID_VIDEO_MODE:                return "Invalid Video mode";
  case IS_INVALID_AGC_MODE:                  return "Invalid AGC Mode";
  case IS_INVALID_GAMMA_MODE:                return "Invalid Gamma mode";
  case IS_INVALID_SYNC_LEVEL:                return "Invalid Sync level";
  case IS_INVALID_CBARS_MODE:                return "Invalid CBars mode";
  case IS_INVALID_COLOR_MODE:                return "Invalid Color Mode";
  case IS_INVALID_SCALE_FACTOR:              return "Invalid Scale factor";
  case IS_INVALID_IMAGE_SIZE:                return "Invalid Image Size";
  case IS_INVALID_IMAGE_POS:                 return "Invalid Image Pos";
  case IS_INVALID_CAPTURE_MODE:              return "Invalid capture mode";
  case IS_INVALID_RISC_PROGRAM:              return "Invalid RISC program";
  case IS_INVALID_BRIGHTNESS:                return "Invalid brightness";
  case IS_INVALID_CONTRAST:                  return "Invalid contrast";
  case IS_INVALID_SATURATION_U:              return "Invalid saturation U";
  case IS_INVALID_SATURATION_V:              return "Invalid saturation V";
  case IS_INVALID_HUE:                       return "Invalid HUE";
  case IS_INVALID_HOR_FILTER_STEP:           return "Invalid horizontal filter step";
  case IS_INVALID_VERT_FILTER_STEP:          return "Invalid vertical filter step";
  case IS_INVALID_EEPROM_READ_ADDRESS:       return "Invalid EEPROM read address ";
  case IS_INVALID_EEPROM_WRITE_ADDRESS:      return "Invalid EEPROM Write address";
  case IS_INVALID_EEPROM_READ_LENGTH:        return "Invalid EEPROM Read Length";
  case IS_INVALID_EEPROM_WRITE_LENGTH:       return "Invalid EEPROM Write Length";
  case IS_INVALID_BOARD_INFO_POINTER:        return "Invalid board info ptr";
  case IS_INVALID_DISPLAY_MODE:              return "Invalid display mode";
  case IS_INVALID_ERR_REP_MODE:              return "Invalid err rep mode";
  case IS_INVALID_BITS_PIXEL:                return "Invalid bits per pixel";
  case IS_INVALID_MEMORY_POINTER:            return "Invalid memory ptr";
  }
  static char msg[100] ;
  sprintf_s( msg ,  "Unknown Error %d(0x%x)" , errCode , errCode ) ;
  return "Unknown error";
}

Videoformats vFormats[]=
{
  {BI_Y8, IS_CM_MONO8 , "Y8",true},
  //   {BI_YUV411,"YUV411", true},
  //  {BI_YUV9,"YUV9",true},
  //  {BI_YUV12,"YUV12", true},
//   {BI_UYVY, IS_CM_UYVY_PACKED , "YUV422", false},
  //   {BI_YUV444, "YUV444",false},
  //   {BI_RGB8, "RGB8",false},
  {BI_Y16, IS_CM_MONO16 , "Y16",true},
  //   {BI_RGB16,"RGB16",false},
  //   {BI_SY16, "SY16",false},
  // 	{CM_SRGB16, "SRGB16",false},
  {IS_CM_SENSOR_RAW8 , IS_CM_SENSOR_RAW8  ,"RAW8",true},
  {IS_CM_SENSOR_RAW16, IS_CM_SENSOR_RAW16 , "RAW16",false},
};


#define FGP_FRAMERATE  ((FG_PARAMETER)(FGP_IMAGEFORMAT-1))
#define FGP_RESOLUTION ((FG_PARAMETER)(FGP_IMAGEFORMAT-2))
#define FGP_ROI        ((FG_PARAMETER)(FGP_IMAGEFORMAT-3))
#define FGP_TRIGGERONOFF ((FG_PARAMETER)(FGP_IMAGEFORMAT-4))
#define FGP_EXTSHUTTER ((FG_PARAMETER)(FGP_IMAGEFORMAT-5))
#define FGP_SEND_FINFO ((FG_PARAMETER)(FGP_IMAGEFORMAT-6))
#define FGP_TRIGGERDELAY ((FG_PARAMETER)(FGP_IMAGEFORMAT-7))
#define FGP_GRAB       ((FG_PARAMETER)(FGP_IMAGEFORMAT-8))
#define FGP_EXTSHUT_ADD ((FG_PARAMETER)(FGP_IMAGEFORMAT-9))

CamProperties cProperties[] =
{
  {FGP_IMAGEFORMAT, "Format"},
  {FGP_FRAMERATE, "FrameRate"},
  {FGP_RESOLUTION, "Resolution"},
  {FGP_BRIGHTNESS,"Brightness"},
  {FGP_AUTOEXPOSURE,"Exposure"},
  {FGP_SHARPNESS,"Sharpness"},
  {FGP_WHITEBALCB,"HWWhitebalanceB"},
  {FGP_WHITEBALCR,"HWWhitebalanceR"},
  {FGP_HUE,"Hue"},
  {FGP_SATURATION,"Saturation"},
  {FGP_GAMMA,"Gamma"},
  {FGP_IRIS,"Iris"},
  {FGP_FOCUS,"Focus"},
  {FGP_ZOOM,"Zoom"},
  {FGP_PAN,"Pan"},
  {FGP_TILT,"Tilt"},
  {FGP_SHUTTER,"Shutter"},
  {FGP_ROI,"ROI"},
  {FGP_TRIGGERONOFF, "Trigger"},
  {FGP_EXTSHUTTER, "Shutt_uS"},
  {FGP_EXTSHUT_ADD, "Shutt_Perc" } ,
  {FGP_SEND_FINFO , "FrameInfo" } ,
  {FGP_TRIGGERDELAY , "TrigDelay_uS" } ,
  {FGP_GRAB , "Grab" },
  {FGP_GAIN,"Gain"}
};


// __forceinline int GetFormatId(int format)
// {
// 	for (int i=0; i<(sizeof(vFormats)/sizeof(Videoformats)); i++)
// 	{
// 		if (format==vFormats[i].vm) return i;
// 	}
// 	return -1;
// }

// __forceinline int GetFramerateId(int framerate)
// {
// 	for (int i=0; i<(sizeof(fRates)/sizeof(Framerates)); i++)
// 	{
// 		if (framerate==fRates[i].fr) return i;
// 	}
// 	return -1;
// }

// __forceinline int GetResolutionId(int resolution)
// {
// 	for (int i=0; i<(sizeof(cResolutions)/sizeof(Resolutions)); i++)
// 	{
// 		if (resolution==cResolutions[i].res) return i;
// 	}
// 	return -1;
// }


static FXLockObject idsInitLock;
static long idsInitCntr=0;
static bool idsDrvReady=false;
// static FGNODEINFO  nodeinfo[MAX_CAMERASNMB];

// __forceinline UINT32HL getFullGid(unsigned low, int camonbus)
// {
// 	UINT32HL retV={-1,-1};
// 	for (int i=0; i<camonbus; i++)
// 	{
// 		if (nodeinfo[i].Guid.Low==low)
// 		{
// 			retV=nodeinfo[i].Guid;
// 			break;
// 		}
// 	}
// 	return retV;
// }

__forceinline unsigned GetXSize( HIDS hCamera )
{
  IS_RECT rectAOI;
  INT nRet = is_AOI(hCamera, IS_AOI_IMAGE_GET_AOI, (void*)&rectAOI, sizeof(rectAOI));
  if (nRet == IS_SUCCESS)
    return rectAOI.s32Width;
  return 0;
}

__forceinline unsigned GetYSize( HIDS hCamera )
{
  IS_RECT rectAOI;
  INT nRet = is_AOI(hCamera, IS_AOI_IMAGE_GET_AOI, (void*)&rectAOI, sizeof(rectAOI));
  if (nRet == IS_SUCCESS)
    return rectAOI.s32Height;
  return 0;
}

__forceinline unsigned GetXOffset(HIDS hCamera)
{
  IS_RECT rectAOI;
  INT nRet = is_AOI(hCamera, IS_AOI_IMAGE_GET_AOI, (void*)&rectAOI, sizeof(rectAOI));
  if (nRet == IS_SUCCESS)
    return rectAOI.s32X;
  return 0;
}

__forceinline unsigned GetYOffset(HIDS hCamera)
{
  IS_RECT rectAOI;
  INT nRet = is_AOI(hCamera, IS_AOI_IMAGE_GET_AOI, (void*)&rectAOI, sizeof(rectAOI));
  if (nRet == IS_SUCCESS)
    return rectAOI.s32Y;
  return 0;
}

__forceinline unsigned GetMaxWidth(HIDS hCamera)
{
  SENSORINFO Info;
  UINT LastResult= is_GetSensorInfo(hCamera,&Info);
  if(LastResult==IS_SUCCESS)
    return Info.nMaxWidth;
  return 0;
}

__forceinline unsigned GetMaxHeight(HIDS hCamera)
{
  SENSORINFO Info;
  UINT LastResult= is_GetSensorInfo(hCamera,&Info);
  if(LastResult==IS_SUCCESS)
    return Info.nMaxHeight;
  return 0;
}


// __forceinline bool IsFormatSupported( HIDS hCamera , FG_COLORMODE& vf)
// {
// 	UINT32 Result,ImageFormat;
// 	Camera.SetParameter(FGP_ENUMIMAGEFORMAT,0);
// 	do
// 	{
// 		Result=Camera.GetParameter(FGP_ENUMIMAGEFORMAT,&ImageFormat);
// 		if(Result==FCE_NOERROR)
// 		{
// 			if ((unsigned)vf==IMGCOL(ImageFormat))
// 				return true;
// 		}
// 	}while(Result==FCE_NOERROR);
// 	return false;
// }

// __forceinline bool IsModeSupported(CFGCamera& Camera, FG_COLORMODE vf,FG_FRAMERATE fr,FG_RESOLUTION rs)
// {
// 	UINT32 Result,ImageFormat;
// 	Camera.SetParameter(FGP_ENUMIMAGEFORMAT,0);
// 	do
// 	{
// 		Result=Camera.GetParameter(FGP_ENUMIMAGEFORMAT,&ImageFormat);
// 		if(Result==FCE_NOERROR)
// 		{
// 			if (ImageFormat==MAKEIMAGEFORMAT(rs,vf,fr))
// 				return true;
// 		}
// 	}while(Result==FCE_NOERROR);
// 	return false;
// }


bool idsInit()
{
  FXAutolock al(idsInitLock);
  LONG res;
  InterlockedExchange(&res,idsInitCntr);
  if (res==0)
  {
    is_SetErrorReport( 0 , IS_ENABLE_ERR_REP ) ;
    int iVersion = is_GetDLLVersion() ;
    int build = iVersion & 0xFFFF;
    iVersion = iVersion >> 16;
    int minor = iVersion & 0xFF;
    iVersion = iVersion >> 8;
    int major = iVersion & 0xFF;   
    int iNumCameras = 0 ;
    res = is_GetNumberOfCameras( &iNumCameras ) ;
    if ( (res == IS_SUCCESS) || (iNumCameras > 0)  )
    {
      UEYE_CAMERA_LIST * p = (UEYE_CAMERA_LIST * )  new BYTE[
        sizeof(ULONG) + (iNumCameras * sizeof(UEYE_CAMERA_INFO)) ] ;
        if ( p )
        {
          p->dwCount = iNumCameras ;
          res = is_GetCameraList( p ) ;
          if ( res == IS_SUCCESS )
            idsDrvReady=true;
          delete[] p ;
        }
    }
  }
  InterlockedIncrement(&idsInitCntr);
  return idsDrvReady;
}

void idsDone()
{
  FXAutolock al(idsInitLock);
  LONG res;
  InterlockedDecrement(&idsInitCntr);
  InterlockedExchange(&res,idsInitCntr);
  if (res==0)
  {
    idsDrvReady=false;
  }
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IDS::IDS():
m_bFrameInfoOn(false)
{
  idsInit();
  m_hCam = 0 ;
  m_pImgMem = NULL ;
  m_iMemId = 0 ;
  m_SensorSize.cx = m_SensorSize.cy = 0 ;
  m_iMemPitch = 0 ;
  m_iCurrentFormat = -1 ;
  m_pImageFormats = NULL ;
  m_Format = -1 ;
  m_iShutterPercChange = 10 ;
  memset( &m_CurrentROI , 0 , sizeof(m_CurrentROI) ) ;
  m_bLiveVideo = false ;
  memset(&m_BMIH,0,sizeof(BITMAPINFOHEADER));
  m_hGrabEndEvent = CreateEvent( NULL , FALSE , FALSE , NULL ) ;
}

void IDS::ShutDown()
{
  m_bCameraRunning = false ;
  if (IsTriggerByInputPin())
    SetTriggerMode(0); // no trigger
  SetEvent( m_hGrabEndEvent ) ;
  Sleep(5) ;
  CloseHandle( m_hGrabEndEvent ) ;
  FXAutolock al(m_Lock);
  C1394Camera::ShutDown();
  if ( m_pImageFormats )
    delete m_pImageFormats ;

  idsDone();
}

bool IDS::DriverInit()
{
  if (!idsDrvReady) 
    return false;
  //UINT32      CamerasOnBus;
  FXAutolock al(idsInitLock);

  int iNumCameras = 0 ;
  m_LastError = is_GetNumberOfCameras( &iNumCameras ) ;
  if ( (m_LastError != IS_SUCCESS) || (iNumCameras <= 0)  )
  {
    SENDERR_2("!!! is_GetNumberOfCameras Error: IDS (%d) %s\n", iNumCameras ,
      GetErrorMessage(m_LastError));
    return false;
  }
  m_CamerasOnBus = iNumCameras ;
  UEYE_CAMERA_LIST * p = (UEYE_CAMERA_LIST * )  new BYTE
    [ sizeof(ULONG) + (iNumCameras * sizeof(UEYE_CAMERA_INFO)) ] ;
  if ( !p )
    return false ;
  p->dwCount = iNumCameras ;
  m_LastError = is_GetCameraList( p ) ;
  if ( m_LastError != IS_SUCCESS )
  {
    SENDERR_2("!!! is_GetCameraList Error: IDS (%d) %s\n", iNumCameras ,
      GetErrorMessage(m_LastError));
    return false;
  }

  char cName[17];
  for (unsigned i=0; i<m_CamerasOnBus; i++)
  {
    if ( (i == m_CurrentCamera)  ||  (m_CurrentCamera == -1) )
    {
      memcpy( cName , p->uci[i].Model , sizeof(p->uci[i].Model) ) ;
      m_CamerasInfo[i].name = cName;
      m_CamerasInfo[i].serialnmb = p->uci[i].dwCameraID ;
    }
  }
  m_CurrentROI = CRect(-1,-1,-1,-1) ;
  return true ;
}

bool IDS::CameraInit()
{
  if (m_CamerasOnBus==0) 
  {
    SENDERR_0("Error: No IDS Cameras found");
    return false;
  }
  if ( m_CurrentCamera < 0 )
  {
    SENDERR_0("Error: IDS Camera is not selected (m_Camera.Connect) " );
    return false;
  }
  if (CameraConnected())
    CameraClose();
  m_LastError = is_InitCamera( &m_hCam , NULL ) ;
  if ( m_LastError!= IS_SUCCESS )
  {
    //Check if GigE uEye SE needs a new starter firmware
    if (m_LastError == IS_STARTER_FW_UPLOAD_NEEDED)
    {
      //Calculate time needed for updating the starter firmware
      INT nTime;
      is_GetDuration (m_hCam, IS_SE_STARTER_FW_UPLOAD, &nTime);
      SENDWARN_2( "Warning: Camera %d FW update execution (%d ms)" ,
        m_hCam , nTime ) ;
      /*
      e.g. have progress bar displayed in separate thread
      */
      //Upload new starter firmware during initialization
      m_hCam =  m_hCam | IS_ALLOW_STARTER_FW_UPLOAD ;
      m_LastError = is_InitCamera (&m_hCam, NULL);
      SENDWARN_1( "Info: Camera %d FW update execution" ,
        m_hCam ) ;
    }
    SENDERR_1("Error: IDS Initialization (m_Camera.Connect) %s", 
      GetErrorMessage(m_LastError));
    return false;
  }
  m_CameraID.Format( "%s_%d" , m_CamerasInfo[m_CurrentCamera].name ,
    m_CamerasInfo[m_CurrentCamera].serialnmb );
  m_SensorSize.cx = GetMaxWidth( m_hCam ) ;
  m_SensorSize.cy = GetMaxHeight( m_hCam ) ;
  m_iPixelDepth = 8 ;
  m_pImgMem = NULL ;
  return BuildPropertyList();;    
}

bool IDS::AllocateImage()
{
  if ( !m_hCam )
    return false ;

  FXAutolock al(m_Lock) ;
  if ( m_pImgMem )
  {
    is_StopLiveVideo( m_hCam , IS_FORCE_VIDEO_STOP ) ;
    is_FreeImageMem( m_hCam , m_pImgMem , m_iMemId ) ;
    m_pImgMem = NULL ;
  }
  CorrectBMPIH() ;

  m_LastError = is_AllocImageMem( m_hCam , 
    m_BMIH.biWidth , m_BMIH.biHeight , 
    m_iPixelDepth = (m_BMIH.biBitCount * m_BMIH.biPlanes) ,
    &m_pImgMem , &m_iMemId ) ;
  if ( m_LastError != IS_SUCCESS )
  {
    SENDERR_1("Error: IDS memory alloc %s", 
      GetErrorMessage(m_LastError));
    return false ;
  }
  m_LastError = is_SetImageMem( m_hCam , m_pImgMem , m_iMemId ) ;
  if ( m_LastError != IS_SUCCESS )
  {
    SENDERR_1("Error: IDS can't set image mem %s", 
      GetErrorMessage(m_LastError));
    return false ;
  }
  m_LastError = is_GetImageMemPitch( m_hCam , &m_iMemPitch ) ;
  if ( m_LastError != IS_SUCCESS )
  {
    SENDERR_1("Error: IDS can't get pitch %s", 
      GetErrorMessage(m_LastError));
    return false ;
  }
  m_CurrentROI=CRect(0,0,m_BMIH.biWidth,m_BMIH.biHeight);
  return true ;
}

bool IDS::CorrectBMPIH()
{
  if ( !m_hCam  ||  !m_pImageFormats || (m_iCurrentResolution < 0) )
    return false ;

  m_BMIH.biWidth = m_pImageFormats->FormatInfo[m_iCurrentResolution].nWidth ;
  m_BMIH.biHeight = m_pImageFormats->FormatInfo[m_iCurrentResolution].nHeight ;
  if ( m_CurrentROI.left < 0 )
    m_CurrentROI.left = 0 ;
  if ( m_CurrentROI.top < 0 )
    m_CurrentROI.top = 0 ;
  if ( m_CurrentROI.left + m_CurrentROI.right > m_BMIH.biWidth )
    m_CurrentROI.right = m_BMIH.biWidth - m_CurrentROI.left ;
  if ( m_CurrentROI.top + m_CurrentROI.bottom > m_BMIH.biHeight )
    m_CurrentROI.bottom = m_BMIH.biHeight - m_CurrentROI.top ;

  if ( m_CurrentROI.right > 0 )
  {  
    if ( m_BMIH.biWidth > m_CurrentROI.right ) // instead of right we hold width
      m_BMIH.biWidth = m_CurrentROI.right ;
    if ( m_BMIH.biHeight > m_CurrentROI.bottom ) // instead if bottom we hold height
      m_BMIH.biHeight = m_CurrentROI.bottom ;
  }
  else
  {
    m_CurrentROI.right = m_BMIH.biWidth ;
    m_CurrentROI.bottom = m_BMIH.biHeight ;
  }
  m_Format = is_SetColorMode( m_hCam , IS_GET_COLOR_MODE ) ;
  switch( m_Format )
  {
  case IS_CM_MONO8    : 
  case IS_CM_SENSOR_RAW8:
    m_BMIH.biCompression=BI_Y8;
    m_BMIH.biBitCount=8;
    m_BMIH.biSizeImage=m_BMIH.biWidth*m_BMIH.biHeight;
    break;
    // 		case CM_YUV411:
    // 			m_BMIH.biCompression=BI_YUV411;
    // 			m_BMIH.biBitCount=12;
    // 			m_BMIH.biSizeImage=3*m_BMIH.biWidth*m_BMIH.biHeight/2;
    // 			break;
  case IS_CM_MONO16   : 
  case IS_CM_SENSOR_RAW16:
    m_BMIH.biCompression=BI_Y16;
    m_BMIH.biBitCount=16;
    m_BMIH.biSizeImage=2*m_BMIH.biWidth*m_BMIH.biHeight;
    break;
  case IS_CM_RGB8_PACKED:
  case IS_CM_BGR8_PACKED:
    m_BMIH.biCompression=0;
    m_BMIH.biBitCount=24;
    m_BMIH.biSizeImage=3*m_BMIH.biWidth*m_BMIH.biHeight;
    break ;
//   case IS_CM_UYVY_PACKED:  // YUV422
//     m_BMIH.biCompression=BI_UYVY;
//     m_BMIH.biBitCount=16;
//     m_BMIH.biSizeImage=2*m_BMIH.biWidth*m_BMIH.biHeight;
    break ;
    // 		case CM_YUV444:
    // 		case CM_RGB8  :
    // 		case CM_RGB16 :

  default: 
    m_BMIH.biSize=0;
    TRACE("!!! Unsupported format #%d\n", m_Format);
    SENDERR_1("!!! Unsupported format #%d",m_Format);
    return false;
  }
  return true ;
}

void IDS::CameraClose()
{
  if( m_hCam != 0 )
  {
    // Disable messages
    //     is_EnableMessage( m_hCam, IS_FRAME, NULL );

    is_SetExternalTrigger( m_hCam , IS_SET_TRIGGER_OFF ) ;
    is_StopLiveVideo( m_hCam, IS_WAIT );
    // Free the allocated buffer
    if( m_pImgMem != NULL )
      is_FreeImageMem( m_hCam, m_pImgMem, m_iMemId );

    m_pImgMem = NULL;

    // Close camera
    is_ExitCamera( m_hCam );
    m_hCam = NULL;
  }
}

bool IDS::BuildPropertyList()
{

  if ( m_hCam == NULL )
    return true ;

  CAMINFO Info ;
  m_LastError = is_GetCameraInfo( m_hCam , &Info ) ;
  if ( m_LastError != IS_SUCCESS )
  {
    SENDERR_1("Error: IDS can't get camera info %s", 
      GetErrorMessage(m_LastError));
    return false ;
  }
  int iNFormats = 0 ;
  m_LastError = is_ImageFormat( m_hCam , IMGFRMT_CMD_GET_NUM_ENTRIES , 
    &iNFormats , sizeof(iNFormats) );
  if ( m_LastError != IS_SUCCESS )
  {
    SENDERR_1("Error: IDS can't get # of image formats %s", 
      GetErrorMessage(m_LastError));
    return false ;
  }

  if ( m_pImageFormats )
    delete[] m_pImageFormats ;

  int iListSize = sizeof(UINT)*6 + iNFormats*sizeof(IMAGE_FORMAT_INFO) ;
  m_pImageFormats = (IMAGE_FORMAT_LIST *)new BYTE[ iListSize ] ;
  if ( !m_pImageFormats )
  {
    SENDERR_0("Error: IDS can't allocate format list");
    return false ;
  }
  memset( m_pImageFormats , 0 , iListSize ) ;
  m_pImageFormats->nSizeOfListEntry = sizeof(IMAGE_FORMAT_INFO) ;
  m_pImageFormats->nNumListElements = iNFormats ;

  m_LastError = is_ImageFormat( m_hCam , IMGFRMT_CMD_GET_LIST , 
    m_pImageFormats , iListSize );
  if ( m_LastError != IS_SUCCESS )
  {
    SENDERR_1("Error: IDS can't get image formats %s", 
      GetErrorMessage(m_LastError));
    return false ;
  }
  m_LastError = is_GetSensorInfo( m_hCam , &m_SensorInfo ) ;
  if ( m_LastError != IS_SUCCESS )
  {
    SENDERR_1("Error: IDS can't get sensor info %s", 
      GetErrorMessage(m_LastError));
    return false ;
  }
  m_LastError = is_GetAutoInfo ( m_hCam , &m_AutoInfo ) ;
  if ( m_LastError != IS_SUCCESS )
  {
    SENDERR_1("Error: IDS can't get auto info %s", 
      GetErrorMessage(m_LastError));
    return false ;
  }
  double dAutoSpeed , dDummy;
  m_LastError = is_SetAutoParameter (m_hCam, 
    IS_GET_AUTO_SPEED , &dAutoSpeed , &dDummy ) ;
  if ( m_LastError != IS_SUCCESS )
  {
    SENDERR_1("Error: IDS auto speed get : %s", GetErrorMessage(m_LastError));
  }
  dAutoSpeed = 100. ;
  m_LastError = is_SetAutoParameter (m_hCam, 
    IS_SET_AUTO_SPEED , &dAutoSpeed , &dDummy ) ;
  if ( m_LastError != IS_SUCCESS )
  {
    SENDERR_1("Error: IDS auto speed set : %s", GetErrorMessage(m_LastError));
  }

  m_Properties.RemoveAll();
  int propCount=sizeof(cProperties)/sizeof(CamProperties);
  CAMERA1394::Property P;
  for (int i=0; i<propCount; i++)
  {
    switch (cProperties[i].pr)
    {
    case FGP_IMAGEFORMAT:
      {
        FXString items,tmpS;
        int formatCount = sizeof(vFormats)/sizeof(vFormats[0]) ;
        for (int j=0; j<formatCount; j++)
        {
          tmpS.Format("%s(%d)," , vFormats[j].name , j );
          //             if ( j != 0 )
          //               items += ',' ;
          items+=tmpS;
        }
        if (items.GetLength())
        {
          P.name=cProperties[i].name;
          P.id=(unsigned)cProperties[i].pr;
          P.property.Format("ComboBox(%s(%s))",P.name,items);
          m_Properties.Add(P);
        }
        break;
      }
    case FGP_FRAMERATE:
      {
      }
      break ;
    case FGP_RESOLUTION:
      {

        FXString items,tmpS;
        int formatCount = iNFormats ;
        for (int j=0; j<formatCount; j++)
        {
          char * p = m_pImageFormats->FormatInfo[j].strFormatName ;
          char name[64] ;
          char * pDst = name ;
          do 
          {
            switch ( *p )
            {
              //case ' ': break ;
            case '(': *(pDst++)='-' ; break ;
            case ')': break ;
            default:
              *(pDst++) = *p ; break ;
            }
          } while ( *(p++) ) ;
          tmpS.Format("%s(%d)," , name , j 
            /*m_pImageFormats->FormatInfo[j].nFormatID*/ );
          //             if ( j != 0 )
          //               items += ',' ;
          items+=tmpS;
        }
        if (items.GetLength())
        {
          P.name=cProperties[i].name;
          P.id=(unsigned)cProperties[i].pr;
          P.property.Format("ComboBox(%s(%s))",P.name,items);
          m_Properties.Add(P);
        }
        break;
      }
    case FGP_ROI:
			P.name=cProperties[i].name;
			P.id=(unsigned)cProperties[i].pr;
			P.property.Format("EditBox(%s)",P.name);
			m_Properties.Add(P);
      break;
    case FGP_TRIGGERONOFF:
      {
				P.name=cProperties[i].name;
				P.id=(unsigned)cProperties[i].pr;
				P.property.Format(
          "ComboBox(%s(Off(0),TrHiLow(1),TrLowHig(2),TrSoft(3)))",
					P.name);
				m_Properties.Add(P);
        break;
      }
    case FGP_EXTSHUTTER:
      {
        P.name=cProperties[i].name;
        P.id=(unsigned)cProperties[i].pr;
        double dExp[3] ;
        m_LastError = is_Exposure( m_hCam , IS_EXPOSURE_CMD_GET_EXPOSURE_RANGE ,
          dExp , sizeof(dExp) ) ;
        if ( m_LastError == IS_SUCCESS )
        {
          P.property.Format("Spin&Bool(%s,%d,%d)", P.name ,
            (int)(dExp[0] * 1000.) , (int)(dExp[1] * 1000.) );
          m_Properties.Add(P);
        }

        //           UINT LongExpEna = 1 ;
        //           m_LastError = is_Exposure( m_hCam , IS_EXPOSURE_CMD_SET_LONG_EXPOSURE_ENABLE ,
        //             &LongExpEna , sizeof(LongExpEna) ) ;
        break;
      }
    case FGP_EXTSHUT_ADD:
      {
        P.name=cProperties[i].name;
        P.id=(unsigned)cProperties[i].pr;
        if ( m_LastError == IS_SUCCESS )
        {
          P.property.Format("Spin(%s,%d,%d)", P.name ,
            -50 , 50 );
          m_Properties.Add(P);
        }
        break;
      }    
    case FGP_SEND_FINFO:
      {
        // 					P.name=cProperties[i].name;
        // 					P.id=(unsigned)cProperties[i].pr;
        // 					P.property.Format("ComboBox(%s(Off(0),On(1)))", P.name );
        // 					m_Properties.Add(P);
        break;
      }
    case FGP_TRIGGERDELAY:
      {
        // 					P.name=cProperties[i].name;
        // 					P.id=(unsigned)cProperties[i].pr;
        // 					P.property.Format("Spin(%s,%d,%d)", P.name , 0 , 30000000 );
        // 					m_Properties.Add(P);
        break;
      }
    case FGP_GRAB:
      {
        P.name=cProperties[i].name;
        P.id=(unsigned)cProperties[i].pr;
        P.property.Format("Spin(%s,%d,%d)", P.name , -1 , 30000000 );
        m_Properties.Add(P);
      }
      break ; 
    case FGP_GAIN:
      {
        P.name=cProperties[i].name;
        P.id=(unsigned)cProperties[i].pr;
        P.property.Format("Spin&Bool(%s,%d,%d)", P.name , 0 , 100 );
        m_Properties.Add(P);
      }
      break ;
    case FGP_AUTOEXPOSURE:
      {
        P.name=cProperties[i].name;
        P.id=(unsigned)cProperties[i].pr;
        P.property.Format("Spin(%s,%d,%d)", P.name , 0 , 255 );
        m_Properties.Add(P);
      }
      break ;
    case FGP_BRIGHTNESS:
    case FGP_SHARPNESS: 
    case FGP_WHITEBALCB: 
    case FGP_WHITEBALCR: 
    case FGP_HUE:       
    case FGP_SATURATION: 
    case FGP_GAMMA:      
    case FGP_SHUTTER:    
    case FGP_IRIS:
    case FGP_FOCUS:
    case FGP_TEMPERATURE:   
    case FGP_WHITESHD:   
    case FGP_ZOOM:          
    case FGP_PAN:           
    case FGP_TILT:          
    case FGP_OPTICALFILTER:
    case FGP_CAPTURESIZE: 
    case FGP_CAPTUREQUALITY:
      break ;
    default:
      {
        SENDERR_1("Error: unsupported property %s", cProperties[i].name);
        break;
      }
    }
  }  
  if (m_LastError!=IS_SUCCESS)
    m_BMIH.biSize=0;
  else
  {
    m_BMIH.biSize=sizeof(BITMAPINFOHEADER);
    //     m_BMIH.biWidth=GetXSize(m_hCam);
    //     m_BMIH.biHeight=GetYSize(m_hCam);
    //     unsigned XOff=GetXOffset(m_hCam);
    //     unsigned YOff=GetYOffset(m_hCam);
    //     m_CurrentROI=CRect(XOff,YOff,XOff+m_BMIH.biWidth,YOff+m_BMIH.biHeight);
    m_BMIH.biPlanes=1;

    if ( CorrectBMPIH() )
    {
      return AllocateImage() ;
    }
  }
  return false ;
}

bool IDS::GetCameraProperty(unsigned i, FXSIZE &value, bool& bauto)
{
  bauto=false;
  switch (i)
  {
  case FGP_IMAGEFORMAT:
    {
      value = m_iCurrentFormat ;
      return true;
    }
  case FGP_FRAMERATE:
    {
      return false ;
    }
  case FGP_RESOLUTION:
    {
      value = m_iCurrentResolution ;
      return true ;
    }
  case FGP_ROI:
    {
			static FXString sROI;
			CRect rc;
			GetROI(rc);
			sROI.Format("%d,%d,%d,%d",rc.left,rc.top,rc.right,rc.bottom);
			value=(FXSIZE)(LPCTSTR)sROI;
			return true;
    }
  case FGP_TRIGGERONOFF:
    {
      value = GetTriggerMode() ;
      return true;
    }
  case FGP_EXTSHUTTER:
    {
      value = GetLongExposure( bauto ) ;
      return true ;
    }
  case FGP_EXTSHUT_ADD:
    {
      value = m_iShutterPercChange ;
      return true ;
    }
  case FGP_SEND_FINFO:
    {
      value=m_bFrameInfoOn;
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
      int iVal ;
      bool bRes = GetGrabConditions( bContinuous , iVal ) ;
      value = iVal ;
      return true;
    }
  case FGP_GAIN:
    {
      value = GetGain( bauto ) ;
      return true ;
    }
  case FGP_AUTOEXPOSURE:
    {
      double dValue , dDummy ;
      m_LastError = is_SetAutoParameter (m_hCam, 
        IS_GET_AUTO_REFERENCE , &dValue , &dDummy ) ;
      if ( m_LastError != IS_SUCCESS )
      {
        SENDERR_1("Error: IDS Auto ref get ", GetErrorMessage(m_LastError));
        return false;
      }
      value = ROUND( dValue ) ;
      return true ;
    }
  default:
    break;
  }
  return false;
}

bool IDS::SetCameraProperty(unsigned i, FXSIZE &value, bool& bauto, bool& Invalidate)
{
  bool res = false ;

  FG_PARAMETER Par = (FG_PARAMETER)i ;
  switch (Par)
  {
  case FGP_IMAGEFORMAT:
    {
      bool wasRunning=IsRunning();
      if (wasRunning)
        OnStop();
      m_LastError = is_SetColorMode( m_hCam , vFormats[ value ].IDSCompression ) ;
      if ( m_LastError == IS_SUCCESS )
      {
        m_iCurrentFormat = (int)value ;
        m_Format = vFormats[ value ].IDSCompression ;
        if ( BuildPropertyList() )
        {
          Invalidate=true;
          if (wasRunning)
            OnStart();
          return true ;
        }
      }
      return res;
    }
  case FGP_FRAMERATE:
    {
      // 				bool wasRunning=IsRunning();
      // 				if (wasRunning)
      // 					OnStop();
      // 				m_FrameRate=(FG_FRAMERATE)value;
      // 				bool res=
      // 					(m_Camera.SetParameter(FGP_IMAGEFORMAT,
      // 					MAKEIMAGEFORMAT(
      // 					m_Resolution,
      // 					m_Format,
      // 					m_FrameRate
      // 					)
      // 					)==FCE_NOERROR);
      // 				BuildPropertyList();
      // 				Invalidate=true;
      // 				if (wasRunning)
      // 					OnStart();
      return res;

    }
  case FGP_RESOLUTION:
    {
      if ( (UINT)value >= m_pImageFormats->nNumListElements )
        return false ;
      bool wasRunning=IsRunning();
      if (wasRunning)
      {
        OnStop();
      }
      int iResolution = m_pImageFormats->FormatInfo[value].nFormatID ;
      m_LastError = is_ImageFormat( m_hCam , IMGFRMT_CMD_SET_FORMAT ,
        &iResolution , sizeof(iResolution) ) ;
      if ( m_LastError == IS_SUCCESS )
      {
        m_iCurrentResolution = (int)value ;
        if ( BuildPropertyList() )
        {
          Invalidate=true;
          if (wasRunning)
            OnStart();
          return true ;
        }
      }
      return res;
    }
  case FGP_ROI:
    {
	    CRect rc;
	    if (sscanf((LPCTSTR)value,"%d,%d,%d,%d",&rc.left,&rc.top,&rc.right,&rc.bottom)==4)
	    {
		    res=SetROI(rc);
        Invalidate=true;
      }
      return res;
    }
  case FGP_TRIGGERONOFF:
    {
				SetTriggerMode( (int) value );
				return true;
    }
  case FGP_EXTSHUTTER:
    {
      SetLongExposure( (int) value , bauto ) ;
      return true ;
    }
  case FGP_EXTSHUT_ADD:
    {
      bool bAuto ;
      double dExp = GetLongExposure( bAuto ) ;
      if ( !bAuto )
      {
        dExp *= (100. + value)/100. ;
        int iExp = ROUND(dExp) ;
        if( iExp < 10)
          iExp = 10 ;
        if ( iExp > 10000000 )
          iExp = 10000000 ;
        
        SetLongExposure( iExp , bauto ) ;
      }
      m_iShutterPercChange = (int) value ;
      return true ;
    }
  case FGP_SEND_FINFO:
    {
      //         if (!IsFrameInfoAvailable())
      //         {
      //             m_bFrameInfoOn=false;
      //             return false;
      //         }
      // 				SetSendFrameInfo( value ) ;
      m_bFrameInfoOn=(value!=0);
      return true ;
    }
  case FGP_TRIGGERDELAY:
    {
      SetTriggerDelay( (int) value );
      return true;
    }
  case FGP_GRAB:
    {
      SetGrab( (int) value ) ;
      return true;
    }
  case FGP_GAIN:
    {
      SetGain( (int) value , bauto ) ;
      return true ;
    }
  case FGP_AUTOEXPOSURE:
    {
      double dValue = (double) value, dDummy ;
      m_LastError = is_SetAutoParameter (m_hCam, 
        IS_SET_AUTO_REFERENCE , &dValue , &dDummy ) ;
      if ( m_LastError != IS_SUCCESS )
      {
        SENDERR_1("Error: IDS Auto ref ", GetErrorMessage(m_LastError));
        return false;
      }
      return true ;
    }
  default:
    break;
  }
  return false;
}

bool IDS::CameraStart()
{
  if ( !m_hCam )
    return false ;
  m_LastError = is_InitEvent( m_hCam , m_hGrabEndEvent , IS_SET_EVENT_FRAME ) ;
  if ( m_LastError != IS_SUCCESS )
  {
    SENDERR_1("Error: IDS InitEvent %s", GetErrorMessage(m_LastError));
    return false;
  }
  m_LastError = is_EnableEvent( m_hCam , IS_SET_EVENT_FRAME ) ;
  if ( m_LastError != IS_SUCCESS )
  {
    SENDERR_1("Error: IDS EnableEvent: %s", GetErrorMessage(m_LastError));
    return false;
  }
  m_LastError = is_CaptureVideo( m_hCam , IS_DONT_WAIT ) ; 
  if ( m_LastError != IS_SUCCESS )
  {
    SENDERR_1("Error: IDS CAptureVideo: %s", GetErrorMessage(m_LastError));
    return false;
  }

  return true;
}

void IDS::CameraStop()
{
  if ( !m_hCam )
    return ;

  m_LastError = is_StopLiveVideo ( m_hCam,  IS_WAIT ) ;
  if ( m_LastError != IS_SUCCESS )
    SENDERR_1("Error on IDS StopLiveVideo: %s", GetErrorMessage(m_LastError));
  m_LastError = is_DisableEvent( m_hCam , IS_SET_EVENT_FRAME ) ;
  if ( m_LastError != IS_SUCCESS )
    SENDERR_1("Error on IDS DisableEvent: %s", GetErrorMessage(m_LastError));
  m_LastError = is_ExitEvent( m_hCam , IS_SET_EVENT_FRAME ) ;
  if ( m_LastError != IS_SUCCESS )
    SENDERR_1("Error on IDS ExitEvent: %s", GetErrorMessage(m_LastError));
}

CVideoFrame* IDS::GetCapturedFrame( double * pdStartTime )
{
  if ( !m_hCam )
    return NULL ;

  if (m_BMIH.biSize==0) 
  {
    Sleep(10);
    return NULL;
  }
  FXAutolock al(m_Lock);

  pTVFrame    frame = (pTVFrame)malloc(sizeof(TVFrame));
  frame->lpBMIH=NULL;
  frame->lpData=NULL;
  memcpy(&m_RealBMIH,&m_BMIH,m_BMIH.biSize);

  bool rescaled=false;
  int newWidth = m_iMemPitch ;
  int newHeight = m_BMIH.biHeight ;
  if ( (newHeight % 4) != 0 )
    newHeight = (newHeight + 4) & 0xfffffffc ;

  rescaled=true;
  int size = newHeight*newWidth ;
  switch( m_Format )
  {
  case IS_CM_MONO16:
  case IS_CM_SENSOR_RAW16:
  case IS_CM_UYVY_PACKED:
    size *= 2 ; break ;
  case IS_CM_RGB8_PACKED:
    size *= 3 ; break ;
  }
  m_RealBMIH.biSizeImage=size;
  m_RealBMIH.biWidth=newWidth;
  m_RealBMIH.biHeight=newHeight;

  switch( m_Format )
  {
  case IS_CM_MONO8:
  case IS_CM_SENSOR_RAW8:
    m_RealBMIH.biCompression = BI_Y8 ;
    frame->lpBMIH = (LPBITMAPINFOHEADER) malloc( sizeof(BITMAPINFOHEADER) + size ) ;
    memcpy( frame->lpBMIH , &m_RealBMIH , m_RealBMIH.biSize ) ;
    is_CopyImageMem( m_hCam , m_pImgMem , m_iMemId , (char*) &frame->lpBMIH[1] ) ;      
    break ;
  case IS_CM_RGB8_PACKED:
    m_RealBMIH.biCompression = BI_RGB ;
    frame->lpBMIH = rgb24yuv9( &m_RealBMIH , (LPBYTE) m_pImgMem ) ;
    break ;
//   case IS_SET_CM_UYVY:
//     m_RealBMIH.biCompression = BI_UYVY ;
//     frame->lpBMIH = YUV422
  default: return NULL ;
  }

  if ((frame)&&(frame->lpBMIH))
  {
    CVideoFrame* vf=CVideoFrame::Create(frame);
    vf->SetLabel(m_CameraID);
    return vf;
  }
  else if (frame)
    free(frame);
  return NULL;
}

bool IDS::IsTriggerByInputPin()
{
  if ( !m_hCam )
    return false ;
  UINT32 TriggerMode = is_SetExternalTrigger( m_hCam , IS_GET_EXTERNALTRIGGER ) ;
  return ( TriggerMode != IS_SET_TRIGGER_OFF ) ;
}
int IDS::GetTriggerMode()
{
  if ( !m_hCam )
    return -1 ;
  UINT32 TriggerMode = is_SetExternalTrigger( m_hCam , IS_GET_EXTERNALTRIGGER ) ;
  switch ( TriggerMode )
  {
  case IS_SET_TRIGGER_OFF: return 0 ; // no external trigger
  case IS_SET_TRIGGER_HI_LO: return 1 ; // High to low
  case IS_SET_TRIGGER_LO_HI: return 2 ; // Low to high
  case IS_SET_TRIGGER_SOFTWARE: return 3 ; // Software trigger
  default: return 0 ;
  }
}

void IDS::SetTriggerMode(int iMode)
{
  if ( !m_hCam )
    return ;
  // iMode : 0 - no trigger, 1 - Hi to Low , 2 - Low to High
  //                         3 - Software
  int iEnumMode = -1 ;
  switch( iMode )
  {
  case 0: iEnumMode = IS_SET_TRIGGER_OFF ; break ;
  case 1: iEnumMode = IS_SET_TRIGGER_HI_LO ; break ;
  case 2: iEnumMode = IS_SET_TRIGGER_LO_HI ; break ;
  case 3: iEnumMode = IS_SET_TRIGGER_SOFTWARE ; break ;
  }
  if ( iEnumMode != -1 )
    is_SetExternalTrigger( m_hCam , iEnumMode ) ;
}

void IDS::GetROI(CRect& rc)
{
  m_LastError = is_AOI( m_hCam , IS_AOI_IMAGE_GET_AOI , &rc , sizeof(rc) ) ;
  if ( m_LastError != IS_SUCCESS )
  {
    SENDERR_1("Error: IDS is_AOI: %s", GetErrorMessage(m_LastError));
  }
  m_CurrentROI = rc ;
}

bool IDS::SetROI(CRect& rc)
{
  if ( !m_hCam )
    return false ;
  bool wasRunning=IsRunning();
  BOOL bChangeSize = ( m_CurrentROI.BottomRight() != rc.BottomRight() ) ;
  if (wasRunning )           // width and height
    OnStop();
  if ( bChangeSize )
  m_LastError = is_AOI( m_hCam , IS_AOI_IMAGE_SET_AOI , &rc , sizeof(rc) ) ;
  else
    m_LastError = is_AOI( m_hCam , IS_AOI_IMAGE_SET_POS_FAST , &rc , sizeof(CPoint) ) ;
  if ( m_LastError != IS_SUCCESS )
  {
    SENDERR_1("Error: IDS is_AOI: %s", GetErrorMessage(m_LastError));
    return false;
  }
  m_CurrentROI = rc ;
  bool bRes = true ;
  if ( bChangeSize )
  {
    if ( CorrectBMPIH() )
      bRes = AllocateImage();
    else
      bRes = false ;
}
  if ( BuildPropertyList() )
{
    if (wasRunning )           // width and height
      OnStart() ;
  }

  return bRes ;
}
int IDS::GetLongExposure( bool& bAuto )
{
  if ( !m_hCam )
    return 0 ;
  double dAuto , dDummy ;
  m_LastError = is_SetAutoParameter (m_hCam, 
    IS_GET_ENABLE_AUTO_SHUTTER , &dAuto , &dDummy ) ;
  if ( m_LastError != IS_SUCCESS )
  {
    SENDERR_1("Error: IDS auto exposure get : %s", GetErrorMessage(m_LastError));
    dAuto = 0. ;
  }
  bAuto = dAuto > 0.5 ;
  double dExp ;
  m_LastError = is_Exposure( m_hCam , IS_EXPOSURE_CMD_GET_EXPOSURE ,
    &dExp , sizeof(dExp) ) ;
  if ( m_LastError != IS_SUCCESS )
  {
    SENDERR_1("Error: IDS is_Exposure get : %s", GetErrorMessage(m_LastError));
    return 0 ;
  }
  return (int)(dExp * 1000.) ;
}

void IDS::SetLongExposure( int iExp_usec , bool bAuto )
{
  if ( !m_hCam )
    return ;

  if ( bAuto )
  {
    double dAuto = (bAuto) ? 1.0 : 0.0 ;
    double dDummy ;
    m_LastError = is_SetAutoParameter (m_hCam, 
      IS_SET_ENABLE_AUTO_SHUTTER , &dAuto , &dDummy ) ;
    if ( m_LastError != IS_SUCCESS )
    {
      SENDERR_1("Error: IDS auto exposure set : %s", GetErrorMessage(m_LastError));
    }
  }
  else
  {
  double dExp = (double)iExp_usec / 1000. ;
  m_LastError = is_Exposure( m_hCam , IS_EXPOSURE_CMD_SET_EXPOSURE ,
    &dExp , sizeof(dExp) ) ;
  if ( m_LastError != IS_SUCCESS )
    SENDERR_1("Error: IDS is_Exposure set : %s", GetErrorMessage(m_LastError));
}
}

int IDS::GetGain( bool& bAuto )
{
  if ( !m_hCam )
    return 0 ;
  double dAuto , dDummy ;
  m_LastError = is_SetAutoParameter (m_hCam, 
    IS_GET_ENABLE_AUTO_GAIN , &dAuto , &dDummy ) ;
  if ( m_LastError != IS_SUCCESS )
  {
    SENDERR_1("Error: IDS auto gain get : %s", GetErrorMessage(m_LastError));
    dAuto = 0. ;
  }
  bAuto = dAuto > 0.5 ;

  return is_SetHardwareGain( m_hCam , IS_GET_MASTER_GAIN , 
    IS_IGNORE_PARAMETER , IS_IGNORE_PARAMETER , IS_IGNORE_PARAMETER );
}

void IDS::SetGain( int iGain , bool bAuto )
{
  if ( !m_hCam )
    return ;
  if ( bAuto )
  {
    double dAuto = (bAuto) ? 1.0 : 0.0 ;
    double dDummy ;
    m_LastError = is_SetAutoParameter (m_hCam, 
      IS_SET_ENABLE_AUTO_GAIN , &dAuto , &dDummy ) ;
    if ( m_LastError != IS_SUCCESS )
    {
      SENDERR_1("Error: IDS auto gain set : %s", GetErrorMessage(m_LastError));
    }
  }
  else
  {
  if ( m_SensorInfo.bRGain && m_SensorInfo.bGGain && m_SensorInfo.bBGain )
  {
    m_LastError = is_SetHardwareGain( m_hCam , iGain , iGain , iGain , iGain ) ;
    if ( m_LastError != IS_SUCCESS )
    {
      SENDERR_1("Error: IDS Gain set : %s", GetErrorMessage(m_LastError));
    }
  }  
  else if ( m_SensorInfo.bMasterGain )
  {
      m_LastError = is_SetHardwareGain( m_hCam , iGain , 
        IS_IGNORE_PARAMETER , IS_IGNORE_PARAMETER , IS_IGNORE_PARAMETER ) ;
     // m_LastError = is_SetHWGainFactor( m_hCam , IS_SET_MASTER_GAIN_FACTOR , iGain );
      if ( m_LastError != IS_SUCCESS )
      {
        SENDERR_1("Error: IDS Master Gain set : %s", GetErrorMessage(m_LastError));
      }
  }
  }
}

DWORD IDS::IsFrameInfoAvailable()
{
  return 0 ;
}

void IDS::SetSendFrameInfo( int iSend)
{

}

int IDS::GetTriggerDelay()
{
  UINT32 iDelay_usec = is_SetTriggerDelay( m_hCam , IS_GET_TRIGGER_DELAY ) ;
  return iDelay_usec ;
}

void IDS::SetTriggerDelay( int iDelay_uS)
{
  if ( !m_hCam )
    return ;
  is_SetTriggerDelay( m_hCam , iDelay_uS ) ;
}

bool IDS::SetGrab( int iNFrames )
{
  if ( !m_hCam )
    return false ;
  if (  (iNFrames < -1)  
    ||  (iNFrames > 0xffff) )
    return false ;
  if ( iNFrames == -1 )
    m_LastError = is_CaptureVideo( m_hCam , IS_DONT_WAIT ) ;
  else if ( iNFrames > 0 )
    m_LastError = is_FreezeVideo( m_hCam , IS_DONT_WAIT ) ;
  else 
    m_LastError = is_StopLiveVideo( m_hCam , IS_WAIT ) ;
  if ( m_LastError != IS_SUCCESS )
    SENDERR_1("Error: IDS Can't start/stop grab : %s", GetErrorMessage(m_LastError));
  else
    m_bLiveVideo = ( iNFrames == -1 ) ;
  m_iNorderedFrames = iNFrames ;
  m_bCameraRunning = iNFrames != 0 ;

  return ( m_LastError == IS_SUCCESS ) ;
}

bool IDS::GetGrabConditions( 
                            bool& bContinuous , int& iNRestFrames ) 
{
  bContinuous = m_bLiveVideo ;

  return true ;
}

CDataFrame* IDS::GetNextFrame(double* StartTime)
{
  CDataFrame* df=NULL;
  if (IsRunning())
  {
    HANDLE pEvents[] = { m_evExit, m_evSWTriggerPulse , m_hGrabEndEvent };
    DWORD cEvents = sizeof(pEvents) / sizeof(HANDLE);
    DWORD res = ::WaitForMultipleObjects(cEvents,pEvents, FALSE, INFINITE) ;
    switch( res )
    {
    case (WAIT_OBJECT_0 + 2) : 
      df = GetCapturedFrame(StartTime); 
      if ( (m_iNorderedFrames > 0)  &&  (--m_iNorderedFrames > 0) )
        is_FreezeVideo( m_hCam , IS_DONT_WAIT ) ;
      break ; // take ready image
    case (WAIT_OBJECT_0 + 1) : // order next grab
        if ( m_hCam )
          is_ForceTrigger( m_hCam ) ;
        break ;
    case WAIT_ABANDONED:
    case WAIT_OBJECT_0 : return NULL; // Exit
    } 
  }
  return df;
}
