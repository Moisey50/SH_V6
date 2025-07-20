// FLWCaptureGadget.h: interface for the FLWCapture class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FLWCAPTUREGADGET_H__CCAABD7F_74A4_48F9_9277_EE044309195D__INCLUDED_)
#define AFX_FLWCAPTUREGADGET_H__CCAABD7F_74A4_48F9_9277_EE044309195D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FLWGadget.h"
#include <gadgets\gadbase.h>
#include <gadgets\flwarchive.h>
#include "FLWLogRecord.h"

class CFLWCapStpDialog;
class FLWCapture : public CCaptureGadget
{
protected:
  CFLWArchive       m_Archive;
  CDuplexConnector*	m_pControl;
  CInputConnector*	m_pInputTrigger;
  unsigned          m_NextFrame;
  double			    	m_NextTime;
  bool				      m_bStopping;
  bool              m_bRewind ;
  FLWLog            m_Log ;
  BOOL              m_bDoLog ;
  int               m_FrameNumberMin = 0;
  int               m_FrameNumberMax = 100 ;
  double            m_dFrameTimeMin_ms = 0. ;
  double            m_dFrameTimeMax_ms = 1000. ;
  int               m_PlayNumberMin = 0 ;
  int               m_PlayNumberMax = 100 ;
  double            m_dPlayTimeMin_ms = 0. ;
  double            m_dPlayTimeMax_ms = 1000. ;
  double            m_dTimeZoom = 1.0 ;
public:
  FLWCapture();
  virtual void ShutDown();
  virtual int	 GetInputsCount();
  CInputConnector* GetInputConnector( int n );
  virtual void OnStart();
  virtual void OnStop();
  virtual CDataFrame* GetNextFrame( double* StartTime );
  virtual void AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pParamFrame );
  virtual int GetDuplexCount();
  virtual CDuplexConnector* GetDuplexConnector( int n );
  virtual bool PrintProperties( FXString& text );
  virtual bool ScanProperties( LPCTSTR text , bool& Invalidate );
  bool ScanSettings( FXString& text );
protected:
  void TriggerPulse( CDataFrame* pDataFrame ) { pDataFrame->Release(); m_Archive.SkipDelay(); };
  void ToggleExternalTrigger();
  static friend void CALLBACK _fn_capture_trigger( CDataFrame* pDataFrame , void* pGadget , CConnector* lpInput )
  {
    ( ( FLWCapture* )pGadget )->TriggerPulse( pDataFrame );
  }
  DECLARE_RUNTIME_GADGET( FLWCapture );
};

#endif // !defined(AFX_FLWCAPTUREGADGET_H__CCAABD7F_74A4_48F9_9277_EE044309195D__INCLUDED_)
