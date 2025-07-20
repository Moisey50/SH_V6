// avt2_11Camera.h: interface for the avt2_11 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UNIBRAIN_CAMERA_H__84FCD8FA_3B15_4BDE_99A7_EC03545294C4__INCLUDED_)
#define AFX_UNIBRAIN_CAMERA_H__84FCD8FA_3B15_4BDE_99A7_EC03545294C4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <FIREI.H>
// #include "CFireiXManager.h"
// #include "CFireiXCamera.h"
// #include "CFireiXGUID.h"
// #include "CFireiXFrame.h"
// #include "CFireiXFixedStreamFormat.h"
// #include "CFireiXRegister.h"
// #include "CFireiXStreamFormat.h"
// #include "CFireiXFeature.h"
// #include "CFireiXTrigger.h"

#include <..\..\gadgets\cameras\common\gadgets\1394Camera.h>



enum Parameters
{
//   FGP_VIDEOFORMAT = 1 ,
//   FGP_VIDEOMODE    ,
  FGP_PIXELFORMAT  ,
  FGP_FRAMERATE    ,
  FGP_RESOLUTION   ,
  FGP_BRIGHTNESS   ,
  FGP_AUTOEXPOSURE ,
  FGP_SHARPNESS    ,
  FGP_WHITEBALCB   ,
  FGP_WHITEBALCR   ,
  FGP_HUE          ,
  FGP_SATURATION   ,
  FGP_GAMMA        ,
  FGP_IRIS         ,
  FGP_FOCUS        ,
  FGP_ZOOM         ,
  FGP_PAN          ,
  FGP_TILT         ,
  FGP_SHUTTER      ,
  FGP_GAIN         ,
  FGP_ROI          ,
  FGP_TRIGGERONOFF ,
  FGP_EXTSHUTTER   ,
  FGP_SEND_FINFO   ,
  FGP_TRIGGERDELAY ,
  FGP_GRAB         
};

class FVPR_Combination
{
public:
  FVPR_Combination() { m_PixelFormat = pixel_format_none ; m_Resolution = res_none ;  }
  FVPR_Combination( FIREi_VIDEO_FORMAT VideoFormat , FIREi_VIDEO_MODE   VideoMode ,
                   FIREi_PIXEL_FORMAT PixelFormat , FIREi_RES  Res , FIREi_FPS FPS ,
                   CSize * pSizeForFormat7 = NULL )
  {
    m_VideoFormat = VideoFormat ;
    m_VideoMode = VideoMode ;
    m_PixelFormat = PixelFormat ;
    m_Resolution = Res ;
    m_FPS = FPS ;
    m_MaxSizeForFormat7 = ( pSizeForFormat7 ) ? *pSizeForFormat7 : CSize(0,0) ;
  };

  FVPR_Combination& operator =( const FVPR_Combination& Orig) 
  { memcpy( this , &Orig , sizeof(*this) ) ; return *this ; }
  FIREi_VIDEO_FORMAT m_VideoFormat ;
  FIREi_VIDEO_MODE   m_VideoMode ;
  FIREi_PIXEL_FORMAT m_PixelFormat ;
  FIREi_RES          m_Resolution ;
  FIREi_FPS          m_FPS ;
  CSize              m_MaxSizeForFormat7 ;
} ;

typedef FXArray<FVPR_Combination> FVPRs ;

class Bus1394
{
public:
  ULONG         m_ulBusNumber ;
  FXArray <int> m_IsoChannels ;

  bool IsThisBus( ULONG ulBus ) { return ulBus == m_ulBusNumber ; } ;
  bool GetFreeChannel( int& iChannel )
  {
    for ( int i = 0 ; i < m_IsoChannels.GetCount() ; i++ )
    {
      if ( m_IsoChannels[i] == 0 )
      {
        iChannel = i ;
        m_IsoChannels[i] = 1 ;
        return true ;
      }
    }
    return false ;
  }
  bool ReleaseChannel( int iChannel )
  {
    if ( m_IsoChannels[iChannel] )
    {
      m_IsoChannels[iChannel] = 0 ;
      return true ;
    }
    TRACE( _T("Bus %d Channel %d is not busy for release") , m_ulBusNumber , iChannel ) ;
    return false ;
  }
};


class StartUpData: public FIREi_CAMERA_STARTUP_INFO
{
public:
  StartUpData() 
  { 
    memset(this , 0 , sizeof(*this) ) ;
    m_iAdapterNumber = -1 ; // unknown adapter
  }

  int          m_iAdapterNumber ;
  int GetChannel() { return ChannelNumber ; }
};

class CameraIdentity
{
public:
  CameraIdentity() 
  { 
    memset( this , 0 , (BYTE*)(&m_StartUpInfo) - (BYTE*)(this) ) ;
    m_StartUpInfo.pFiFormat7StartupInfo = &m_StartUpInfoForMode7 ;
  }

  ~CameraIdentity() {}
  CameraIdentity& operator = (const CameraIdentity& Orig)
  {
    m_hCamera = Orig.m_hCamera ;
    m_GUID = Orig.m_GUID ;
    m_SerialNumber = Orig.m_SerialNumber ;
    m_Name = Orig.m_Name ;
    m_StartUpInfo = Orig.m_StartUpInfo ;
    m_hIsochEngine = Orig.m_hIsochEngine ;
    m_hIsochEngineEx = Orig.m_hIsochEngineEx ;
    m_StartUpInfoForMode7 = Orig.m_StartUpInfoForMode7 ;
    m_StartUpInfo.pFiFormat7StartupInfo = &m_StartUpInfoForMode7 ;
    return *this ;
  }

  FIREi_CAMERA_HANDLE      m_hCamera;
  FIREi_CAMERA_GUID        m_GUID ;
  ULONG                    m_SerialNumber ;
//  HANDLE                   m_hChannelStartEvent ;
  FIREi_CAMERA_FORMAT_7_STARTUP_INFO  m_StartUpInfoForMode7 ;
  FIREi_ISOCH_ENGINE_HANDLE     m_hIsochEngine ;
  FIREi_ISOCH_ENGINE_EX_HANDLE  m_hIsochEngineEx ;
  StartUpData              m_StartUpInfo ;
  FXString                 m_Name ;
};

typedef FXArray<CameraIdentity> AllCameras ;

class UnibrainParam
{
public:
  UnibrainParam() { memset( &m_Reg , 0 , (LPBYTE)&m_Name - (LPBYTE)&m_Reg ) ; }
  UnibrainParam( int iEnum , DWORD OID_Control , DWORD OID_Inq , LPCTSTR pName = NULL )
  {
    memset( &m_Reg , 0 , (LPBYTE)&m_Name - (LPBYTE)&m_Reg ) ;
    m_PropertyEnum = iEnum ;
    m_OIDControl = OID_Control ;
    m_OIDInq = OID_Inq ;
    m_Name = pName ;
  }

  UnibrainParam& operator =( UnibrainParam& Orig )
  {
    memcpy( &m_Reg , &Orig.m_Reg , (LPBYTE)&m_Name - (LPBYTE)&m_Reg ) ;
    m_Name = Orig.m_Name ;
    return *this ;
  }
  
  bool IsPresent( FIREi_CAMERA_HANDLE hCam , UINT& Status )
  {
    Status = 0xffffffff ;
    if ( m_bCheckedForPresence )
    {
      Status = FIREi_STATUS_SUCCESS ;
      return m_Inq.bIsPresent != FALSE ;
    }
    if ( ! hCam )
      return false ;
    m_Inq.Tag = FIREi_CAMERA_FEATURE_INQUIRY_REGISTER_TAG ;
    Status = FiQueryCameraRegister( hCam ,
      m_OIDInq , &m_Inq , sizeof(m_Inq) ) ;
    if ( Status == FIREi_STATUS_SUCCESS )
    {
      m_bCheckedForPresence = true ;
      if ( m_Inq.bIsPresent )
      {
        m_Reg.Tag = FIREi_CAMERA_FEATURE_CONTROL_REGISTER_TAG ;
        Status = FiQueryCameraRegister( hCam ,
          m_OIDControl , &m_Reg , sizeof( m_Reg ) ) ;
        return true ;
      }
    }
    return false ;
  }

  UINT GetValue( FIREi_CAMERA_HANDLE hCam , int& Value , bool& bAuto )
  {
    UINT Status = STATUS_1394_INVALID_HANDLE ;
    if ( !m_bCheckedForPresence )
    {
      if ( !IsPresent( hCam , Status ) )
        return Status ;
    }
    m_Reg.Tag = FIREi_CAMERA_FEATURE_CONTROL_REGISTER_TAG ;
    Value = m_Reg.ushCurValue ;
    bAuto = m_Reg.bSetAuto == TRUE ;
    if ( hCam )
    {
    Status = FiQueryCameraRegister( hCam ,
      m_OIDControl , &m_Reg , sizeof( m_Reg ) ) ;
      if ( Status == FIREi_STATUS_SUCCESS )
      {
        bAuto = (m_Reg.bSetAuto != FALSE) ;
        Value = (int) m_Reg.ushCurValue ;
      }
      else
      {
        //C1394_SENDERR_2("Can't get property %s (%s)", m_Name , FiStatusString( Status ) );
        FxSendLogMsg(MSG_ERROR_LEVEL, m_pOwnerInfo , 0 ,  
          _T("Can't get property %s (%s)"), m_Name,FiStatusString( Status ) ) ;
      }
    }

    return Status ;
  }

  UINT GetValue( FIREi_CAMERA_HANDLE hCam , int& Value )
  {
    UINT Status = STATUS_1394_INVALID_HANDLE ;
    if ( !m_bCheckedForPresence )
    {
      if ( !IsPresent( hCam , Status ) )
      return Status ;
    }
    m_Reg.Tag = FIREi_CAMERA_FEATURE_CONTROL_REGISTER_TAG ;
    Value = m_Reg.ushCurValue ;
    if ( hCam )
    {
      Status = FiQueryCameraRegister( hCam ,
        m_OIDControl , &m_Reg , sizeof( m_Reg ) ) ;
      if ( Status == FIREi_STATUS_SUCCESS )
    Value = (int) m_Reg.ushCurValue ;
      else
      {
        //C1394_SENDERR_2("Can't get property %s (%s)", m_Name , FiStatusString( Status ) );
        FxSendLogMsg(MSG_ERROR_LEVEL, m_pOwnerInfo , 0 ,  
          _T("Can't get property %s (%s)"), m_Name,FiStatusString( Status ) ) ;
      }
    }

    return Status ;
  }

  UINT SetValue( FIREi_CAMERA_HANDLE hCam , int iValue , bool bAuto = false )
  {
    UINT Status = STATUS_1394_INVALID_HANDLE ;
    if ( !m_bCheckedForPresence )
    {
      if ( !IsPresent( hCam , Status ) )
        return Status ;
    }
    m_Reg.Tag = FIREi_CAMERA_FEATURE_CONTROL_REGISTER_TAG ;
    BOOL bOldAuto = m_Reg.bSetAuto ;
    if ( m_Inq.bHasAuto )
      m_Reg.bSetAuto = bAuto ;
    if ( iValue < (int)m_Inq.ushMinValue )
      iValue = (int)m_Inq.ushMinValue ;
    if ( iValue > (int)m_Inq.ushMaxValue )
      iValue = (int)m_Inq.ushMaxValue ;
    m_Reg.ushCurValue = (unsigned short) iValue ;
    if ( hCam )
    {
    Status = FiSetCameraRegister( hCam ,
      m_OIDControl , &m_Reg , sizeof( m_Reg ) ) ;
      if ( Status != FIREi_STATUS_SUCCESS)
      {
        //C1394_SENDERR_2("Can't set property %s (%s)", m_Name , FiStatusString( Status ) );
        FxSendLogMsg(MSG_ERROR_LEVEL, m_pOwnerInfo , 0 , 
          _T("Can't set property %s (%s)"), m_Name,FiStatusString( Status ) ) ;
      }

    m_bAutoChanged = (bOldAuto != m_Reg.bSetAuto) ;
    if ( m_bAutoChanged && !m_Reg.bSetAuto)
    {
      Status = FiQueryCameraRegister( hCam ,
        m_OIDControl , &m_Reg , sizeof( m_Reg ) ) ;
        if ( Status != FIREi_STATUS_SUCCESS)
        {
          //C1394_SENDERR_2("Can't get value for property %s (%s)", m_Name , FiStatusString( Status ) );
          FxSendLogMsg(MSG_ERROR_LEVEL, m_pOwnerInfo , 0 , 
            _T("Can't get volatile %s (%s)"), m_Name,FiStatusString( Status ) ) ;
        }
      }
    }
    return Status ;
  }
  bool IsAutoChanged() { return m_bAutoChanged ; }
  FIREi_CAMERA_FEATURE_CONTROL_REGISTER m_Reg ;
  FIREi_CAMERA_FEATURE_INQUIRY_REGISTER m_Inq ;  
  DWORD m_OIDControl ;
  DWORD m_OIDInq ;
  int   m_PropertyEnum ;
  bool  m_bCheckedForPresence ;
  bool  m_bAutoChanged ;
  LPCTSTR m_pOwnerInfo ;
  FXString m_Name ;
};

typedef FXArray<UnibrainParam> ParamArray ;

class Unibrain : public C1394Camera  
{
protected:
    UINT                     m_LastError;
    BITMAPINFOHEADER         m_BMIH;
    BITMAPINFOHEADER         m_RealBMIH;
    CRect                    m_Current;
    FXString                 m_CameraName;
    bool                     m_bFrameInfoOn;
    CRect                    m_CurrentROI ;
    static FXArray<Bus1394>  m_Busses ;
    static AllCameras        m_AllCameras ;
    FXLockObject             m_Lock;

    UnibrainParam            m_Brightness ;
    UnibrainParam            m_Exposure ;
    UnibrainParam            m_Sharpness ;
    UnibrainParam            m_WBB ;
    UnibrainParam            m_WBR ;
    UnibrainParam            m_HUE ;
    UnibrainParam            m_Saturation ;
    UnibrainParam            m_Gamma ;
    UnibrainParam            m_Shutter ;
    UnibrainParam            m_Gain ;


    ULONG                    m_ulSerialNumber ;  // selected serial number
    FIREi_CAMERA_HANDLE      m_hSelectedCamera ;
    FIREi_ISOCH_ENGINE_HANDLE m_hIsochEngine ;
    StartUpData               m_StartUpInfo ;
    FIREi_CAMERA_FORMAT_7_STARTUP_INFO  m_StartUpInfoForMode7 ;
    FIREi_CAMERA_SUPPORTED_FORMAT       m_SupportedFormat ;
    FIREi_CAMERA_FORMAT_7_REGISTERS     m_Format7Registers ;
    FXString                  m_ROI ;

    FXArray<FIREi_VIDEO_FORMAT>   m_SupportedVideoFormats ;
    FXArray<FIREi_VIDEO_MODE>     m_SupportedVideoModes ;
    FXArray<FIREi_PIXEL_FORMAT>   m_SupportedPixelFormats ;
    FXArray<FIREi_RES>            m_SupportedResolutions ;
    FXArray<FIREi_FPS>            m_SupportedFPS ;
    FVPRs                         m_SupportedCombinations ;
    TCHAR                         m_GadgetInfoString[256] ;
    
    static FXLockObject     m_UnibrainInitLock ;
    static long             m_UnibrainInitCntr ;
    static bool             m_UnibrainDrvReady ;
public:
	  Unibrain();
    // HW stuff
    bool UnibrainInit() ;
    void UnibrainDone() ;
    int  GetFreeIsoChannel() ;
    virtual bool DriverInit();
    virtual bool EnumCameras();
    virtual bool CameraInit();
    virtual void CameraClose();

    virtual bool CameraStart();
    virtual void CameraStop();

    bool GetCameraProperty(unsigned i, int &value, bool& bauto);
    bool SetCameraProperty(unsigned i, int &value, bool& bauto, bool& Invalidate);

    virtual CVideoFrame* CameraDoGrab(double* StartTime);
    // Capture gadget stuff
    virtual void ShutDown();

    DECLARE_RUNTIME_GADGET(Unibrain);
private: // helpers
    inline LPCTSTR GetErrorMessage(UINT32 errCode)
    {
      return FiStatusString( errCode ) ;
    }

    bool            GetCameraDescription( CameraIdentity& Cam ) ;
    int             InitCameraArray() ;
    bool            initIsochEngine() ;
    void            CleanUp() ;
    bool            BuildPropertyList();
    bool            CameraConnected() 
    {
      if ( !m_hSelectedCamera )
        return false ;
      return (FALSE != FiIsCameraConnected( m_AllCameras[(int)m_CurrentCamera].m_hCamera )) ; 
    };
    virtual bool IsRunning()
    {
      if ( !m_bRun  || !m_hSelectedCamera || (m_CurrentCamera >= (unsigned)m_AllCameras.GetCount()) )
        return false ;
      BOOLEAN bRunning = false ;
      m_LastError = FiIsCameraRunning( m_hSelectedCamera , &bRunning ) ;
//       if ( !bRunning  ||  !m_hIsochEngine )
//       {
//         initIsochEngine() ;
//         Sleep( 10 ) ;
//         m_LastError = FiIsCameraRunning( m_hSelectedCamera , &bRunning ) ;
//       }
      return bRunning == TRUE ;
    }
    inline bool ShowError( LPCTSTR pPlacing , bool bAsTrace = false )
    {
      if (  m_LastError == FIREi_STATUS_SUCCESS  
        ||  m_LastError == FIREi_STATUS_CAMERA_ALREADY_RUNNING )
        return true ;
      if ( bAsTrace )
        TRACE("%s: %s\n", pPlacing , GetErrorMessage(m_LastError)) ;
      else
        C1394_SENDERR_2("%s: %s\n", pPlacing , GetErrorMessage(m_LastError)) ;
      return false ;
    }
    int FormSupportedResolutions( FIREi_PIXEL_FORMAT PixForm ) ;
    int FormSupportedFPS( FIREi_PIXEL_FORMAT PixForm , FIREi_RES Resol ) ;
    int FindSupportedModeAndFormat( FIREi_PIXEL_FORMAT PixForm ) ;
    int FindSupportedModeAndFormat( 
      FIREi_PIXEL_FORMAT PixForm ,FIREi_FPS fps, FIREi_RES rs) ;
    bool            FormPixelFormat_Resolution_FPS() ;
    bool            FormBMPInfoHeader() ;
    FIREi_PIXEL_FORMAT GetPixFormat( StartUpData& StartupInfo ) ;
    FIREi_RES       GetResolution( StartUpData& StartupInfo ) ;
    bool            IsTriggerByInputPin() ;
    int             GetTriggerMode();
    void            SetTriggerMode(int iMode);
    void            GetROI(CRect& rc);
    bool            SetROI(CRect& rc);
    int             GetLongExposure() ;
    void            SetLongExposure( int iExp ) ;
    int             GetExposure( BOOL& bAuto ) ;
    bool            SetExposure( int iAverageValue , BOOL bAuto ) ;
    int             GetBrightness( BOOL& bAuto ) ;
    void            SetBrightness( int iBrightness , BOOL bAuto ) ;
    int             GetGain( BOOL& bAuto ) ;
    void            SetGain( int iGain , BOOL bAuto ) ;
    int             GetShutter( BOOL& bAuto ) ;
    void            SetShutter( int iShutter , BOOL bAuto ) ;
    int             GetGamma() ;
    void            SetGamma( int iGamma ) ;
    int             GetSharpness() ;
    void            SetSharpness( int iGamma ) ;
    int             GetHue() ;
    void            SetHue( int iHue ) ;
    int             GetWBR( BOOL& bAuto ) ;
    void            SetWBR( int iWBR ,  BOOL bAuto ) ;
    int             GetWBB(  BOOL& bAuto  ) ;
    void            SetWBB( int iWBB , BOOL bAuto ) ;
    int             GetSaturation() ;
    void            SetSaturation( int iSaturation ) ;
    void            SetSendFrameInfo( int iSend ) ;
    DWORD           IsFrameInfoAvailable() ; // also availability check
    int             GetTriggerDelay() ;
    void            SetTriggerDelay( int iDelay_uS ) ;
    bool            SetGrab( int iNFrames ) ;
    bool            GetGrabConditions( bool& bContinuous , int& iNRestFrames ) ;
    bool            CamRegGet( UnibrainParam& Param , int& iVal , bool& bAuto ) ;
    bool            CamRegSet( UnibrainParam& Param , int iVal , bool bAuto ) ;
};

#endif // !defined(AFX_AVT2_11CAMERA_H__84FCD8FA_3B15_4BDE_99A7_EC03545294C4__INCLUDED_)
