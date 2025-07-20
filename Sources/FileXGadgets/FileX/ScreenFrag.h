#pragma once
#include "helpers/UserBaseGadget.h"
#include "CMouseHook.h"

enum SF_CaptureMode
{
  SFCM_Idle = 0,
  SFCM_Absolute ,
  SFCM_Display ,
  SFCM_WindowName ,
  SFCM_Handle ,
  SFCM_ByCursor
};

enum SF_OutputFormat
{
  SF_Unknown = 0 ,
  SF_Y800 ,
  SF_RGB ,
  SF_YUV9 ,
  SF_YUV12
};

enum SF_Timing
{
  SFT_ByTrigger = 0 ,
  SFT_ByTimer
};
class ScreenFrag :
//  public UserBaseGadget
  public UserCaptureBaseGadget
{
public:
  CRect m_CaptureArea ;
  SF_CaptureMode  m_CaptureMode ;
  SF_OutputFormat m_OutputFormat ;
  SF_Timing       m_Timing ;
  int             m_iNTrigger = 0 ;
  double          m_OutPeriod_ms = 100. ;
  double          m_dLastFrameTime_ms = 0. ;
  FXString m_GadgetName ;
  FXString m_WindowName ;
  HWND     m_hWindow ;
  FXString m_hWindowAsText ;
  int      m_iDisplay ;
  CMouseHook * m_Hook ;
  bool     m_bInRreprogram ;
public:
  DECLARE_RUNTIME_GADGET( ScreenFrag );
  ScreenFrag() ;
  void ShutDown();

  void PropertiesRegistration();
  void ConnectorsRegistration();
  static void CALLBACK fn_capture_trigger( CDataFrame* pDataFrame , void* pGadget , CConnector* lpInput )
  {
    ( ( ScreenFrag* ) pGadget )->DeviceTriggerPulse( pDataFrame );
  }
  void DeviceTriggerPulse( CDataFrame* pDataFrame )
  {
    if ( Tvdb400_IsEOS( pDataFrame ) )
      /*m_Timing = SFT_ByTimer*/ ;
    else
    {
      if ( m_Timing == SFT_ByTrigger )
        m_iNTrigger++ ;
      else
        m_iNTrigger = 0 ;
    }
    pDataFrame->Release();
  }

  virtual bool CameraStart() { return true ; } ;
  virtual void CameraStop() {} ;
  virtual void OnStart()
  {
//    CameraStart() ;
    CCaptureGadget::OnStart() ;
    return ;
  }
  virtual void OnStop()
  {
//    CameraStop() ;
    CCaptureGadget::OnStop() ;
    return ;
  }
//   CDataFrame* DoProcessing( const CDataFrame* pDataFrame );
  CDataFrame* GetScreeny() ;
  static void CaptureModeChange( LPCTSTR pName , void* pObject ,
    bool& bInvalidate , bool& bRescan ) ; 

  virtual CDataFrame* GetNextFrame( double * dStartTime );
  virtual int  GetInputsCount()
  {
    return UserGadgetBase::GetInputsCount();
  }
  virtual int GetOutputsCount()
  {
    return UserGadgetBase::GetOutputsCount();
  }
  virtual int GetDuplexCount()
  {
    return UserGadgetBase::GetDuplexCount();
  }

  virtual CInputConnector*		GetInputConnector( int n )
  {
    return UserGadgetBase::GetInputConnector( n );
  }
  virtual COutputConnector*	GetOutputConnector( int n )
  {
    return UserGadgetBase::GetOutputConnector( n );
  }
  virtual CDuplexConnector*	GetDuplexConnector( int n )
  {
    return UserGadgetBase::GetDuplexConnector( n );
  }

};

