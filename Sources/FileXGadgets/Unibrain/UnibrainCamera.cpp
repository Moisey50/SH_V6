// UnibrainCamera.cpp: implementation of the avt2_11 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Unibrain.h"
#include "UnibrainCamera.h"
#include <video\shvideo.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define THIS_MODULENAME "Unibrain"
#define CAMERAMAXNAMELENGTH 128

#pragma comment( lib, "ub1394.lib" )

IMPLEMENT_RUNTIME_GADGET_EX(Unibrain, C1394Camera, "Video.capture", TVDB400_PLUGIN_NAME);

#define ARR_SIZE(x) ( sizeof(x)/sizeof(x[0]))


typedef struct tagPixelFormats
{
  FIREi_PIXEL_FORMAT  vm;
  TCHAR *        name; 
  bool                supported;
} PixelFormats;

typedef struct tagFramerates
{
  FIREi_FPS           fr;
  TCHAR *        name; 
  TCHAR *        m7name;
}Framerates;

typedef struct tagResolutions
{
  FIREi_RES           res;
  TCHAR *        name; 
}Resolutions;

typedef struct tagCamProperties
{
  int                 pr;
  TCHAR *        name; 
}CamProperties;

FIREi_VIDEO_FORMAT VideoFormats[] =
{
  Format_0 , Format_1 , Format_2 , Format_6 , Format_7  
};

PixelFormats PixFormats[]=
{
  {Y_MONO,    _T("Y8"),true},
  {YUV_411,   _T("YUV411"), true},
  {YUV_422,   _T("YUV422"), false},
  {YUV_444,   _T("YUV444"),false},
  {RGB_24,    _T("RGB24"),false},
  {Y_MONO_16, _T("Y16"),true},
  {Y_MONO_16_SIGNED, _T("sY16"),false},
  {RGB_48,    _T("RGB48"),false},
  {RGB_48_SIGNED,_T("sRGB48"),false},
  {RAW_8,     _T("RAW8"),true},
  {RAW_16,    _T("RAW16"),false}
};

Framerates fRates[]= 
{ 
  {fps_1_875, _T("1.875 fps."),  _T("0")},
  {fps_3_75,  _T("3.75 fps."),   _T("1")},
  {fps_7_5,   _T("7.5 fps."),    _T("2")},
  {fps_15,    _T("15 fps."),     _T("3")},
  {fps_30,    _T("30 fps."),     _T("4")},
  {fps_60,    _T("60 fps."),     _T("5")},
  {fps_120,   _T("120 fps."),    _T("6")},
  {fps_240,   _T("240 fps."),    _T("7")}
};                                     

Resolutions cResolutions[]=
{
  {res_160x120,   _T("160x120"  ) },
  {res_320x240,   _T("320x240"  ) },
  {res_640x480,   _T("640x480"  ) },
  {res_800x600,   _T("800x600"  ) },
  {res_1024x768,  _T("1024x768" ) },
  {res_1280x960,  _T("1280x960" ) },
  {res_1600x1200, _T("1600x1200") },
  {res_variable,  _T("Scalable" ) }
}; 

CamProperties cProperties[] =
{
  {FGP_PIXELFORMAT  , "Format"},
  {FGP_FRAMERATE    , "FrameRate"},
  {FGP_RESOLUTION   , "Resolution"},
  {FGP_BRIGHTNESS   , "Brightness"},
  {FGP_AUTOEXPOSURE , "Auto_Exposure"},
  {FGP_SHARPNESS    , "Sharpness"},
  {FGP_WHITEBALCB   , "W.B.Blue"},
  {FGP_WHITEBALCR   , "W.B.Red"},
  {FGP_HUE          , "Hue"},
  {FGP_SATURATION   , "Saturation"},
  {FGP_GAMMA        , "Gamma"},
  {FGP_SHUTTER      , "Shutter"},
  {FGP_GAIN         , "Gain"},
  {FGP_ROI          , "ROI"},
  {FGP_TRIGGERONOFF , "Trigger"},
  // 	{FGP_EXTSHUTTER   , "Shutt_uS"},
  // 	{FGP_SEND_FINFO   , "FrameInfo" } ,
  // 	{FGP_TRIGGERDELAY , "TrigDelay_uS" } ,
  {FGP_GRAB         , "Grab" }
};

LPCTSTR GetPropertyName( int Prop )
{
  for ( int i = 0 ; i < ARR_SIZE(cProperties) ; i++ )
  {
    if ( cProperties[i].pr == Prop )
      return cProperties[i].name ;
  }
  return _T("Unknown") ;
}
FXArray<Bus1394>  Unibrain::m_Busses ;
AllCameras        Unibrain::m_AllCameras ;
// __forceinline int GetFormatId(int format)
// {
// 	for (int i=0; i<(sizeof(vFormats)/sizeof(PixelFormats)); i++)
// 	{
// 		if (format==vFormats[i].vm) return i;
// 	}
// 	return -1;
// }

#define _HANDLE_ERROR1( status, function ) \
  if( status != FIREi_STATUS_SUCCESS ) \
  \
  C1394_SENDERR_2("%s: %s\n",function, GetErrorMessage(status));  

TCHAR * GetResolutionAsString( FIREi_RES Res )
{
  for ( int iResCnt = 0 ; iResCnt < sizeof( cResolutions )/sizeof(cResolutions[0]) ; iResCnt++ )
  {
    if ( cResolutions[iResCnt].res == Res )
      return cResolutions[ iResCnt ].name ;
  }
  return _T("Res Unknown") ;
}

__forceinline int GetFramerateId(int framerate)
{
  for (int i=0; i<(sizeof(fRates)/sizeof(Framerates)); i++)
  {
    if (framerate==fRates[i].fr) return i;
  }
  return -1;
}

__forceinline int GetResolutionId(int resolution)
{
  for (int i=0; i<(sizeof(cResolutions)/sizeof(Resolutions)); i++)
  {
    if (resolution==cResolutions[i].res) return i;
  }
  return -1;
}

FXLockObject Unibrain::m_UnibrainInitLock;
long Unibrain::m_UnibrainInitCntr = 0;
bool Unibrain::m_UnibrainDrvReady=false;

__forceinline unsigned GetXSize( StartUpData& StartUpInfo )
{
  switch ( StartUpInfo.VideoFormat )
  {
  case Format_0:
    switch( StartUpInfo.VideoMode )
    {
    case Mode_0: return 160 ;
    case Mode_1: return 320 ;
    case Mode_2: 
    case Mode_3: 
    case Mode_4: 
    case Mode_5: 
    case Mode_6: return 640 ;
    default: break ;
    }
    break ;
  case Format_1:
    switch( StartUpInfo.VideoMode )
    {
    case Mode_0: 
    case Mode_1: 
    case Mode_2: 
    case Mode_6: return 800 ;
    case Mode_3: 
    case Mode_4: 
    case Mode_5: 
    case Mode_7: return 1024 ;
    default: break ;
    }
    break ;
  case Format_2:
    switch( StartUpInfo.VideoMode )
    {
    case Mode_0: 
    case Mode_1: 
    case Mode_2: 
    case Mode_6: return 1280 ;
    case Mode_3: 
    case Mode_4: 
    case Mode_5: 
    case Mode_7: return 1600 ;
    default: break ;
    }
    break ;

  }
  return 0;
}

__forceinline unsigned GetYSize( StartUpData& StartUpInfo )
{
  switch ( StartUpInfo.VideoFormat )
  {
  case Format_0:
    switch( StartUpInfo.VideoMode )
    {
    case Mode_0: return 120 ;
    case Mode_1: return 240 ;
    case Mode_2: 
    case Mode_3: 
    case Mode_4: 
    case Mode_5: 
    case Mode_6: return 480 ;
    default: break ;
    }
    break ;
  case Format_1:
    switch( StartUpInfo.VideoMode )
    {
    case Mode_0: 
    case Mode_1: 
    case Mode_2: 
    case Mode_6: return 600 ;
    case Mode_3: 
    case Mode_4: 
    case Mode_5: 
    case Mode_7: return 768 ;
    default: break ;
    }
    break ;
  case Format_2:
    switch( StartUpInfo.VideoMode )
    {
    case Mode_0: 
    case Mode_1: 
    case Mode_2: 
    case Mode_6: return 960 ;
    case Mode_3: 
    case Mode_4: 
    case Mode_5: 
    case Mode_7: return 1200 ;
    default: break ;
    }
    break ;
  }
  return 0;
}
__forceinline unsigned GetTargetPixelFormat( StartUpData& StartUpInfo )
{
  switch ( StartUpInfo.VideoFormat )
  {
  case Format_0:
    switch( StartUpInfo.VideoMode )
    {
    case Mode_0: 
    case Mode_1: 
    case Mode_2: 
    case Mode_3: 
    case Mode_4: return BI_YUV9 ; 
    case Mode_5: return BI_Y8 ;
    case Mode_6: return BI_Y16 ;
    default: break ;
    }
  case Format_1:
    switch( StartUpInfo.VideoMode )
    {
    case Mode_0: 
    case Mode_1: 
    case Mode_2: 
    case Mode_3: 
    case Mode_4: return BI_YUV9 ;
    case Mode_5: return BI_Y8 ;
    case Mode_6: 
    case Mode_7: return BI_Y16 ;
    default: break ;
    }
    break ;
  case Format_2:
    switch( StartUpInfo.VideoMode )
    {
    case Mode_0: 
    case Mode_1: 
    case Mode_2: 
    case Mode_3: 
    case Mode_4: return BI_YUV9 ; 
    case Mode_5: return BI_Y8 ;
    case Mode_6: 
    case Mode_7: return BI_Y16 ;
    default: break ;
    }
    break ;
  }
  return 0;
}

__forceinline unsigned GetXOffset( StartUpData& StartUpInfo )
{
  if ( StartUpInfo.VideoFormat == 7 )
  {
    return StartUpInfo.pFiFormat7StartupInfo->ushLeftImagePosition ;
  }
  return 0;
}

__forceinline unsigned GetYOffset( StartUpData& StartUpInfo )
{
  if ( StartUpInfo.VideoFormat == 7 )
  {
    return StartUpInfo.pFiFormat7StartupInfo->ushTopImagePosition ;
  }
  return 0;
}

// __forceinline unsigned GetMaxWidth( CameraIdentity& Param )
// {
//   if ( StartUpInfo.VideoFormat == 7 )
//   {
//     return StartUpInfo.pFiFormat7StartupInfo->ushMaxImageHSize ;
//   }
// 	return 0;
// }
// 
// __forceinline unsigned GetMaxHeight( CameraIdentity& Param )
// {
//   if ( StartUpInfo.VideoFormat == 7 )
//   {
//     return StartUpInfo.pFiFormat7StartupInfo->ushMaxImageVSize ;
//   }
// 	return 0;
// }

int Unibrain::FindSupportedModeAndFormat( 
  FIREi_PIXEL_FORMAT PixForm ,FIREi_FPS fps, FIREi_RES rs)
{
  for ( int i = 0 ; i < m_SupportedCombinations.GetCount() ; i++ )
  {
    if ( m_SupportedCombinations[i].m_PixelFormat == PixForm 
      && m_SupportedCombinations[i].m_FPS == fps
      && m_SupportedCombinations[i].m_Resolution == rs )
    {
      return i ;
    }
  }

  return -1 ;
}

int Unibrain::FindSupportedModeAndFormat( FIREi_PIXEL_FORMAT PixForm )
{
  FIREi_RES FoundRes = res_none ;
  int iFoundIndex = -1 ;
  for ( int i = 0 ; i < m_SupportedCombinations.GetCount() ; i++ )
  {
    if ( m_SupportedCombinations[i].m_PixelFormat == PixForm )
    {
      if ( m_SupportedCombinations[i].m_Resolution > FoundRes )
      {
        iFoundIndex = i ;
        FoundRes = m_SupportedCombinations[i].m_Resolution ;
      }
    }
  }

  return iFoundIndex ;
}

FIREi_PIXEL_FORMAT Unibrain::GetPixFormat( StartUpData& StartupInfo )
{
  FIREi_VIDEO_FORMAT Format = StartupInfo.VideoFormat ;
  FIREi_VIDEO_MODE Mode = StartupInfo.VideoMode ;
  for ( int i = 0 ; i < m_SupportedCombinations.GetCount() ; i++ )
  {
    if ( m_SupportedCombinations[i].m_VideoFormat == Format 
      && m_SupportedCombinations[i].m_VideoMode == Mode )
    {
      return m_SupportedCombinations[i].m_PixelFormat ;
    }
  }
  return ILLEGAL_PIXEL_FORMAT ;
}

FIREi_RES Unibrain::GetResolution( StartUpData& StartupInfo )
{
  FIREi_VIDEO_FORMAT Format = StartupInfo.VideoFormat ;
  FIREi_VIDEO_MODE Mode = StartupInfo.VideoMode ;
  //   if ( Format != Format_7 )
  //   {
  for ( int i = 0 ; i < m_SupportedCombinations.GetCount() ; i++ )
  {
    FVPR_Combination& ThisCombin = m_SupportedCombinations.GetAt(i) ;
    if ( ThisCombin.m_VideoFormat == Format 
      && ThisCombin.m_VideoMode == Mode )
    {
      return m_SupportedCombinations[i].m_Resolution ;
    }
  }
  //   }
  return res_none ;
}

int Unibrain::FormSupportedResolutions( FIREi_PIXEL_FORMAT PixForm )
{
  m_SupportedResolutions.RemoveAll() ;
  for ( int i = 0 ; i < m_SupportedCombinations.GetCount() ; i++ )
  {
    FVPR_Combination& ThisCombin = m_SupportedCombinations.GetAt(i) ;
    if ( ThisCombin.m_PixelFormat == PixForm )
    {
      int iResCntr = 0 ;
      for ( ; iResCntr < m_SupportedResolutions.GetCount() ; iResCntr++ )
      {
        if ( m_SupportedResolutions[iResCntr] == ThisCombin.m_Resolution )
          break ; // nothing to add, we have this resolution
      }
      if ( iResCntr >= m_SupportedResolutions.GetCount() )
        m_SupportedResolutions.Add( ThisCombin.m_Resolution ) ;
    }
  }
  return (int) m_SupportedResolutions.GetCount() ;
}

int Unibrain::FormSupportedFPS( FIREi_PIXEL_FORMAT PixForm , FIREi_RES Resol )
{
  m_SupportedFPS.RemoveAll() ;
  for ( int i = 0 ; i < m_SupportedCombinations.GetCount() ; i++ )
  {
    FVPR_Combination& ThisCombin = m_SupportedCombinations.GetAt(i) ;
    if ( ThisCombin.m_PixelFormat == PixForm 
      && ThisCombin.m_Resolution == Resol )
    {
      int iFPSCntr = 0 ;
      for ( ; iFPSCntr < m_SupportedFPS.GetCount() ; iFPSCntr++ )
      {
        if ( m_SupportedFPS[iFPSCntr] == ThisCombin.m_FPS )
          break ; // nothing to add, we have this FPS
      }
      if ( iFPSCntr >= m_SupportedFPS.GetCount() )
        m_SupportedFPS.Add( m_SupportedCombinations[i].m_FPS ) ;
    }
  }
  return (int) m_SupportedFPS.GetCount() ;
}


bool Unibrain::FormPixelFormat_Resolution_FPS()
{
  if ( m_CurrentCamera >= (UINT)m_AllCameras.GetCount() )
    return false ;

  FXString items,tmpS;
  int SuppFormatCnt = 0 ;
  m_SupportedVideoFormats.RemoveAll() ;
  m_SupportedCombinations.RemoveAll() ;
  FIREi_VIDEO_FORMAT_INFO  FormatInfo;
  memset( &m_SupportedFormat , 0 , sizeof(m_SupportedFormat) ) ;
  m_LastError = FiQueryCameraRegister( m_hSelectedCamera , 
    OID_CAMERA_VIDEO_FORMATS , &m_SupportedFormat , sizeof(m_SupportedFormat) ) ;
  int iVariableResCnt = 0 ;
  for ( int iFormatCnt = 0 ; iFormatCnt < 8 ; iFormatCnt++ )
  {
    if ( iFormatCnt > 2  &&  iFormatCnt < 6 )
      continue ;

    if ( m_LastError!=FIREi_STATUS_SUCCESS )
    {
      C1394_SENDERR_2("%s Formats Query ERROR: %s", (LPCTSTR)m_CamerasInfo[ m_CurrentCamera ].name , GetErrorMessage(m_LastError));
      return false ;
    }
    if ( m_SupportedFormat.uFormat & (0x80 >> iFormatCnt) )  // is format supported?
    {
      m_SupportedVideoFormats.Add( (FIREi_VIDEO_FORMAT) iFormatCnt ) ;
      FIREi_CAMERA_SUPPORTED_MODE& SuppMode = m_SupportedFormat.SupportedMode[iFormatCnt] ;
      memset( &FormatInfo, 0 , sizeof(FIREi_VIDEO_FORMAT_INFO) );
      /* Get all the Digital Camera Spec info for the current format */
      FiInitFormatInfo( &FormatInfo , (FIREi_VIDEO_FORMAT)iFormatCnt );
      for ( int iModeCnt = 0 ; iModeCnt < 8 ; iModeCnt++ )
      {        // Is mode supported for this format
        if ( SuppMode.uMode & ( 0x80 >> iModeCnt ) ) 
        {
          m_SupportedVideoModes.Add( (FIREi_VIDEO_MODE) iModeCnt ) ;
          FIREi_VIDEO_MODE_INFO& ModeInfo =FormatInfo.Mode[ iModeCnt ] ; 
          FIREi_PIXEL_FORMAT PixFormat = ModeInfo.PixelFormat ;
          FIREi_RES Res = res_none ;
          if ( iFormatCnt != 7 )
          {
            switch( FormatInfo.Mode[ iModeCnt ].uWidth )
            {
            case 160: Res = res_160x120 ; break ;
            case 320: Res = res_320x240 ; break ;
            case 640: Res = res_640x480 ; break ;
            case 800: Res = res_800x600 ; break ;
            case 1024: Res = res_1024x768 ; break ;
            case 1280: Res = res_1280x960 ; break ;
            case 1600: Res = res_1600x1200 ; break ;
            default: Res = res_variable ; break ;
            }
            for ( int iFPSScan = 0 ; SuppMode.FrameRate[iModeCnt]  &&  iFPSScan < 8 ; iFPSScan++ )
            {
              if ( SuppMode.FrameRate[iModeCnt] & ( 0x80 >> iFPSScan) )
              {
                FIREi_FPS FPS = ModeInfo.FpsInfo[iFPSScan].fps ;
                FVPR_Combination NewCombination( FormatInfo.VideoFormat , 
                  ModeInfo.VideoMode , 
                  ModeInfo.PixelFormat , Res , FPS ) ;
                m_SupportedCombinations.Add( NewCombination ) ;
              }
            }
          }
          else
          {
            FIREi_CAMERA_FORMAT_7_REGISTERS regs ;
            regs.Tag                   = FIREi_CAMERA_FORMAT_7_REGISTERS_TAG;
            regs.Mode                  = (FIREi_VIDEO_MODE) iModeCnt;
            regs.TransmitSpeed         = S400 ;
            regs.pFiFormat7StartupInfo = NULL ;
            m_LastError = FiQueryCameraRegister(m_hSelectedCamera, OID_FORMAT_7_REGISTERS, &regs, sizeof(regs));
            ShowError( "QueryFormat7Registers" , true ) ;
            for ( int iPixScan = 0 ; iPixScan < 8 ; iPixScan++ )
            {
              if ( regs.SupportedPixelFormat & (0x80000000 >> iPixScan) )
              {
                FIREi_PIXEL_FORMAT PixForm = (FIREi_PIXEL_FORMAT)iPixScan ;
                CSize MaxImageSize( regs.ushMaxImageHSize , regs.ushMaxImageVSize ) ;
                FVPR_Combination NewCombination( (FIREi_VIDEO_FORMAT)iFormatCnt , 
                  ModeInfo.VideoMode , 
                  PixForm , 
                  (FIREi_RES)((int)res_variable + iVariableResCnt * 100) , 
                  fps_none , &MaxImageSize ) ;
                m_SupportedCombinations.Add( NewCombination ) ;
              }
            }
            iVariableResCnt++ ;
          }
        }
      }
    }
  }
  m_SupportedPixelFormats.RemoveAll() ;
  for ( int i = 0 ; i < m_SupportedCombinations.GetCount() ; i++ )
  {
    FIREi_PIXEL_FORMAT PixForm = m_SupportedCombinations[i].m_PixelFormat ;
    int iPFormCnt = 0 ;
    for ( ; iPFormCnt < m_SupportedPixelFormats.GetCount() ; iPFormCnt++ )
    {
      if ( PixForm == m_SupportedPixelFormats[iPFormCnt] )
        break ;
    }
    if ( iPFormCnt >= m_SupportedPixelFormats.GetCount() )
      m_SupportedPixelFormats.Add( PixForm ) ;
  }
  return true ;
}

bool Unibrain::UnibrainInit()
{
  FXAutolock al(m_UnibrainInitLock);
  if ( !m_UnibrainInitCntr )
  {
    UINT32 LastResult=FiInitialize();
    if (LastResult!=FIREi_STATUS_SUCCESS)
    {
      TRACE("!!! UnibrainInit error\n");
      SENDERR_1("Error: Unibrain Camera Initialization error %s", GetErrorMessage(LastResult));
      return false;
    }
    m_UnibrainDrvReady = true;
  }
  m_hIsochEngine = NULL ;
  m_hSelectedCamera= NULL ;
  m_UnibrainInitCntr++ ;
  return m_UnibrainDrvReady;
}

void Unibrain::UnibrainDone()
{
  FXAutolock al(m_UnibrainInitLock);
  if ( --m_UnibrainInitCntr == 0 )
  {
    FiTerminate();
    m_UnibrainDrvReady=false;
  }
}

/*-----------------------------------------------------------------------------------------------*/
bool Unibrain::GetCameraDescription( CameraIdentity& Cam )
{
  if ( !Cam.m_hCamera )
    return false ;

  char                szTmp[128];
  FIREi_STATUS        FireiStatus;

  FireiStatus = FiQueryCameraRegister( Cam.m_hCamera, 
    OID_VENDOR_NAME,
    szTmp,
    sizeof(szTmp));

  if(FIREi_STATUS_SUCCESS != FireiStatus)
    return false ;
  Cam.m_Name = szTmp ;
  Cam.m_Name += _T(" ") ;

  FireiStatus = FiQueryCameraRegister( Cam.m_hCamera, 
    OID_MODEL_NAME,
    szTmp,
    sizeof(szTmp));

  if(FIREi_STATUS_SUCCESS != FireiStatus)
    return false;

  Cam.m_Name += szTmp ;
  Cam.m_Name += _T(" ") ;

  ULONG CamSN = 0 ;
  FireiStatus = FiQueryCameraRegister( Cam.m_hCamera, 
    OID_CAMERA_SERIAL_NUMBER,
    &CamSN,
    sizeof(CamSN) );

  if(FIREi_STATUS_SUCCESS != FireiStatus)
    return false;

  sprintf_s( szTmp , "%u" , CamSN ) ;
  Cam.m_Name += szTmp ;
  Cam.m_SerialNumber = CamSN ;

  return true;
}
/*-----------------------------------------------------------------------------------------------*/
int Unibrain::InitCameraArray()
{
  FIREi_CAMERA_GUID   CamGuidArray[64];

  FXAutolock al(m_UnibrainInitLock);
  ZeroMemory(CamGuidArray,sizeof(CamGuidArray));

  m_AllCameras.RemoveAll() ;
  m_CamerasOnBus = 0 ;
  for ( int iBus = 1 ; iBus <= 16 ; iBus++)
  {
    ULONG               uCameras = 64 ;
    m_LastError = FiLocateCamerasEx( iBus , CamGuidArray , FIREi_LOCATE_ALL_CAMERAS , &uCameras ) ;
    if( m_LastError == FIREi_STATUS_SUCCESS    &&  uCameras > 0 )
    {
      for(ULONG i = 0 ; i < uCameras ; i++ )
      {
        CameraIdentity NewCamera ;
        m_LastError = FiOpenCameraHandleEx( iBus , &NewCamera.m_hCamera , &CamGuidArray[i] );
        if( ShowError( "FiOpenCameraHandleEx" , true  ) )
        {
          if ( GetCameraDescription( NewCamera ) )
          {
            if ( i == 0 )  // the first camera on the bus
            {
              ULONG NChannels = 0 ;
              m_LastError = FiQueryCameraRegister( NewCamera.m_hCamera , 
                OID_ISO_CHANNELS_SUPPORTED , &NChannels , sizeof( NChannels) ) ;
              if ( m_LastError == FIREi_STATUS_SUCCESS )
              {
                if ( NChannels )
                {
                  m_Busses.SetSize( m_Busses.GetSize() + 1 ) ;
                  m_Busses[m_Busses.GetUpperBound()].m_ulBusNumber = iBus ;
                  for ( ULONG iChanNum = 0 ; iChanNum < NChannels ; iChanNum++ )
                  {
                    m_Busses[m_Busses.GetUpperBound()].m_IsoChannels.Add( 0 ) ;
                  }
                }
              }
              else
                ShowError( _T("# of Iso channels check") , true ) ;
            }

            FiCloseCameraHandle( NewCamera.m_hCamera ) ;
            NewCamera.m_GUID = CamGuidArray[i] ;
            NewCamera.m_hCamera = 0 ;
            NewCamera.m_StartUpInfo.m_iAdapterNumber = iBus ;
            m_AllCameras.Add( NewCamera ) ;
          }
        }
      }
    }
  }


  for ( int i = 0 ; i < m_AllCameras.GetCount() ; i++ )
  {
    FXString Name = m_CamerasInfo[i].name ;
    m_CamerasInfo[i].name.Empty() ;
    m_CamerasInfo[i].name = m_AllCameras[i].m_Name ;
    m_CamerasInfo[i].serialnmb = m_AllCameras[i].m_SerialNumber ;
  }

  m_CamerasOnBus = (UINT)m_AllCameras.GetCount() ;
  return m_CamerasOnBus ;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Unibrain::Unibrain():
m_bFrameInfoOn(false)
{
  m_GadgetInfo = "Unibrain gadget";
  UnibrainInit();
  m_ulSerialNumber = 0 ;
  memset( &m_BMIH , 0 , sizeof(BITMAPINFOHEADER) );
  memset( &m_RealBMIH , 0 , sizeof(BITMAPINFOHEADER) ) ;
  memset( &m_StartUpInfoForMode7 , 0 , sizeof(m_StartUpInfoForMode7) );
  m_StartUpInfo.pFiFormat7StartupInfo = &m_StartUpInfoForMode7 ;

  m_Brightness.m_OIDControl = OID_BRIGHTNESS_CONTROL ;
  m_Brightness.m_OIDInq = OID_BRIGHTNESS_INQ ;
  m_Brightness.m_PropertyEnum = FGP_BRIGHTNESS ;
  m_Brightness.m_Name = _T("Brightness") ;

  m_Exposure.m_OIDControl = OID_AUTO_EXPOSURE_CONTROL ;
  m_Exposure.m_OIDInq = OID_AUTO_EXPOSURE_INQ ;
  m_Exposure.m_PropertyEnum = FGP_AUTOEXPOSURE ;
  m_Exposure.m_Name = _T("Exposure") ;

  m_Sharpness.m_OIDControl = OID_SHARPNESS_CONTROL ;
  m_Sharpness.m_OIDInq = OID_SHARPNESS_INQ ;
  m_Sharpness.m_PropertyEnum = FGP_SHARPNESS ;
  m_Sharpness.m_Name = _T("Sharpness") ;

  m_WBB.m_OIDControl = OID_UB_CONTROL ;
  m_WBB.m_OIDInq = OID_UB_INQ ;
  m_WBB.m_PropertyEnum = FGP_WHITEBALCB ;
  m_WBB.m_Name = _T("WBB") ;

  m_WBR.m_OIDControl = OID_VR_CONTROL ;
  m_WBR.m_OIDInq = OID_VR_INQ ;
  m_WBR.m_PropertyEnum = FGP_WHITEBALCR ;
  m_WBR.m_Name = _T("WBR") ;

  m_HUE.m_OIDControl = OID_HUE_CONTROL ;
  m_HUE.m_OIDInq = OID_HUE_INQ ;
  m_HUE.m_PropertyEnum = FGP_HUE ;
  m_HUE.m_Name = _T("HUE") ;

  m_Saturation.m_OIDControl = OID_SATURATION_CONTROL ;
  m_Saturation.m_OIDInq = OID_SATURATION_INQ ;
  m_Saturation.m_PropertyEnum = FGP_SATURATION ;
  m_Saturation.m_Name = _T("Saturation") ;

  m_Gamma.m_OIDControl = OID_GAMMA_CONTROL ;
  m_Gamma.m_OIDInq = OID_GAMMA_INQ ;
  m_Gamma.m_PropertyEnum = FGP_GAMMA ;
  m_Gamma.m_Name = _T("Gamma") ;

  m_Shutter.m_OIDControl = OID_SHUTTER_CONTROL ;
  m_Shutter.m_OIDInq = OID_SHUTTER_INQ ;
  m_Shutter.m_PropertyEnum = FGP_SHUTTER ;
  m_Shutter.m_Name = _T("Shutter") ;

  m_Gain.m_OIDControl = OID_GAIN_CONTROL ;
  m_Gain.m_OIDInq = OID_GAIN_INQ ;
  m_Gain.m_PropertyEnum = FGP_GAIN ;
  m_Gain.m_Name = _T("Gain") ;
}

bool Unibrain::CamRegGet( UnibrainParam& Param , int& iVal , bool& bAuto )
{
  if ( m_CurrentCamera >= (UINT)m_AllCameras.GetSize() ||  !m_hSelectedCamera )
    return 0 ;

  int iLastVal = Param.m_Reg.ushCurValue ;
  bool bLastAuto = Param.m_Reg.bSetAuto != FALSE ;
  m_LastError = Param.GetValue( m_hSelectedCamera , iVal , bAuto ) ;
  if ( m_LastError != FIREi_STATUS_SUCCESS )
  {
    LPCTSTR PropName =  GetPropertyName(Param.m_PropertyEnum) ;
    LPCTSTR ErrorName = ( m_LastError == 0xffffffff ) ? 
      _T("No Such Property") : GetErrorMessage( m_LastError ) ;
    TRACE("Get %s Error: %s", PropName , ErrorName );
    //     C1394_SENDERR_2("Get %s Error: %s", PropName , ErrorName );
    iVal = iLastVal ;
    bAuto = bLastAuto ;
    return false ;
  }
  return true ;
}
bool Unibrain::CamRegSet( UnibrainParam& Param , int iVal , bool bAuto )
{
  if ( m_CurrentCamera >= (UINT)m_AllCameras.GetSize() ||  !m_hSelectedCamera )
    return 0 ;
  m_LastError = Param.SetValue( m_hSelectedCamera , iVal , bAuto ) ;
  if ( m_LastError != FIREi_STATUS_SUCCESS )
  {
    LPCTSTR PropName =  GetPropertyName(Param.m_PropertyEnum) ;
    LPCTSTR ErrorName = ( m_LastError == 0xffffffff ) ? 
      _T("No Such Property") : GetErrorMessage( m_LastError ) ;
    TRACE("Set %s Error: %s", PropName , ErrorName );
    //     C1394_SENDERR_2("Set %s Error: %s", PropName , ErrorName );
    return false ;
  }
  return true ;
}


void Unibrain::ShutDown()
{
  if (IsTriggerByInputPin())
    SetTriggerMode(0); // no trigger
  C1394Camera::ShutDown();
  UnibrainDone();
}

bool Unibrain::DriverInit()
{

  if (!m_UnibrainDrvReady) 
    return false;

  return  EnumCameras() ;
}

bool Unibrain::EnumCameras()
{
  if ( !InitCameraArray() ) 
    return false ;
  if ( m_CurrentCamera < (UINT)m_AllCameras.GetCount() )
  {
    if ( !m_hSelectedCamera )
      return CameraInit() ;
    else
      return BuildPropertyList() ;
  }
  return (m_CamerasOnBus > 0) ;
}

bool Unibrain::CameraInit()
{
  FXAutolock al( m_UnibrainInitLock ) ;
  if (m_CamerasOnBus==0) 
  {
    C1394_SENDERR_0("Error: No Unibrain Cameras found on a bus");
    return false;
  }
  bool bCameraFound = false ;
  if ( m_CurrentCamera >= (UINT)m_AllCameras.GetCount() )
  {
    C1394_SENDERR_1("Error: No Camera %d" , m_CurrentCamera );
    return false;
  }
  if ( m_ulSerialNumber != m_AllCameras[(int) m_CurrentCamera ].m_SerialNumber )
  {
    if ( m_hSelectedCamera )
    {
      m_LastError = FiCloseCameraHandle( m_hSelectedCamera ) ;
      ShowError( "CameraInit: Close Previous Camera Handle ERROR" , true ) ;
      m_hSelectedCamera = NULL ;
    }
    m_StartUpInfo.Tag = 0 ;
    m_StartUpInfoForMode7.Tag = 0 ;
  }
  m_ulSerialNumber = m_AllCameras[ (int)m_CurrentCamera ].m_SerialNumber ;
  m_StartUpInfo.m_iAdapterNumber = m_AllCameras[ (int) m_CurrentCamera ].m_StartUpInfo.m_iAdapterNumber ;
  CameraIdentity& SelectedCam = m_AllCameras.GetAt( m_CurrentCamera ) ;

  LPCTSTR CamName = m_CamerasInfo[ m_CurrentCamera ].name ; 
  if ( SelectedCam.m_hCamera )
  {
    C1394_SENDERR_1("Camera %s already in use", GetErrorMessage(m_LastError));
    return false;
  }
  m_LastError = FiOpenCameraHandleEx( m_StartUpInfo.m_iAdapterNumber , 
    &SelectedCam.m_hCamera , &SelectedCam.m_GUID ) ;
  if (m_LastError!=FIREi_STATUS_SUCCESS)
  {
    C1394_SENDERR_1("FiOpenCameraHandleEx ERROR: %s", GetErrorMessage(m_LastError));
    SelectedCam.m_hCamera = NULL ;
    m_AllCameras[ (int) m_CurrentCamera ].m_hCamera = NULL ;
    return false;
  }
  m_hSelectedCamera = SelectedCam.m_hCamera ;

  //   m_StartUpInfo.Tag = FIREi_CAMERA_STARTUP_INFO_TAG ;
  //   m_LastError = FiGetCameraStartupInfo( m_hSelectedCamera , &m_StartUpInfo ) ;
  //   if ( m_LastError!=FIREi_STATUS_SUCCESS )
  //   {
  //     C1394_SENDERR_1("FiGetCameraStartupInfo ERROR: %s", GetErrorMessage(m_LastError));
  //     FiCloseCameraHandle( m_hSelectedCamera ) ;
  //     SelectedCam.m_hCamera = 0 ;
  //     m_hSelectedCamera = 0 ;
  //     return false;
  //   }
  memset( &m_SupportedFormat , 0 , sizeof(m_SupportedFormat) ) ;
  m_LastError = FiQueryCameraRegister( m_hSelectedCamera , 
    OID_CAMERA_VIDEO_FORMATS , &m_SupportedFormat , sizeof(m_SupportedFormat) ) ;
  if ( m_LastError!=FIREi_STATUS_SUCCESS )
  {
    C1394_SENDERR_2("%s Formats Query ERROR: %s", CamName , GetErrorMessage(m_LastError));
    return false ;
  }

  m_ulSerialNumber = SelectedCam.m_SerialNumber ;
  m_CameraName = m_CamerasInfo[m_CurrentCamera].name ;
  m_GadgetInfo.Format("Unibrain gadget: %s ",m_CamerasInfo[m_CurrentCamera].name );
  _tcscpy_s( m_GadgetInfoString , (LPCTSTR)m_GadgetInfo ) ;

  m_Brightness.m_pOwnerInfo = m_GadgetInfoString ;
  m_Exposure.m_pOwnerInfo = m_GadgetInfoString ;
  m_Sharpness.m_pOwnerInfo = m_GadgetInfoString ;
  m_WBB.m_pOwnerInfo = m_GadgetInfoString ;
  m_WBR.m_pOwnerInfo = m_GadgetInfoString ;
  m_HUE.m_pOwnerInfo = m_GadgetInfoString ;
  m_Saturation.m_pOwnerInfo = m_GadgetInfoString ;
  m_Gamma.m_pOwnerInfo = m_GadgetInfoString ;
  m_Shutter.m_pOwnerInfo = m_GadgetInfoString ;
  m_Gain.m_pOwnerInfo = m_GadgetInfoString ;

  C1394_SENDINFO_0(m_Status);
  return BuildPropertyList();;    
}

void Unibrain::CameraClose()
{
  if ( m_hSelectedCamera )
  {
    if ( IsRunning() )
      CameraStop() ;
    m_LastError = FiCloseCameraHandle( m_hSelectedCamera ) ;
    if ( m_LastError!=FIREi_STATUS_SUCCESS )
      C1394_SENDERR_2("FiCloseCameraHandle %s ERROR: %s", m_CameraName , GetErrorMessage(m_LastError));
    m_hSelectedCamera = 0 ;
  }
  m_CameraName = "No Unibrain Camera";
  return ;

}

bool Unibrain::FormBMPInfoHeader()
{
  m_BMIH.biSize=sizeof(BITMAPINFOHEADER);
  m_BMIH.biWidth = GetXSize( m_StartUpInfo );
  m_BMIH.biHeight = GetYSize( m_StartUpInfo );
  unsigned XOff = GetXOffset( m_StartUpInfo );
  unsigned YOff = GetYOffset( m_StartUpInfo );
  m_CurrentROI=CRect(XOff,YOff,XOff+m_BMIH.biWidth,YOff+m_BMIH.biHeight);
  m_BMIH.biPlanes=1;
  FIREi_PIXEL_FORMAT PixForm = GetPixFormat( m_StartUpInfo ) ;
  switch( PixForm )
  {
  case RAW_8:
  case Y_MONO    : 
    m_BMIH.biCompression=BI_Y8;
    m_BMIH.biBitCount=8;
    m_BMIH.biSizeImage=m_BMIH.biWidth*m_BMIH.biHeight;
    break;
  case YUV_411:
    m_BMIH.biCompression=BI_YUV411;
    m_BMIH.biBitCount=12;
    m_BMIH.biSizeImage=3*m_BMIH.biWidth*m_BMIH.biHeight/2;
    break;
  case RAW_16:
  case Y_MONO_16   : 
    m_BMIH.biCompression=BI_Y16;
    m_BMIH.biBitCount=16;
    m_BMIH.biSizeImage=2*m_BMIH.biWidth*m_BMIH.biHeight;
    break;
  case RGB_24  : 
    m_BMIH.biCompression=BI_RGB;
    m_BMIH.biBitCount=24;
    m_BMIH.biSizeImage=3*m_BMIH.biWidth*m_BMIH.biHeight;
    break;
  case YUV_422:
  case YUV_444:
  default: 
    m_BMIH.biSize=0;
    TRACE("!!! Unsupported format #%d\n", m_StartUpInfo.VideoFormat );
    C1394_SENDERR_1("!!! Unsupported format #%d",m_StartUpInfo.VideoFormat );
    return false;
  }
  return true; 
}

bool Unibrain::BuildPropertyList()
{
  if ( !m_hSelectedCamera || m_CurrentCamera >= (UINT) m_AllCameras.GetCount() )
    return false ;
  m_Properties.RemoveAll();
  int propCount=sizeof(cProperties)/sizeof(CamProperties);
  CAMERA1394::Property P;
  LPCTSTR CamName = m_CamerasInfo[ m_CurrentCamera ].name ;
  if( m_StartUpInfo.Tag != FIREi_CAMERA_STARTUP_INFO_TAG )
  {
    m_Lock.Lock() ;
    memset( &m_StartUpInfo , 0 , sizeof(FIREi_CAMERA_STARTUP_INFO) ) ;
    memset( &m_StartUpInfoForMode7 , 0 , sizeof( m_StartUpInfoForMode7) ) ;
    m_StartUpInfo.pFiFormat7StartupInfo = &m_StartUpInfoForMode7 ;
    m_StartUpInfo.Tag = FIREi_CAMERA_STARTUP_INFO_TAG ;
    m_StartUpInfoForMode7.Tag = FIREi_CAMERA_FORMAT_7_STARTUP_INFO_TAG ;
    m_LastError = FiGetCameraStartupInfo( m_hSelectedCamera , &m_StartUpInfo ) ;
    m_Lock.Unlock() ;
    if ( m_LastError!=FIREi_STATUS_SUCCESS )
    {
      C1394_SENDERR_1("FiGetCameraStartupInfo ERROR: %s", GetErrorMessage(m_LastError));
      FiCloseCameraHandle( m_hSelectedCamera ) ;
      m_hSelectedCamera = 0 ;
      return false;
    }
  }
  if ( !FormPixelFormat_Resolution_FPS() )
    return false ;
  FIREi_PIXEL_FORMAT PixForm = GetPixFormat( m_StartUpInfo ) ;
  bool bColor = (PixForm == YUV_411) || (PixForm == YUV_422)
    || (PixForm == YUV_444) || (PixForm == RGB_24) ;

  for (int i=0; i<propCount; i++)
  {
    switch (cProperties[i].pr)
    {
    case FGP_PIXELFORMAT:
      {
        FXString items,tmpS;
        for (int j=0; j < m_SupportedPixelFormats.GetCount() ; j++)
        {
          tmpS.Format("%s(%d),", FiPixelFormatString( m_SupportedPixelFormats[j] ) , j );
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
    case FGP_RESOLUTION:
      {
        CString items,tmpS;
        if ( FormSupportedResolutions( PixForm ) )
        {
          for (int j = 0 ; j < m_SupportedResolutions.GetCount() ; j++)
          {
            FIREi_RES ThisRes = m_SupportedResolutions[j] ;
            if ( m_SupportedResolutions[j] < res_variable )
            {
              for ( int iResCnt = 0 ; iResCnt < sizeof( cResolutions )/sizeof(cResolutions[0]) ; iResCnt++ )
              {
                if ( cResolutions[iResCnt].res == ThisRes )
                {
                  if (items.IsEmpty())
                    tmpS.Format("%s(%d)", cResolutions[iResCnt].name , j );
                  else
                    tmpS.Format(",%s(%d)", cResolutions[iResCnt].name , j );
                  items+=tmpS;
                  break ;
                }
              }
            }
            else
            {
              for ( int i = 0 ; i < m_SupportedCombinations.GetCount() ; i++ )
              {
                FVPR_Combination& ThisCombin = m_SupportedCombinations.GetAt(i) ;
                if ( ThisCombin.m_PixelFormat == PixForm 
                  && ThisCombin.m_Resolution == ThisRes )
                {
                  if (items.IsEmpty())
                    tmpS.Format("var_Max%dx%d(%d)", ThisCombin.m_MaxSizeForFormat7.cx , 
                    ThisCombin.m_MaxSizeForFormat7.cy , j );
                  else
                    tmpS.Format(",var_Max%dx%d(%d)", ThisCombin.m_MaxSizeForFormat7.cx , 
                    ThisCombin.m_MaxSizeForFormat7.cy , j );
                  items+=tmpS;
                }
              }
            }
          }
          if (items.GetLength())
          {
            P.name=cProperties[i].name;
            P.id=(unsigned)cProperties[i].pr;
            P.property.Format("ComboBox(%s(%s))",P.name,items);
            m_Properties.Add(P);
          }
        }
        break;
      }
    case FGP_FRAMERATE:
      {
        if ( m_StartUpInfo.VideoFormat == Format_7 )
        {
        }
        else if ( FormSupportedFPS( PixForm , GetResolution( m_StartUpInfo ) ) )
        {
          CString items,tmpS;
          for (int j=0; j < m_SupportedFPS.GetCount() ; j++)
          {
            tmpS.Format("%s(%d),", FiFpsString( m_SupportedFPS[j]) , j) ;
            items+=tmpS;
          }
          if (items.GetLength())
          {
            P.name=cProperties[i].name;
            P.id=(unsigned)cProperties[i].pr;
            P.property.Format("ComboBox(%s(%s))",P.name,items);
            m_Properties.Add(P);
          }
        }
        break;
      }
    case FGP_ROI:
      if ( m_StartUpInfo.VideoFormat == Format_7 )
      {
        P.name=cProperties[i].name;
        P.id=(unsigned)cProperties[i].pr;
        P.property.Format("EditBox(%s)",P.name);
        m_Properties.Add(P);
      }
      break;
    case FGP_TRIGGERONOFF:
      {
        FIREi_CAMERA_TRIGGER_FEATURE_INQUIRY_REGISTER Inq ;
        Inq.Tag = FIREi_CAMERA_TRIGGER_FEATURE_INQUIRY_REGISTER_TAG ;
        m_LastError = FiQueryCameraRegister( m_hSelectedCamera ,
          OID_TRIGGER_INQ , &Inq , sizeof(Inq) ) ;
        //         if ( m_LastError!=FIREi_STATUS_SUCCESS )
        //           C1394_SENDERR_2("%s TriggerInquire ERROR: %s", CamName , GetErrorMessage(m_LastError));
        /*else*/ if ( m_LastError == FIREi_STATUS_SUCCESS  &&   Inq.bIsPresent )
        {
          P.name=cProperties[i].name;
          P.id=(unsigned)cProperties[i].pr;
          P.property.Format("ComboBox(%s(Off(0),Tr1(1),Tr1Inv(2),Tr2(3),Tr2Inv(4)))",
            P.name);
          m_Properties.Add(P);
        }
        break;
      }
    case FGP_BRIGHTNESS:
      {
        UnibrainParam& Prop = m_Brightness ;
        if ( Prop.IsPresent( m_hSelectedCamera , m_LastError ) )
        {
          P.name=cProperties[i].name;
          P.id=(unsigned)cProperties[i].pr;
          P.property.Format("%s(%s,%d,%d)", Prop.m_Inq.bHasAuto ? "Spin&Bool" : "Spin" , 
            P.name , Prop.m_Inq.ushMinValue ,
            Prop.m_Inq.ushMaxValue );
          m_Properties.Add(P);
        }
        break;
      }
    case FGP_SHUTTER:
      {
        UnibrainParam& Prop = m_Shutter ;
        if ( Prop.IsPresent( m_hSelectedCamera , m_LastError ) )
        {
          P.name=cProperties[i].name;
          P.id=(unsigned)cProperties[i].pr;
          P.property.Format("%s(%s,%d,%d)", Prop.m_Inq.bHasAuto ? "Spin&Bool" : "Spin" , 
            P.name , Prop.m_Inq.ushMinValue ,
            Prop.m_Inq.ushMaxValue );
          m_Properties.Add(P);
        }
        break;
      }
    case FGP_GAIN:
      {
        UnibrainParam& Prop = m_Gain ;
        if ( Prop.IsPresent( m_hSelectedCamera , m_LastError ) )
        {
          P.name=cProperties[i].name;
          P.id=(unsigned)cProperties[i].pr;
          P.property.Format("%s(%s,%d,%d)", Prop.m_Inq.bHasAuto ? "Spin&Bool" : "Spin" , 
            P.name , Prop.m_Inq.ushMinValue ,
            Prop.m_Inq.ushMaxValue );
          m_Properties.Add(P);
        }
        break;
      }
    case FGP_GAMMA:
      {
        UnibrainParam& Prop = m_Gamma ;
        if ( Prop.IsPresent( m_hSelectedCamera , m_LastError ) )
        {
          P.name=cProperties[i].name;
          P.id=(unsigned)cProperties[i].pr;
          P.property.Format("%s(%s,%d,%d)", Prop.m_Inq.bHasAuto ? "Spin&Bool" : "Spin" , 
            P.name , Prop.m_Inq.ushMinValue ,
            Prop.m_Inq.ushMaxValue );
          m_Properties.Add(P);
        }
        break;
      }
    case FGP_AUTOEXPOSURE:
      {
        UnibrainParam& Prop = m_Exposure ;
        if ( Prop.IsPresent( m_hSelectedCamera , m_LastError ) )
        {
          P.name=cProperties[i].name;
          P.id=(unsigned)cProperties[i].pr;
          P.property.Format("%s(%s,%d,%d)", Prop.m_Inq.bHasAuto ? "Spin&Bool" : "Spin" , 
            P.name , Prop.m_Inq.ushMinValue ,
            Prop.m_Inq.ushMaxValue );
          m_Properties.Add(P);
        }
        break;
      }
    case FGP_SHARPNESS:
      {
        UnibrainParam& Prop = m_Sharpness ;
        if ( Prop.IsPresent( m_hSelectedCamera , m_LastError ) )
        {
          P.name=cProperties[i].name;
          P.id=(unsigned)cProperties[i].pr;
          P.property.Format("%s(%s,%d,%d)", Prop.m_Inq.bHasAuto ? "Spin&Bool" : "Spin" , 
            P.name , Prop.m_Inq.ushMinValue ,
            Prop.m_Inq.ushMaxValue );
          m_Properties.Add(P);
        }
        break;
      }
    case FGP_HUE:
        if ( bColor )
        {
          UnibrainParam& Prop = m_HUE ;
          if ( Prop.IsPresent( m_hSelectedCamera , m_LastError ) )
          {
            P.name=cProperties[i].name;
            P.id=(unsigned)cProperties[i].pr;
            P.property.Format("%s(%s,%d,%d)", Prop.m_Inq.bHasAuto ? "Spin&Bool" : "Spin" , 
              P.name , Prop.m_Inq.ushMinValue ,
              Prop.m_Inq.ushMaxValue );
            m_Properties.Add(P);
          }
        }
        break;
    case FGP_WHITEBALCB:
      {
        if ( bColor )
        {
          UnibrainParam& Prop = m_WBB ;
          if ( Prop.IsPresent( m_hSelectedCamera , m_LastError ) )
          {
            P.name=cProperties[i].name;
            P.id=(unsigned)cProperties[i].pr;
            P.property.Format("%s(%s,%d,%d)", Prop.m_Inq.bHasAuto ? "Spin&Bool" : "Spin" , 
              P.name , Prop.m_Inq.ushMinValue ,
              Prop.m_Inq.ushMaxValue );
            m_Properties.Add(P);
          }
        }
        break;
      }
    case FGP_WHITEBALCR:
      {
        if ( bColor )
        {
          UnibrainParam& Prop = m_WBR ;
          if ( Prop.IsPresent( m_hSelectedCamera , m_LastError ) )
          {
            P.name=cProperties[i].name;
            P.id=(unsigned)cProperties[i].pr;
            P.property.Format("%s(%s,%d,%d)", Prop.m_Inq.bHasAuto ? "Spin&Bool" : "Spin" , 
              P.name , Prop.m_Inq.ushMinValue ,
              Prop.m_Inq.ushMaxValue );
            m_Properties.Add(P);
          }
        }
        break;
      }
    case FGP_SATURATION:
      {
        if ( bColor )
        {
          UnibrainParam& Prop = m_Saturation ;
          if ( Prop.IsPresent( m_hSelectedCamera , m_LastError ) )
          {
            P.name=cProperties[i].name;
            P.id=(unsigned)cProperties[i].pr;
            P.property.Format("%s(%s,%d,%d)", Prop.m_Inq.bHasAuto ? "Spin&Bool" : "Spin" , 
              P.name , Prop.m_Inq.ushMinValue ,
              Prop.m_Inq.ushMaxValue );
            m_Properties.Add(P);
          }
        }
        break;
      }

      // 		case FGP_EXTSHUTTER:
      // 			{
      // 				P.name=cProperties[i].name;
      // 				P.id=(unsigned)cProperties[i].pr;
      // 				P.property.Format("Spin(%s,%d,%d)", P.name , 10 , 30000000 );
      // 				m_Properties.Add(P);
      // 				break;
      // 			}
      // 		case FGP_SEND_FINFO:
      // 			{
      // 				P.name=cProperties[i].name;
      // 				P.id=(unsigned)cProperties[i].pr;
      // 				P.property.Format("ComboBox(%s(Off(0),On(1)))", P.name );
      // 				m_Properties.Add(P);
      // 				break;
      // 			}
      // 		case FGP_TRIGGERDELAY:
      // 			{
      // 				P.name=cProperties[i].name;
      // 				P.id=(unsigned)cProperties[i].pr;
      // 				P.property.Format("Spin(%s,%d,%d)", P.name , 0 , 30000000 );
      // 				m_Properties.Add(P);
      // 				break;
      // 			}
    case FGP_GRAB:
      {
        UINT Capabilities = 0 ;
        UINT uiAddress = 0x400 ;
        m_LastError = FiQueryCameraRegisterEx( m_hSelectedCamera , 
          uiAddress , &Capabilities ) ;
        if ( ShowError( "Check Grab Capabilities" ) )
        {
          if ( Capabilities & 0x00001800) // check for single shot and multishot capabilities
          {
            P.name=cProperties[i].name;
            P.id=(unsigned)cProperties[i].pr;
            P.property.Format("Spin(%s,%d,%d)", P.name , -1 , 30000000 );
            m_Properties.Add(P);
          }
        }
      }
      break ; // not shown property
    default:
      {
        C1394_SENDERR_1("Error: unsupported property %s", cProperties[i].name);
        break;
      }
      }
    }

    return FormBMPInfoHeader() ;
  }

  bool Unibrain::GetCameraProperty( unsigned uPropNum , FXSIZE &value, bool& bauto)
  {
    if ( m_CurrentCamera >= (UINT)m_AllCameras.GetSize() )
      return false ;
    CameraIdentity& SelectedCam = m_AllCameras.GetAt( m_CurrentCamera ) ;
    bauto=false;
    switch ( uPropNum )
    {
    case FGP_PIXELFORMAT:
      {
        FIREi_PIXEL_FORMAT PixForm = GetPixFormat( m_StartUpInfo ) ;
        for ( int i = 0 ; i < m_SupportedPixelFormats.GetCount() ; i++ )
        {
          if ( PixForm == m_SupportedPixelFormats[i] )
          {
            value = i ;
            return true ;
          }
        }
        C1394_SENDERR_1( "GetProperty: %s not found" , FiPixelFormatString( PixForm ) ) ;
        return false;
      }
    case FGP_FRAMERATE:
      {
        FIREi_FPS FrameRate = m_StartUpInfo.FrameRate ;
        bauto = false ;
        for ( int i = 0 ; i < m_SupportedFPS.GetCount() ; i++ )
        {
          if ( FrameRate == m_SupportedFPS[i] )
          {
            value = i ;
            return true ;
          }
        }
        C1394_SENDERR_1( "GetProperty: %s not found" , FiFpsString( FrameRate ) ) ;
        return false;
      }
    case FGP_RESOLUTION:
      {
        FIREi_RES Res = GetResolution( m_StartUpInfo ) ;
        for ( int i = 0 ; i < m_SupportedResolutions.GetCount() ; i++ )
        {
          if ( Res == m_SupportedResolutions[i] )
          {
            value = i ;
            return true ;
          }
        }
        C1394_SENDERR_1( "GetProperty: %s not found" , GetResolutionAsString( Res ) ) ;
        return false ;
      }
    case FGP_ROI:
      {
        if ( m_StartUpInfo.VideoFormat == Format_7 )
        {
          m_ROI.Format("%d,%d,%d,%d" , m_CurrentROI.left , m_CurrentROI.top ,
            m_CurrentROI.Width() , m_CurrentROI.Height() ) ;
          value = (FXSIZE)((LPCTSTR)m_ROI) ;
          return true ;
        }
        return false;
      }
      // 	case FGP_TRIGGERONOFF:
      // 		{
      //  			value = GetTriggerMode() ;
      // 			return true;
      // 		}
    case FGP_EXTSHUTTER:
      {
        value = GetLongExposure() ;
        return true ;
      }
      // 	case FGP_SEND_FINFO:
      // 		{
      //       value = m_bFrameInfoOn;
      // 			//value = (IsFrameInfoAvailable() != 0) ;
      // 			return true ;
      // 		}
      // 	case FGP_TRIGGERDELAY:
      // 		{
      // 			value = GetTriggerDelay() ;
      // 			return true ;
      // 		}
    case FGP_GRAB:
      {
        bool bContinuous ;
        bool bRes = GetGrabConditions( bContinuous , value ) ;
        return true;
      }
    case FGP_SHUTTER:
      {
        BOOL bAuto ;
        value = GetShutter( bAuto ) ;
        bauto = (bAuto != 0) ;
        return true;
      }
    case FGP_GAIN:
      {
        BOOL bAuto ;
        value = GetGain( bAuto ) ;
        bauto = (bAuto != 0) ;
        return true ;
      }
    case FGP_BRIGHTNESS:
      {
        BOOL bAuto ;
        value = GetBrightness( bAuto ) ;
        bauto = bAuto != 0 ;
        return true;
      }
    case FGP_AUTOEXPOSURE:
      {
        BOOL bAuto = false ;
        value = GetExposure( bAuto ) ;
        bauto = (bAuto != 0) ;
        return true;
      }
    case FGP_GAMMA:
      {
        value = GetGamma() ;
        return true;
      }
    case FGP_SHARPNESS:
      {
        value = GetSharpness() ;
        return true;
      }
    case FGP_HUE:
      {
        value = GetHue() ;
        return true;
      }
    case FGP_WHITEBALCB:
      {
        BOOL bAuto ;
        value = GetWBB( bAuto ) ;
        bauto = (bAuto != 0) ;
        return true;
      }
    case FGP_WHITEBALCR:
      {
        BOOL bAuto ;
        value = GetWBR( bAuto ) ;
        bauto = (bAuto != 0) ;
        return true;
      }
    case FGP_SATURATION:
      {
        value = GetSaturation() ;
        return true;
      }
    default:
      break;
    }
    return false;
  }

  bool Unibrain::SetCameraProperty(unsigned uPropNum , int &value, bool& bauto, bool& Invalidate)
  {
    if ( m_CurrentCamera >= MAX_CAMERASNMB ||  !m_hSelectedCamera )
      return false ;
    bool res = false ;
    switch ( uPropNum )
    {
    case FGP_PIXELFORMAT:
      {
        if ( value >= 0  &&  value < m_SupportedPixelFormats.GetCount() )
        {
          FIREi_PIXEL_FORMAT CurrentFormat = GetPixFormat( m_StartUpInfo ) ;
          if ( CurrentFormat != m_SupportedPixelFormats[value] )
          {
            bool wasRunning=IsRunning();
            if (wasRunning)
              CameraStop();
            int iIndex = FindSupportedModeAndFormat( m_SupportedPixelFormats[ value ] ) ;
            m_StartUpInfo.VideoFormat = m_SupportedCombinations[iIndex].m_VideoFormat ;
            m_StartUpInfo.VideoMode = m_SupportedCombinations[iIndex].m_VideoMode ;
            m_StartUpInfo.FrameRate = m_SupportedCombinations[iIndex].m_FPS ;
            Invalidate = true ;
            if ( wasRunning )
              CameraStart() ;
            BuildPropertyList() ;
          }
          res = true ;
        }
        return res;
      }
    case FGP_FRAMERATE:
      {
        if ( value >= 0  &&  value < m_SupportedFPS.GetCount() )
        {
          if ( m_StartUpInfo.FrameRate != m_SupportedFPS[ value ] )
          {
            bool wasRunning=IsRunning();
            if (wasRunning)
              CameraStop();
            m_StartUpInfo.FrameRate = m_SupportedFPS[ value ] ;
            Invalidate=true;
            if (wasRunning)
            {
              Sleep(50) ;
              CameraStart();
            }
            BuildPropertyList();
          }
          res = true ;
        }
        return res;
      }
    case FGP_RESOLUTION:
      {
        if ( value >= 0  &&  value < m_SupportedResolutions.GetCount() )
        {
          bool wasRunning=IsRunning();
          if (wasRunning)
            CameraStop();
          FIREi_RES NecessaryRes = m_SupportedResolutions[ value ] ;
          FIREi_PIXEL_FORMAT PixForm = GetPixFormat( m_StartUpInfo ) ;
          if ( NecessaryRes < res_variable )
          {
            int iIndex = FindSupportedModeAndFormat( PixForm , m_StartUpInfo.FrameRate , NecessaryRes ) ;
            if ( iIndex >= 0 )
            {
              m_StartUpInfo.VideoFormat = m_SupportedCombinations[ iIndex ].m_VideoFormat ;
              m_StartUpInfo.VideoMode = m_SupportedCombinations[ iIndex ].m_VideoMode ;
              m_StartUpInfo.FrameRate = m_SupportedCombinations[ iIndex ].m_FPS ;
              m_StartUpInfo.pFiFormat7StartupInfo = NULL ;
              if (wasRunning)
                CameraStart();
            }
          }
          else
          {
            int iVariableIndex = NecessaryRes / 100 ;
            int iIndex = FindSupportedModeAndFormat( PixForm , fps_none , NecessaryRes ) ;
            m_StartUpInfo.VideoFormat = m_SupportedCombinations[ iIndex ].m_VideoFormat ;
            m_StartUpInfo.VideoMode = m_SupportedCombinations[ iIndex ].m_VideoMode ;
            //             m_StartUpInfo.TransmitSpeed = S400 ;
            m_StartUpInfo.IsochSyCode = 1 ;
            //          m_StartUpInfo.ChannelNumber = 0 ;  // replace to adapter number in future
            m_StartUpInfo.pFiFormat7StartupInfo = &m_StartUpInfoForMode7 ;
            m_StartUpInfoForMode7.Tag = FIREi_CAMERA_FORMAT_7_STARTUP_INFO_TAG ;
            m_StartUpInfoForMode7.ushImageHeight = (USHORT) m_SupportedCombinations[ iIndex ].m_MaxSizeForFormat7.cy ;
            m_StartUpInfoForMode7.ushImageWidth = (USHORT)m_SupportedCombinations[ iIndex ].m_MaxSizeForFormat7.cx ;
            m_StartUpInfoForMode7.ushLeftImagePosition = 0 ;
            m_StartUpInfoForMode7.ushTopImagePosition = 0 ;
            m_StartUpInfoForMode7.PixelFormat = PixForm ;
            m_Format7Registers.Tag                   = FIREi_CAMERA_FORMAT_7_REGISTERS_TAG;
            m_Format7Registers.Mode                  = m_StartUpInfo.VideoMode;
            m_Format7Registers.TransmitSpeed         = S400 ;
            m_Format7Registers.pFiFormat7StartupInfo = &m_StartUpInfoForMode7 ;
            //           m_LastError = FiQueryCameraRegister(m_hSelectedCamera, 
            //             OID_FORMAT_7_REGISTERS, &m_Format7Registers, sizeof(m_Format7Registers));
            //           ShowError( "QueryFormat7Registers" , true ) ;
            if (wasRunning)
              CameraStart();
          }

          Invalidate=true;
          BuildPropertyList();
          res = true ;
        }
        return res;
      }
    case FGP_ROI:
      {
        if ( !m_StartUpInfo.pFiFormat7StartupInfo ) // not format 7
          return false;
        CRect rc;
        FXString ROIAsString( (LPCTSTR)value ) ;
        if (sscanf((LPCTSTR)ROIAsString,"%d,%d,%d,%d",&rc.left,&rc.top,&rc.right,&rc.bottom)==4)
        {
          // right and bottom are width and height. Calculate real right and bottom.
          rc.right += rc.left ;
          rc.bottom += rc.top ;
          bool res = SetROI(rc) ;
          return res;
        }
        return false;
      }
    case FGP_TRIGGERONOFF:
      {
        SetTriggerMode( value );
        return true;
      }
    case FGP_EXTSHUTTER:
      {
        SetLongExposure( value ) ;
        return true ;
      }
    case FGP_SEND_FINFO:
      {
        //         if (!IsFrameInfoAvailable())
        //         {
        //             FrameInfoOn=false;
        //             return false;
        //         }
        //         SetSendFrameInfo( value ) ;
        //         FrameInfoOn=(value!=0);
        return true ;
      }
    case FGP_TRIGGERDELAY:
      {
        SetTriggerDelay( value );
        return true;
      }
    case FGP_GRAB:
      {
        SetGrab( value ) ;
        return true;
      }
    case FGP_SHUTTER:
      {
        SetShutter( value , bauto ) ;
        return true;
      }
    case FGP_GAIN:
      {
        SetGain( value , bauto ) ;
        return true ;
      }
    case FGP_BRIGHTNESS:
      {
        SetBrightness( value , bauto ) ;
        return true;
      }
    case FGP_AUTOEXPOSURE:
      {
        /*Invalidate = */SetExposure( value , bauto ) ;
        return true;
      }
    case FGP_GAMMA:
      {
        SetGamma( value ) ;
        return true;
      }
    case FGP_SHARPNESS:
      {
        SetSharpness( value ) ;
        return true;
      }
    case FGP_HUE:
      {
        SetHue( value ) ;
        return true;
      }
    case FGP_WHITEBALCB:
      {
        SetWBB( value ,  bauto ) ;
        return true;
      }
    case FGP_WHITEBALCR:
      {
        SetWBR( value , bauto ) ;
        return true;
      }
    case FGP_SATURATION:
      {
        SetSaturation( value ) ;
        return true;
      }
    default:
      break;
    }
    return false;
  }

  bool Unibrain::CameraStart()
  {
    if ( m_CurrentCamera >= (UINT)m_AllCameras.GetCount() )
    {
      m_hSelectedCamera = NULL ;
      return false ;
    }
    if ( !m_hSelectedCamera )
    {
      m_LastError = FiOpenCameraHandleEx( m_StartUpInfo.m_iAdapterNumber ,
        &m_hSelectedCamera , &m_AllCameras[ (int) m_CurrentCamera].m_GUID ) ;
      if ( !ShowError( "CameraStart FiOpenCameraHandleEx") )
        return false ;
    }

    //   m_LastError = FiStartCamera( m_hSelectedCamera , 
    //     &m_StartUpInfo );
    //   if ( m_LastError != FIREi_STATUS_SUCCESS )
    //   {
    //     C1394_SENDERR_1("FiStartCamera Error: %s", GetErrorMessage( m_LastError ) );
    //     return false ;
    //   }
    //   return true ;
    FIREi_PIXEL_FORMAT PixForm = GetPixFormat(  m_StartUpInfo ) ;
    switch( PixForm )
    {
    case RAW_8:
    case Y_MONO:    m_BMIH.biCompression = BI_Y8 ; break ;
    case Y_MONO_16_SIGNED:
    case RAW_16:
    case Y_MONO_16: m_BMIH.biCompression = BI_Y16 ; break ;
    case YUV_411:   m_BMIH.biCompression = BI_YUV9 ; break ;
    case YUV_422:   m_BMIH.biCompression = BI_YUV12 ; break ;
    case RGB_24:    m_BMIH.biCompression = BI_YUV12 ; break ;
    }

    return initIsochEngine() ;
  }

  void Unibrain::CameraStop()
  {
    //   m_LastError = FiStopCamera( m_hSelectedCamera );
    //   if ( m_LastError != FIREi_STATUS_SUCCESS )
    //   {
    //     C1394_SENDERR_1("FiStopCamera Error: %s", GetErrorMessage( m_LastError ) );
    //   }

    CleanUp() ;
    Sleep(50) ;
  }

  CVideoFrame* Unibrain::CameraDoGrab(double* StartTime)
  {
    m_Lock.Lock() ;
    if ( m_hSelectedCamera && m_hIsochEngine ) 
    {
      FIREi_ISOCH_ENGINE_HANDLE hSaved = m_hIsochEngine ;
      FIREi_CAMERA_FRAME CameraFrame ;
      m_LastError = FiGetNextCompleteFrame( &CameraFrame , 
        m_hIsochEngine , 100 );
      if ( StartTime )
        *StartTime = GetHRTickCount() ;
      m_Lock.Unlock() ;
      if ( m_LastError != FIREi_STATUS_SUCCESS )
      {
        if ( FiIsCameraConnected( m_hSelectedCamera) )
        {
          CleanUp() ;
          C1394_SENDERR_1( "Camera %s is disconnected" , (LPCTSTR)m_CameraName ) ;
          if ( !CameraStart() )
            C1394_SENDERR_1( "Camera %s can't be restarted" , (LPCTSTR)m_CameraName ) ;
          else
            C1394_SENDERR_1( "Camera %s Restarted" , (LPCTSTR)m_CameraName ) ;

          return NULL ;
        }
        TRACE( "\nFiGetNextCompleteFrame ERROR %s" , GetErrorMessage( m_LastError) ) ;
        return NULL ;
      }

      bool rescaled=false;
      m_RealBMIH.biWidth = CameraFrame.uFrameWidth ;
      m_RealBMIH.biHeight = CameraFrame.uFrameHeight ;
      int iSize = m_RealBMIH.biWidth * m_RealBMIH.biHeight ;
      bool b16Bits = false ;
      int iSrcRowInsterval = CameraFrame.uFrameWidth ;
      int iDestRowInterval = m_RealBMIH.biWidth ;
      pTVFrame pOut = NULL ; 
      switch( m_BMIH.biCompression )
      {
      case BI_RGB:
        break ;
      case BI_Y8:
      case BI_Y800:
        switch ( CameraFrame.PixelFormat )
        {
        case Y_MONO:
          {
            m_RealBMIH.biSize = sizeof( BITMAPINFOHEADER ) ;
            m_RealBMIH.biCompression = BI_YUV9 ;
            m_RealBMIH.biClrImportant = 0 ;
            m_RealBMIH.biBitCount = 9 ;
            m_RealBMIH.biClrUsed = 0 ;
            m_RealBMIH.biPlanes = 1 ;
            m_RealBMIH.biSizeImage = (iSize * 9) / 8 ;
            m_RealBMIH.biXPelsPerMeter = 0 ;
            m_RealBMIH.biYPelsPerMeter = 0 ;
            pOut = (pTVFrame) malloc( sizeof( TVFrame ) ) ;
            pOut->lpBMIH = (LPBITMAPINFOHEADER)malloc( sizeof(BITMAPINFOHEADER) + m_RealBMIH.biSizeImage ) ;
            memcpy( pOut->lpBMIH , &m_RealBMIH , sizeof(BITMAPINFOHEADER) ) ;
            pOut->lpData = NULL ;
            LPBYTE src = (LPBYTE)CameraFrame.pCameraFrameBuffer;
            LPBYTE dst = GetData( pOut ) ;
            memcpy( dst , src , iSize ) ;
            memset( dst + iSize , 128 , m_RealBMIH.biSizeImage - iSize ) ;
          }
          break ;
        case Y_MONO_16:
        case Y_MONO_16_SIGNED:
          {
            m_RealBMIH.biSize = sizeof( BITMAPINFOHEADER ) ;
            m_RealBMIH.biCompression = BI_YUV9 ;
            m_RealBMIH.biClrImportant = 0 ;
            m_RealBMIH.biBitCount = 9 ;
            m_RealBMIH.biClrUsed = 0 ;
            m_RealBMIH.biPlanes = 1 ;
            m_RealBMIH.biSizeImage = (iSize * 9) / 8 ;
            m_RealBMIH.biXPelsPerMeter = 0 ;
            m_RealBMIH.biYPelsPerMeter = 0 ;
            pOut = (pTVFrame) malloc( sizeof( TVFrame ) ) ;
            pOut->lpBMIH = (LPBITMAPINFOHEADER)malloc( sizeof(BITMAPINFOHEADER) + m_RealBMIH.biSizeImage ) ;
            memcpy( pOut->lpBMIH , &m_RealBMIH , sizeof(BITMAPINFOHEADER) ) ;
            pOut->lpData = NULL ;
            LPWORD src = (LPWORD)CameraFrame.pCameraFrameBuffer;
            LPBYTE dst = GetData( pOut ) ;
            LPBYTE pEnd = dst + iSize ;
            while ( dst < pEnd ) *(dst++) = (*(src++) >> 8) ;
            memset( dst , 128 , m_RealBMIH.biSizeImage - iSize ) ;
          }
          break ;
        case YUV_411:
          {
            m_RealBMIH.biSize = sizeof( BITMAPINFOHEADER ) ;
            m_RealBMIH.biCompression = BI_YUV411 ;
            m_RealBMIH.biBitCount=12;
            m_RealBMIH.biSizeImage = 3 * m_RealBMIH.biWidth * m_RealBMIH.biHeight / 2 ;
            m_RealBMIH.biClrImportant = 0 ;
            m_RealBMIH.biClrUsed = 0 ;
            m_RealBMIH.biPlanes = 1 ;
            m_RealBMIH.biXPelsPerMeter = 0 ;
            m_RealBMIH.biYPelsPerMeter = 0 ;
            pOut = (pTVFrame) malloc( sizeof( TVFrame ) ) ;
            pOut->lpBMIH = yuv411yuv9( &m_RealBMIH , (LPBYTE)CameraFrame.pCameraFrameBuffer ) ;
            pOut->lpData = NULL ;
            memset( (LPBYTE)(&(pOut->lpBMIH[1])) + iSize , 128 , pOut->lpBMIH->biSizeImage - iSize ) ; // clear color 
          }
          break ;
        case YUV_422:
          break ;
        case YUV_444:
          break ;
        }
        break ;
      case BI_Y16:
        break ;
      case BI_YUV9:
        {
          switch ( CameraFrame.PixelFormat )
          {
          case YUV_411:
            {
              m_RealBMIH.biSize = sizeof( BITMAPINFOHEADER ) ;
              m_RealBMIH.biCompression = BI_YUV411 ;
              m_RealBMIH.biBitCount=12;
              m_RealBMIH.biSizeImage = 3 * m_RealBMIH.biWidth * m_RealBMIH.biHeight / 2 ;
              m_RealBMIH.biClrImportant = 0 ;
              m_RealBMIH.biClrUsed = 0 ;
              m_RealBMIH.biPlanes = 1 ;
              m_RealBMIH.biXPelsPerMeter = 0 ;
              m_RealBMIH.biYPelsPerMeter = 0 ;
              pOut = (pTVFrame) malloc( sizeof( TVFrame ) ) ;
              pOut->lpBMIH = yuv411yuv9( &m_RealBMIH , (LPBYTE)CameraFrame.pCameraFrameBuffer ) ;
              pOut->lpData = NULL ;
            }
          }
        }
        break ;
      case BI_YUV12:
        switch ( CameraFrame.PixelFormat )
        {
        case YUV_411:
          {
            m_RealBMIH.biSize = sizeof( BITMAPINFOHEADER ) ;
            m_RealBMIH.biCompression = BI_YUV411 ;
            m_RealBMIH.biBitCount=12;
            m_RealBMIH.biSizeImage = 3 * m_RealBMIH.biWidth * m_RealBMIH.biHeight / 2 ;
            m_RealBMIH.biClrImportant = 0 ;
            m_RealBMIH.biClrUsed = 0 ;
            m_RealBMIH.biPlanes = 1 ;
            m_RealBMIH.biXPelsPerMeter = 0 ;
            m_RealBMIH.biYPelsPerMeter = 0 ;
            pOut = (pTVFrame) malloc( sizeof( TVFrame ) ) ;
            pOut->lpBMIH = yuv411yuv9( &m_RealBMIH , (LPBYTE)CameraFrame.pCameraFrameBuffer ) ;
            pOut->lpData = NULL ;
          }
        }
        break ;
      }
      if ( pOut )
      {
        CVideoFrame * pOutFrame = CVideoFrame::Create( pOut ) ;
        return pOutFrame ;
      }
      return NULL ;
    }
    m_Lock.Unlock() ;
    return NULL;
  }

  bool Unibrain::IsTriggerByInputPin()
  {
    UINT32 TriggerValue = 0 ;
    // 	// Get current trigger mode
    // 	m_Camera.GetParameter(FGP_TRIGGER,&TriggerValue);
    return (TriggerValue!=0);
  }
  int Unibrain::GetTriggerMode()
  {
    // 	UINT32 TriggerValue , IO1 , IO2 ;
    // 	m_Camera.ReadRegister( 0xf0f00830 , &TriggerValue ) ;
    // 	// Get current trigger mode
    // 	//m_Camera.GetParameter(FGP_TRIGGER,&TriggerValue);
    // 	int iOn = TRGON(TriggerValue) ;
    // 	if ( !iOn )
    return 0 ;
    // 	m_Camera.ReadRegister( 0xf1000300 , &IO1 ) ;
    // 	m_Camera.ReadRegister( 0xf1000304 , &IO2 ) ;
    // 	int iPol = TRGPOL(TriggerValue) ;
    // 	int iSrc = ( IO2 & 0x00020000 ) ? 1 : 0 ;
    // 	if ( iSrc )
    // 		return (iPol) ? 4 : 3 ;
    // 	else
    // 		return (iPol) ? 2 : 1 ;
  }

  void Unibrain::SetTriggerMode(int iMode)
  {

    // iMode : 0 - no trigger, 1 - channel 1 no inverse , 2 - channel 1 inverse
    //                         3 - channel 2 no inverse , 4 - channel 2 inverse
    UINT32 nOn=(iMode)?1:0; // 0=ext. trigger off, else ext. trigger on
    if ( !nOn )
    {
      // 		UINT32 TriggerValue=MAKETRIGGER(0, 0, 0, 0, 0);
      // 		m_Camera.SetParameter(FGP_TRIGGER,TriggerValue);
      return ;
    }
    // 	UINT32 TrigReg , IO1 , IO2 ;
    // 	m_Camera.ReadRegister( 0xf0f00830 , &TrigReg ) ;
    // 	m_Camera.ReadRegister( 0xf1000300 , &IO1 ) ;
    // 	m_Camera.ReadRegister( 0xf1000304 , &IO2 ) ;
    // 	UINT32 nSrc=(iMode > 2) ? 1 : 0; // not currently applicable to AVT cameras, as of February 2008 - parameter inherited from IIDC/DCAM
    // 	UINT32 uiAddrOn = 0xf1000300 + (nSrc * 4) ;
    // 	UINT32 uiAddrOff = 0xf1000300 + ((nSrc ^ 1) * 4) ;
    // 	UINT32 iPol = (iMode % 2) == 0 ;
    // 	// Check and set polarity together with trigger mode
    // 	UINT32 uiControlIO = 0x00020000 ;
    // 
    // 	UINT32 TriggerValue=MAKETRIGGER( 1 , iPol , nSrc , 0 , 0 ) ;
    // 	m_Camera.SetParameter(FGP_TRIGGER,TriggerValue);
    // 	m_Camera.WriteRegister( uiAddrOn , uiControlIO ) ;
    // 	m_Camera.WriteRegister( uiAddrOff , 0 ) ;
    // 	m_Camera.WriteRegister( 0xf0f00830 , 0x02000000 | (iPol ? 0x01000000 : 0 ) ) ;
    //UINT32 nMode=0; // 0=edge mode, 1=level mode, 15=bulk mode
    //UINT32 nParm=0; // not currently applicable to AVT cameras, as of February 2008 - parameter inherited from IIDC/DCAM
    //UINT32 TriggerValue;
    // Enable ext. trigger, edge mode (0), falling edge
  }

  void Unibrain::GetROI(CRect& rc)
  {
    rc = m_CurrentROI;
  }

  bool Unibrain::SetROI(CRect& rc)
  {
    if ( !m_StartUpInfo.pFiFormat7StartupInfo )
    {
      C1394_SENDERR_0("Can't Set ROI. Reason: The camera format isn't scalable");
      return false;
    }
    if (rc.left!=-1) //must be divisible by 4
    {
      UINT uiMask = ~( m_Format7Registers.ushUnitHPosition - 1 ) ;
      rc.left   &= uiMask;
      rc.right  &= uiMask;
      rc.bottom &= uiMask;
      rc.top    &= uiMask;
    }
    else
    {
      rc.left = rc.top=0;
      rc.right = m_Format7Registers.ushMaxImageHSize ;
      rc.bottom = m_Format7Registers.ushMaxImageHSize ;
    }
    m_StartUpInfoForMode7.ushLeftImagePosition = (UINT16) rc.left ;
    m_StartUpInfoForMode7.ushTopImagePosition = (UINT16)rc.top ;
    m_StartUpInfoForMode7.ushImageHeight = (UINT16)rc.Height() ;
    m_StartUpInfoForMode7.ushImageWidth = (UINT16)rc.Width() ;
    if ( ( rc.Width()  == m_CurrentROI.Width() ) 
      && ( rc.Height() == m_CurrentROI.Height() ) )
    {     // if format 7 and size the same simply do origin shift
      // by writing into IMAGE_ORIGIN register
      // without grab stop
      m_LastError = FiSetCameraRegister( m_hSelectedCamera , OID_FORMAT7_IMAGE_POS ,
        &m_StartUpInfo , sizeof(m_StartUpInfo)) ;
      ShowError( "SetROI Shift Position" ) ;
      if ( m_LastError == FIREi_STATUS_SUCCESS )
        m_CurrentROI = rc ;
      return true;
    }
    bool wasRunning=IsRunning();
    if (wasRunning)
      CameraStop();
    m_LastError = FiSetCameraRegister( m_hSelectedCamera , OID_FORMAT7_IMAGE_POS ,
      &m_StartUpInfo , sizeof(m_StartUpInfo)) ;
    ShowError( "SetROI Change Position" ) ;
    if ( m_LastError == FIREi_STATUS_SUCCESS )
    {
      m_CurrentROI = rc ;
      if (wasRunning)
        CameraStart();
      return true;
    }
    if (wasRunning)
      CameraStart();
    return false;        
  }
  int Unibrain::GetLongExposure()
  {
    UINT32 iExp_usec = 1 ;
    // 	m_Camera.ReadRegister(0xF100020c,&iExp_usec);
    return (iExp_usec & 0x03ffffff);
  }

  void Unibrain::SetLongExposure( int iExp)
  {
    // 	m_Camera.WriteRegister(0xF100020c,iExp_usec);
  }
  int Unibrain::GetExposure( BOOL& bAuto )
  {
    if ( m_CurrentCamera >= (UINT)m_AllCameras.GetSize() )
    {
      C1394_SENDERR_1("Get Auto Exposure Error: Camera %d does not exists", m_CurrentCamera  );
      return 0 ;
    }
    int iValue ;
    bool bbAuto ;
    m_LastError = m_Exposure.GetValue( m_hSelectedCamera , iValue , bbAuto ) ;
    if ( m_LastError != FIREi_STATUS_SUCCESS )
      C1394_SENDERR_1("Get Auto Exposure Error: %s", GetErrorMessage( m_LastError ) );
    bAuto = bbAuto ;
    return iValue ;
  }

  bool Unibrain::SetExposure( int iExp , BOOL bAuto )
  {
    if ( m_CurrentCamera >= (UINT)m_AllCameras.GetSize() )
    {
      C1394_SENDERR_1("Set Auto Exposure Error: Camera %d does not exists", m_CurrentCamera  );
      return 0 ;
    }
    m_LastError = m_Exposure.SetValue( m_hSelectedCamera , iExp , bAuto != FALSE ) ;
    if ( m_LastError != FIREi_STATUS_SUCCESS )
      C1394_SENDERR_1("Set Auto Exposure Error: %s", GetErrorMessage( m_LastError ) );
    return ( m_Exposure.m_bAutoChanged  &&  !m_Exposure.m_Reg.bSetAuto ) ;
  }
  int Unibrain::GetBrightness( BOOL& bAuto )
  {
    if ( m_CurrentCamera >= (UINT)m_AllCameras.GetSize() )
    {
      C1394_SENDERR_1("Get Brightness Error: Camera %d does not exists", m_CurrentCamera  );
      return 0 ;
    }
    int iValue = m_Brightness.m_Reg.ushCurValue ;
    bool bbAuto = m_Brightness.m_Reg.bSetAuto == TRUE ;
    if ( m_hSelectedCamera )
    {
      m_LastError = m_Brightness.GetValue( m_hSelectedCamera , iValue , bbAuto ) ;
//       if ( m_LastError != FIREi_STATUS_SUCCESS )
//         C1394_SENDERR_1("Get Brightness Error: %s", GetErrorMessage( m_LastError ) );
    bAuto = bbAuto ;
    }
    return iValue ;
  }

  void Unibrain::SetBrightness( int iBrightness , BOOL bAuto )
  {
    if ( m_CurrentCamera >= (UINT)m_AllCameras.GetSize() )
      C1394_SENDERR_1("Set Brightness Error: Camera %d does not exists", m_CurrentCamera  );
    else if ( m_hSelectedCamera )
    {
    m_LastError = m_Brightness.SetValue( m_hSelectedCamera , iBrightness , bAuto != FALSE ) ;
//       if ( m_LastError != FIREi_STATUS_SUCCESS )
//         C1394_SENDERR_1("Set Brightness Error: %s", GetErrorMessage( m_LastError ) );
  }
  }

  int Unibrain::GetGain( BOOL& bAuto )
  {
    if ( m_CurrentCamera >= (UINT)m_AllCameras.GetSize() )
    {
      C1394_SENDERR_1("Get Gain Error: Camera %d does nopt exists", m_CurrentCamera  );
      return 0 ;
    }
    int iValue ;
    bool bbAuto ;
    m_LastError = m_Gain.GetValue( m_hSelectedCamera , iValue , bbAuto ) ;
//     if ( m_LastError != FIREi_STATUS_SUCCESS )
//       C1394_SENDERR_1("Get Gain Error: %s", GetErrorMessage( m_LastError ) );
    bAuto = bbAuto ;
    return (int) iValue ;
  }

  void Unibrain::SetGain( int iGain , BOOL bAuto )
  {
    if ( m_CurrentCamera >= (UINT)m_AllCameras.GetSize() )
      C1394_SENDERR_1("Set Gain Error: Camera %d does nopt exists", m_CurrentCamera  );
    else
    {
    bool bbAuto = bAuto != FALSE ;
    m_LastError = m_Gain.SetValue( m_hSelectedCamera , iGain , bbAuto ) ;
//       if ( m_LastError != FIREi_STATUS_SUCCESS )
//         C1394_SENDERR_1("Set Gain Error: %s", GetErrorMessage( m_LastError ) );
  }
  }

  int Unibrain::GetShutter( BOOL& bAuto )
  {
    if ( m_CurrentCamera >= (UINT)m_AllCameras.GetSize() )
    {
      C1394_SENDERR_1("Get Shutter Error: Camera %d does nopt exists", m_CurrentCamera  );
      return 0 ;
    }
    int iValue ;
    bool bbAuto ;
    m_LastError = m_Shutter.GetValue( m_hSelectedCamera , iValue , bbAuto ) ;
//     if ( m_LastError != FIREi_STATUS_SUCCESS )
//       C1394_SENDERR_1("Get Shutter Error: %s", GetErrorMessage( m_LastError ) );
    bAuto = bbAuto ;
    return (int) iValue ;
  }

  void Unibrain::SetShutter( int iShutter , BOOL bAuto )
  {
    if ( m_CurrentCamera >= (UINT)m_AllCameras.GetSize() )
      C1394_SENDERR_1("Set Shutter Error: Camera %d does nopt exists", m_CurrentCamera  );
    else
    {
    bool bbAuto = bAuto != FALSE ;
    m_LastError = m_Shutter.SetValue( m_hSelectedCamera , iShutter , bbAuto ) ;
//       if ( m_LastError != FIREi_STATUS_SUCCESS )
//         C1394_SENDERR_1("Set Shutter Error: %s", GetErrorMessage( m_LastError ) );
  }
 }

  int Unibrain::GetGamma()
  {
    if ( m_CurrentCamera >= (UINT)m_AllCameras.GetSize() )
    {
      C1394_SENDERR_1("Get Gamma Error: Camera %d does nopt exists", m_CurrentCamera  );
      return 0 ;
    }
    int iValue ;
    m_LastError = m_Gamma.GetValue( m_hSelectedCamera , iValue ) ;

//     if ( m_LastError != FIREi_STATUS_SUCCESS )
//       C1394_SENDERR_1("Get Gamma Error: %s", GetErrorMessage( m_LastError ) );
    return iValue ;
  }

  void Unibrain::SetGamma( int iGamma )
  {
    if ( m_CurrentCamera >= (UINT)m_AllCameras.GetSize() )
    {
      C1394_SENDERR_1("Set Gamma Error: Camera %d does nopt exists", m_CurrentCamera  );
    }
    else
    {
    m_LastError = m_Gamma.SetValue( m_hSelectedCamera , iGamma ) ;
//       if ( m_LastError != FIREi_STATUS_SUCCESS )
//         C1394_SENDERR_1("Set Gamma Error: %s", GetErrorMessage( m_LastError ) );
  }
  }
  int Unibrain::GetSharpness()
  {
    if ( m_CurrentCamera >= (UINT)m_AllCameras.GetSize() )
      return 0 ;
    int iValue ;
    m_Sharpness.GetValue( m_hSelectedCamera , iValue ) ;

    return (int) iValue ;
  }

  void Unibrain::SetSharpness( int iSharpness )
  {
    m_LastError = m_Sharpness.SetValue( m_hSelectedCamera , iSharpness ) ;
//     if ( m_LastError != FIREi_STATUS_SUCCESS )
//       C1394_SENDERR_1("Set Sharpness Error: %s", GetErrorMessage( m_LastError ) );
  }

  int Unibrain::GetHue()
  {
    if ( m_CurrentCamera >= (UINT)m_AllCameras.GetSize() )
      return 0 ;

    int iValue ;
    m_HUE.GetValue( m_hSelectedCamera , iValue ) ;
//     CameraIdentity& SelectedCam = m_AllCameras.GetAt( m_CurrentCamera ) ;
//     FIREi_CAMERA_FEATURE_CONTROL_REGISTER Reg ;
//     Reg.Tag = FIREi_CAMERA_FEATURE_CONTROL_REGISTER_TAG ;
//     m_LastError = FiQueryCameraRegister( m_hSelectedCamera ,
//       OID_HUE_CONTROL , &Reg , sizeof( Reg ) ) ;
//     if ( m_LastError != FIREi_STATUS_SUCCESS )
//       C1394_SENDERR_1("Get iHue Error: %s", GetErrorMessage( m_LastError ) );
    return iValue ;
  }

  void Unibrain::SetHue( int iHue )
  {
    m_LastError = m_HUE.SetValue( m_hSelectedCamera , iHue ) ;
//     if ( m_LastError != FIREi_STATUS_SUCCESS )
//       C1394_SENDERR_1("Set iHue Error: %s", GetErrorMessage( m_LastError ) );
  }


  int Unibrain::GetWBR( BOOL& bAuto )
  {
    if ( m_CurrentCamera >= (UINT)m_AllCameras.GetSize() )
      return 0 ;
    int iValue ;
    bool bbAuto ;
    m_WBR.GetValue( m_hSelectedCamera , iValue , bbAuto ) ;
    //     CameraIdentity& SelectedCam = m_AllCameras.GetAt( m_CurrentCamera ) ;
//     FIREi_CAMERA_FEATURE_CONTROL_REGISTER Reg ;
//     Reg.Tag = FIREi_CAMERA_FEATURE_CONTROL_REGISTER_TAG ;
//     m_LastError = FiQueryCameraRegister( m_hSelectedCamera ,
//       OID_VR_CONTROL , &Reg , sizeof( Reg ) ) ;
//     if ( m_LastError != FIREi_STATUS_SUCCESS )
//       C1394_SENDERR_1("Get WhiteBalanceR Error: %s", GetErrorMessage( m_LastError ) );
//     else
    bAuto = bbAuto ;
    return iValue ;
  }

  void Unibrain::SetWBR( int iWhiteBalanceR , BOOL bAuto )
  {
    bool bbAuto = bAuto != FALSE ;
    m_LastError = m_WBR.SetValue( m_hSelectedCamera , iWhiteBalanceR , bbAuto ) ;
//     if ( m_LastError != FIREi_STATUS_SUCCESS )
//       C1394_SENDERR_1("Set WhiteBalanceR Error: %s", GetErrorMessage( m_LastError ) );
  }

  int Unibrain::GetWBB( BOOL& bAuto )
  {
    if ( m_CurrentCamera >= (UINT)m_AllCameras.GetSize() )
      return 0 ;
    int iValue ;
    bool bbAuto ;
    m_WBR.GetValue( m_hSelectedCamera , iValue , bbAuto ) ;
//     CameraIdentity& SelectedCam = m_AllCameras.GetAt( m_CurrentCamera ) ;
//     FIREi_CAMERA_FEATURE_CONTROL_REGISTER Reg ;
//     Reg.Tag = FIREi_CAMERA_FEATURE_CONTROL_REGISTER_TAG ;
//     m_LastError = FiQueryCameraRegister( m_hSelectedCamera ,
//       OID_UB_CONTROL , &Reg , sizeof( Reg ) ) ;
//     if ( m_LastError != FIREi_STATUS_SUCCESS )
//       C1394_SENDERR_1("Get WhiteBalanceB Error: %s", GetErrorMessage( m_LastError ) );
//     else
    bAuto = bbAuto ;
    return iValue ;
  }

  void Unibrain::SetWBB( int iWhiteBalanceB , BOOL bAuto )
  {
    bool bbAuto = bAuto != FALSE ;
    m_LastError = m_WBB.SetValue( m_hSelectedCamera , iWhiteBalanceB , bbAuto ) ;
//     if ( m_LastError != FIREi_STATUS_SUCCESS )
//       C1394_SENDERR_1("Set WhiteBalanceB Error: %s", GetErrorMessage( m_LastError ) );
  }

  int Unibrain::GetSaturation()
  {
    if ( m_CurrentCamera >= (UINT)m_AllCameras.GetSize() )
      return 0 ;
    int iValue ;
    m_Saturation.GetValue( m_hSelectedCamera , iValue ) ;
//     CameraIdentity& SelectedCam = m_AllCameras.GetAt( m_CurrentCamera ) ;
//     FIREi_CAMERA_FEATURE_CONTROL_REGISTER Reg ;
//     Reg.Tag = FIREi_CAMERA_FEATURE_CONTROL_REGISTER_TAG ;
//     m_LastError = FiQueryCameraRegister( m_hSelectedCamera ,
//       OID_SATURATION_CONTROL , &Reg , sizeof( Reg ) ) ;
//     if ( m_LastError != FIREi_STATUS_SUCCESS )
//       C1394_SENDERR_1("Get Saturation Error: %s", GetErrorMessage( m_LastError ) );
    return iValue ;
  }

  void Unibrain::SetSaturation( int iSaturation )
  {
    m_LastError = m_Saturation.SetValue( m_hSelectedCamera , iSaturation ) ;
//     if ( m_LastError != FIREi_STATUS_SUCCESS )
//       C1394_SENDERR_1("Set Saturation Error: %s", GetErrorMessage( m_LastError ) );
  }

  DWORD Unibrain::IsFrameInfoAvailable()
  {
    // 	DWORD iAvail ;
    // 	m_Camera.ReadRegister(0xF1000630,&iAvail);
    // 	if ( iAvail & 0x80000000 )
    // 		return (iAvail & 0x0200ffff ) ;
    // 	else
    return 0 ;
  }

  void Unibrain::SetSendFrameInfo( int iSend)
  {
    // 	m_Camera.WriteRegister(0xF1000630 , (iSend) ? 0x02000000 : 0 );
  }

  int Unibrain::GetTriggerDelay()
  {
    UINT32 iDelay_usec = 0 ;
    // 	m_Camera.ReadRegister(0xF1000400,&iDelay_usec);
    return iDelay_usec ;
  }

  void Unibrain::SetTriggerDelay( int iDelay_uS)
  {
    // 	if ( iDelay_uS )
    // 		iDelay_uS = 0x02000000 | (iDelay_uS & 0x001fffff) ;
    // 	m_Camera.WriteRegister(0xF1000400,iDelay_uS);
  }

  bool Unibrain::SetGrab( int iNFrames )
  {
    if (  !m_hSelectedCamera 
      ||  (iNFrames < -1)  
      ||  (iNFrames > 0xffff) )
      return false ;
    if ( iNFrames == -1 )
    {
      iNFrames = 0x80000000 ;
      m_LastError = FiSetCameraRegisterEx( m_hSelectedCamera , 0xF0f00614 , &iNFrames ) ;
      return ShowError( "SetGrab" ) ;
    }
    else 
    {
      UINT Nul = 0 ;
      m_LastError = FiSetCameraRegisterEx( m_hSelectedCamera , 0xF0f00614 , &Nul ) ;
      if ( iNFrames == 1 )
        iNFrames = 0x80000000 ;
      else if ( iNFrames > 0 )
        iNFrames = 0x40000000 + (iNFrames & 0x0000ffff) ;
      else
        iNFrames = Nul ;
      m_LastError = FiSetCameraRegisterEx( m_hSelectedCamera , 0xF0f0061c , &iNFrames ) ;
      return ShowError( "SetGrab" ) ;
    }
    return true ;
  }

  bool Unibrain::GetGrabConditions( 
    bool& bContinuous , int& iNRestFrames ) 
  {
    // 	UINT32 uiVal = 0 ;
    // 	m_Camera.ReadRegister( 0xF0f00614 , &uiVal );
    // 	if ( uiVal & 0x80000000 )
    // 	{
    // 		bContinuous = true ;
    // 		iNRestFrames = 0 ;
    // 		return true ;
    // 	}
    // 	else
    // 	{
    // 		bContinuous = false ;
    // 		m_Camera.ReadRegister( 0xF0f0061c , &uiVal );
    // 		if ( uiVal & 0x80000000 )
    // 			iNRestFrames = 1 ;
    // 		else if ( uiVal & 0x40000000 )
    // 			iNRestFrames = uiVal & 0xffff ;
    // 		else
    // 			iNRestFrames = 0 ;
    // 	}
    return true ;
  }

  // Create and start an isoch receive engine for the first camera found
  bool Unibrain::initIsochEngine() 
  {
    if ( !m_hSelectedCamera )
      return false ;
    //   m_LastError = FiCreateIsochReceiveEngineEx(
    //     &SelectedCam.m_hIsochEngineEx , 1 , &SelectedCam.m_hChannelStartEvent );
    //   _HANDLE_ERROR1(m_LastError, "FireiDriver::InitIsochEngine()");
    // 
    //   m_LastError = FiAllocateFrames( SelectedCam.m_hIsochEngineEx, 
    //     &m_StartUpInfo, 10);
    //   _HANDLE_ERROR1(m_LastError, "FireiDriver::InitIsochEngine()");
    // 
    //   m_LastError = FiStartIsochReceiveEngineEx(SelectedCam.m_hIsochEngineEx);
    //   _HANDLE_ERROR1(m_LastError, "FireiDriver::InitIsochEngine()");
    //   m_LastError = FiGetCameraStartupInfo( m_hSelectedCamera , &m_StartUpInfo ) ;
    //   if ( m_LastError!=FIREi_STATUS_SUCCESS )
    //   {
    //     C1394_SENDERR_1("FiGetCameraStartupInfo ERROR: %s", GetErrorMessage(m_LastError));
    //     FiCloseCameraHandle( m_hSelectedCamera ) ;
    //     return false;
    //   }
    FXAutolock al( m_Lock ) ;
    BOOLEAN bRunning = false ;
    m_LastError = FiIsCameraRunning( m_hSelectedCamera , &bRunning );
    ShowError( "FiIsCameraRunning" ) ;
    if ( bRunning )
    {
      //     int iIsoChannel
      m_LastError = FiStopCamera( m_hSelectedCamera ) ;
      ShowError( "FiStopCamera" , true ) ;
    }
    if ( m_hIsochEngine )
    {
      m_LastError = FiStopIsochReceiveEngine( m_hIsochEngine ) ;
      if ( ShowError( "FiStopIsochReceiveEngine") )
      {
        m_LastError = FiCleanupIsochReceiveEngine( m_hIsochEngine ) ;
        ShowError( "FiCleanupIsochReceiveEngine" ) ;
        m_LastError = FiDeleteIsochReceiveEngine( m_hIsochEngine );
        ShowError( "FiDeleteIsochReceiveEngine") ;
        m_hIsochEngine = NULL ;
      }
    }
    FiCloseCameraHandle( m_hSelectedCamera ) ;
    m_LastError = FiOpenCameraHandleEx( m_StartUpInfo.m_iAdapterNumber ,
      &m_hSelectedCamera , &m_AllCameras[ (int) m_CurrentCamera].m_GUID ) ;
    if ( !ShowError("FiOpenCameraHandleEx" ) )
      return false ;

    m_LastError = FiCreateIsochReceiveEngine( &m_hIsochEngine );
    if ( ShowError("FiCreateIsochReceiveEngine" ) )
    {
      if ( m_StartUpInfo.VideoFormat == Format_7 )
      {
        memset( &m_Format7Registers , 0 , sizeof( m_Format7Registers ) ) ;
        m_Format7Registers.Tag                   = FIREi_CAMERA_FORMAT_7_REGISTERS_TAG;
        m_Format7Registers.Mode                  = m_StartUpInfo.VideoMode ;
        //       m_Format7Registers.TransmitSpeed         = S400 ;
        //       m_Format7Registers.pFiFormat7StartupInfo = NULL ;
        m_LastError = FiQueryCameraRegister(m_hSelectedCamera, 
          OID_FORMAT_7_REGISTERS, &m_Format7Registers, sizeof(m_Format7Registers));
        if ( ShowError( "QueryFormat7Registers1" , true ) )
        {
          m_StartUpInfo.pFiFormat7StartupInfo = &m_StartUpInfoForMode7 ;
          m_StartUpInfoForMode7.Tag = FIREi_CAMERA_FORMAT_7_STARTUP_INFO_TAG ;
          //        m_StartUpInfoForMode7.ushPacketSize = m_Format7Registers.ushMaxPacketBytes ;
          m_StartUpInfoForMode7.PixelFormat = Y_MONO ;
          m_StartUpInfoForMode7.BytesPerFrame = 0 ;
          m_StartUpInfoForMode7.PixelsPerFrame = 0 ;
          m_StartUpInfoForMode7.ushImageWidth = 1024 ;
          m_StartUpInfoForMode7.ushImageHeight = 768 ;
          m_StartUpInfoForMode7.ushLeftImagePosition = 0 ;
          m_StartUpInfoForMode7.ushTopImagePosition = 0 ;
          m_StartUpInfoForMode7.ushPacketSize = 0 ;
          m_StartUpInfoForMode7.RawMode = 0 ;
          m_StartUpInfoForMode7.RawConversion = 0 ;
          m_StartUpInfoForMode7.ushPacketSize = m_Format7Registers.ushMaxPacketBytes ;

          m_Format7Registers.pFiFormat7StartupInfo = &m_StartUpInfoForMode7 ;
          m_LastError = FiQueryCameraRegister(m_hSelectedCamera, 
            OID_FORMAT_7_REGISTERS, &m_Format7Registers, sizeof(m_Format7Registers));
          m_StartUpInfoForMode7.ushPacketSize = m_Format7Registers.ushMaxPacketBytes ;

          ShowError( "QueryFormat7Registers2" , true ) ;
        };
      }
      BYTE ucChannel = 0 ;
      m_LastError = FiGetAvailableIsoChannel( m_hSelectedCamera , &ucChannel ) ;
      if ( ShowError( "FiGetAvailableIsoChannel" , true ) )
      {
        m_StartUpInfo.ChannelNumber = ucChannel ;
        m_LastError = FiStartIsochReceiveEngine( m_hIsochEngine ,
          &m_StartUpInfo , m_StartUpInfo.m_iAdapterNumber );
        if ( ShowError( "FiStartIsochReceiveEngine") )
        {
          m_LastError = FiStartCamera( m_hSelectedCamera , &m_StartUpInfo ) ;
          ShowError("Last StartCamera After Engine" ) ;
          TRACE("\ninitIsochEngine CameraStarted");
          m_AllCameras[ (int) m_CurrentCamera ].m_hCamera = m_hSelectedCamera ;
          TRACE("\nCamera %s started on adapter %d channel %d" , 
            m_AllCameras[ (int) m_CurrentCamera ].m_Name ,
            m_StartUpInfo.m_iAdapterNumber , 
            m_StartUpInfo.ChannelNumber );
          return true;
        }
      };
      m_LastError = FiDeleteIsochReceiveEngine( m_hIsochEngine );
      ShowError( "FiDeleteIsochReceiveEngine") ;
      m_hIsochEngine = NULL ;
      m_LastError = FiStopCamera( m_hSelectedCamera ) ;
      ShowError( "FiStopCamera") ;
      FiCloseCameraHandle( m_hSelectedCamera ) ;
      m_AllCameras[ (int) m_CurrentCamera ].m_hCamera = NULL ;
      m_hSelectedCamera = NULL ;
    }  
    TRACE("\ninitIsochEngine Camera NOT Started");
    return false ;  
  }

  void Unibrain::CleanUp() 
  {
    FXAutolock al( m_Lock ) ;
    if ( m_hIsochEngine )
    {
      m_StartUpInfo.Tag = FIREi_CAMERA_STARTUP_INFO_TAG ;
      m_LastError = FiGetCameraStartupInfo( m_hSelectedCamera , &m_StartUpInfo ) ;
      if ( m_LastError!=FIREi_STATUS_SUCCESS )
      {
        TRACE("CleanUp FiGetCameraStartupInfo ERROR: %s", GetErrorMessage(m_LastError));
        //         FiCloseCameraHandle( m_hSelectedCamera ) ;
        //         m_hSelectedCamera = 0 ;
        //         return false;
      }
      m_LastError = FiStopIsochReceiveEngine( m_hIsochEngine );
      if ( ShowError( "FiStopIsochReceiveEngine") )
      {
        m_LastError = FiCleanupIsochReceiveEngine( m_hIsochEngine ) ;
        ShowError( "FiCleanupIsochReceiveEngine" ) ;
      }

      m_LastError = FiDeleteIsochReceiveEngine( m_hIsochEngine );
      m_hIsochEngine = NULL ;
    }
    m_LastError = FiStopCamera( m_hSelectedCamera ) ;
    ShowError( "FiStopCamera") ;
    m_LastError = FiCloseCameraHandle( m_hSelectedCamera ) ;
    for ( int i = 0 ; i < m_AllCameras.GetCount() ; i++ )
    {
      if ( m_AllCameras[i].m_hCamera == m_hSelectedCamera )
      {
        m_AllCameras[i].m_hCamera = NULL ;
        break ;
      }
    }
    m_hSelectedCamera = NULL ;
    TRACE("Unibrain::cleanup: Done!\n");
  }

  int Unibrain::GetFreeIsoChannel() 
  {
    bool bFree[8] = { true , true , true , true , true , true , true , true } ;
    for ( int i = 0 ; i < m_AllCameras.GetCount() ; i++ )
    {
      CameraIdentity& Cam = m_AllCameras.GetAt(i) ;
      if ( Cam.m_SerialNumber != m_ulSerialNumber
        && Cam.m_hCamera != NULL
        && Cam.m_StartUpInfo.m_iAdapterNumber == m_StartUpInfo.m_iAdapterNumber )
      {
        BOOLEAN bRunning = FALSE ;
        m_LastError = FiIsCameraRunning( Cam.m_hCamera , &bRunning ) ;
        ShowError( "GetFreeIsoChannel:FiIsCameraRunning" , true ) ;
        if ( bRunning )
        {
          int iChannel = Cam.m_StartUpInfo.GetChannel() ;
          bFree[iChannel] = false ;
        }
      }
    }
    for ( int iChan = 0 ; iChan < 8 ; iChan++ )
    {
      if ( bFree[iChan] )
        return iChan ;
    }
    return -1;
  }

  // void FireiDriver::freeCameraHandles() 
  // {
  //   FIREi_STATUS        FireiStatus;
  // 
  //   for(int i=0; i<m_totalCameras; i++) 
  //   {
  //     if(m_cameraArray[i].hCamera) 
  //     {
  //       FireiStatus = FiCloseCameraHandle(m_cameraArray[i].hCamera);
  //       _HANDLE_ERROR(FireiStatus, "FireiDriver::freeCameraHandles()");
  //     }                
  //   }
  // }
