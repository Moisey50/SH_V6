// DShowCapture.h : Declaration of the DShowCapture class

#pragma once
#include <gadgets\gadbase.h>
#include <gadgets\VideoFrame.h>
#include <DShow.h>
#include "VFilter.h"

enum PlaybackState
{
  STATE_RUNNING ,
  STATE_PAUSED ,
  STATE_STOPPED ,
  STATE_CLOSED
};

class DShowCapture : public CCaptureGadget
{
protected:
  FXString            m_fName;
  PlaybackState	      m_state;
  CWnd*               m_NotifyWnd;
  IGraphBuilder	   *  m_pGraph;
  IMediaControl	   *  m_pControl;
//   IMediaPosition   *  m_pMediaPosition ;
  UINT			          m_EventMsg;
  IBaseFilter      *  m_pSource;
  IMediaEventEx	   *  m_pEvent;
  IMediaSeeking	   *  m_pSeek;
  IVideoFrameStep	 *  m_pStep;
  CVFilter         *  m_VideoRenderer;
  CMediaType       *  m_pMediaType;
  DWORD               m_OutputFormat;
  bool                m_LoopClip;
  CInputConnector*    m_pInput;
  CDuplexConnector*	  m_pControlPin;
  double              m_dSpeedFactor ;
private:
  DShowCapture( void );
  void    ShutDown();
  int		DoJob();
  void	AsyncTransaction( CDuplexConnector* pConnector , CDataFrame* pDataFrame );
  // Direct Show controls
  HRESULT InitializeGraph();
  void	TearDownGraph();
  bool    OpenFile( const FXString& fName );
  HRESULT Start();
  HRESULT Stop();
  HRESULT Pause();
  void    DoSend( pTVFrame inFrame );
  HRESULT MoveToNextFrame();
  HRESULT GetDuration( LONGLONG& Duration );
  HRESULT	RenderStream( IBaseFilter *pSource );
  HRESULT SetPosition( REFERENCE_TIME pos );
  HRESULT GetPosition( REFERENCE_TIME& pos );
public:
  bool    ScanProperties( LPCTSTR text , bool& Invalidate );
  bool    PrintProperties( FXString& text );
  bool    ScanSettings( FXString& text );
  int     GetInputsCount();
  CInputConnector* GetInputConnector( int n );
  int		GetDuplexCount();
  CDuplexConnector* GetDuplexConnector( int n );
  ////////
public:
  void    NotifyMediaType( CMediaType *pMediaType );
  HRESULT DoRenderSample( IMediaSample *pMediaSample );
  LRESULT OnFGNotify( WPARAM wParam , LPARAM lParam );
private:
  void TriggerPulse( CDataFrame* pDataFrame ) { pDataFrame->Release(); MoveToNextFrame(); }
  static friend void CALLBACK _fn_capture_trigger( CDataFrame* pDataFrame , void* pGadget , CConnector* lpInput )
  {
    ((DShowCapture*) pGadget)->TriggerPulse( pDataFrame );
  }
  DECLARE_RUNTIME_GADGET( DShowCapture );
};
