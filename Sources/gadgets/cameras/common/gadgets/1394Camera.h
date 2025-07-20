// 1394Camera.h: interface for the C1394Camera class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_1394CAMERA_H__2D237E9D_6C31_4347_A976_91CF227172B3__INCLUDED_)
#define AFX_1394CAMERA_H__2D237E9D_6C31_4347_A976_91CF227172B3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include <gadgets\VideoFrame.h>
#include <gadgets\gadbase.h>

namespace CAMERA1394
{
#define SYNC_INPUT 1
#define CTRL_INPUT 2

#define WRONG_PROPERTY (-1000)

  class CameraInfo
  {
  public:
    CameraInfo() { serialnmb = 0; }
    unsigned long serialnmb ;
    FXString       Id ;
    FXString       name ;
    FXString       model ;
    FXString       szSerNum ;
    FXString       InterfaceId ;
  } ;

  typedef struct tagProperty
  {
    unsigned    id;
    FXString       name;
    FXParser    property;
  }Property;
};

#define MAX_CAMERASNMB 16

#define C1394_SENDINFO_0(sz)        FxSendLogMsg(MSG_INFO_LEVEL,GetDriverInfo(),0,sz)
#define C1394_SENDINFO_1(sz,a)      FxSendLogMsg(MSG_INFO_LEVEL,GetDriverInfo(),0,sz,a)
#define C1394_SENDINFO_2(sz,a,b)    FxSendLogMsg(MSG_INFO_LEVEL,GetDriverInfo(),0,sz,a,b)
#define C1394_SENDINFO_3(sz,a,b,c)  FxSendLogMsg(MSG_INFO_LEVEL,GetDriverInfo(),0,sz,a,b,c)

#define C1394_SENDTRACE_0(sz)       FxSendLogMsg(MSG_DEBUG_LEVEL,GetDriverInfo(),0,sz)
#define C1394_SENDTRACE_1(sz,a)     FxSendLogMsg(MSG_DEBUG_LEVEL,GetDriverInfo(),0,sz,a)
#define C1394_SENDTRACE_2(sz,a,b)   FxSendLogMsg(MSG_DEBUG_LEVEL,GetDriverInfo(),0,sz,a,b)
#define C1394_SENDTRACE_3(sz,a,b,c) FxSendLogMsg(MSG_DEBUG_LEVEL,GetDriverInfo(),0,sz,a,b,c)

#define C1394_SENDWARN_0(sz)        FxSendLogMsg(MSG_WARNING_LEVEL,GetDriverInfo(),0,sz)
#define C1394_SENDWARN_1(sz,a)      FxSendLogMsg(MSG_WARNING_LEVEL,GetDriverInfo(),0,sz,a)
#define C1394_SENDWARN_2(sz,a,b)    FxSendLogMsg(MSG_WARNING_LEVEL,GetDriverInfo(),0,sz,a,b)
#define C1394_SENDWARN_3(sz,a,b,c)  FxSendLogMsg(MSG_WARNING_LEVEL,GetDriverInfo(),0,sz,a,b,c)

#define C1394_SENDERR_0(sz)         FxSendLogMsg(MSG_ERROR_LEVEL,GetDriverInfo(),0,sz)
#define C1394_SENDERR_1(sz,a)       FxSendLogMsg(MSG_ERROR_LEVEL,GetDriverInfo(),0,sz,a)
#define C1394_SENDERR_2(sz,a,b)     FxSendLogMsg(MSG_ERROR_LEVEL,GetDriverInfo(),0,sz,a,b)
#define C1394_SENDERR_3(sz,a,b,c)   FxSendLogMsg(MSG_ERROR_LEVEL,GetDriverInfo(),0,sz,a,b,c)

#define C1394_SENDFAIL_0(sz)        FxSendLogMsg(MSG_CRITICAL_LEVEL,GetDriverInfo(),0,sz)
#define C1394_SENDFAIL_1(sz,a)      FxSendLogMsg(MSG_CRITICAL_LEVEL,GetDriverInfo(),0,sz,a)
#define C1394_SENDFAIL_2(sz,a,b)    FxSendLogMsg(MSG_CRITICAL_LEVEL,GetDriverInfo(),0,sz,a,b)
#define C1394_SENDFAIL_3(sz,a,b,c)  FxSendLogMsg(MSG_CRITICAL_LEVEL,GetDriverInfo(),0,sz,a,b,c)



class C1394Camera : public CCaptureGadget /*,   public FXInstanceCntr*/
{
protected: // gadget stuff
  FXString          m_GadgetInfo;
  CInputConnector*  m_pInputTrigger;
  CDuplexConnector* m_pControl;
  HANDLE			      m_evSWTriggerPulse;
  HANDLE            m_TriggerEvents[ 2 ] ;
  FXString          m_Status;
  bool              m_bCameraRunning ;
  BOOL              m_bInScanProperties ;
  bool              m_bInsertCamera ;
  bool              m_bGadgetInputConnected ;
  bool              m_bSoftwareTriggerMode ;
  bool              m_bHardwareTrigger ;
  bool              m_bTriggerOnPinReceived ;
protected: // HW specific stuff
  unsigned    m_FrameNmb;
  unsigned    m_CamerasOnBus;
  unsigned    m_CurrentCamera;
  CAMERA1394::CameraInfo  m_CamerasInfo[ MAX_CAMERASNMB ];
  CArray<CAMERA1394::Property , CAMERA1394::Property&> m_Properties;
protected:
  unsigned    SerialToNmb( unsigned serial );
  bool        IsDigitField( unsigned ID );
public:
  C1394Camera( DWORD Inputs = SYNC_INPUT | CTRL_INPUT );
  ~C1394Camera() {} ;
  // helpers
  unsigned GetPropertyId( LPCTSTR name );
  // HW stuff
  virtual bool DriverInit() { return false; }
  virtual bool EnumCameras() { return false; }
  virtual bool CameraInit() { return false; }
  virtual void CameraClose() {}

  virtual bool DriverValid();
  virtual bool CameraStart();
  virtual void CameraStop() { m_bRun = FALSE ; };
  virtual bool IsCameraRunning() { return false ; }

  virtual bool IsRunning();

  virtual bool GetCameraProperty( unsigned i , FXSIZE &value , bool& bauto ) { return false; }
  virtual bool SetCameraProperty( unsigned i , FXSIZE &value , bool& bauto , bool& Invalidate ) { return false; }

  virtual void CameraTriggerPulse( CDataFrame* pDataFrame );

  virtual CVideoFrame* CameraDoGrab( double* dStartTime );

  // Capture gadget stuff
  virtual void ShutDown();

  virtual void OnStart()
  {
    CameraStart();
    CCaptureGadget::OnStart();
  }
  virtual void OnStop()
  {
    CameraStop();
    CCaptureGadget::OnStop();
  }

  virtual bool PrintProperties( FXString& text );
  virtual bool ScanProperties( LPCTSTR text , bool& Invalidate );
  virtual bool ScanSettings( FXString& text );
  virtual bool ShouldBeStoppedOnReprogramming() { return false ; }

  virtual CDataFrame* GetNextFrame( double * dStartTime );

  virtual int GetInputsCount() { return (m_pInputTrigger) ? 1 : 0; }
  virtual CInputConnector* GetInputConnector( int n ) { return (n == 0) ? m_pInputTrigger : NULL; }
  virtual int GetDuplexCount() { return (m_pControl) ? 1 : 0; }
  virtual CDuplexConnector* GetDuplexConnector( int n ) { return (n == 0) ? m_pControl : NULL; }

  virtual void AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame );

  virtual LPCTSTR GetDriverInfo() { return m_GadgetInfo; }
  virtual bool SetSoftwareTriggerMode( bool bSet )
  {
    m_bSoftwareTriggerMode = bSet ;
    return true ;
  }
  // SOft trigger mode indicator
  virtual bool IsTriggerByInputPin() { return ((m_pInputTrigger) && (m_pInputTrigger->IsConnected())); }
  bool CheckAndResetSoftTriggerReceived()
  {
    if ( IsTriggerByInputPin() )
    {
      if ( m_bSoftwareTriggerMode )
      {
        if ( m_bTriggerOnPinReceived )
        {
          m_bTriggerOnPinReceived = false ;
          return true ;
        }

      }
    }
    return false ;
  }
  static /*friend*/ void CALLBACK c1394c_fn_capture_trigger( CDataFrame* pDataFrame , void* pGadget , CConnector* lpInput )
  {
    ((C1394Camera*) pGadget)->CameraTriggerPulse( pDataFrame );
  }
  DECLARE_RUNTIME_GADGET( C1394Camera );
};

#endif // !defined(AFX_1394CAMERA_H__2D237E9D_6C31_4347_A976_91CF227172B3__INCLUDED_)
