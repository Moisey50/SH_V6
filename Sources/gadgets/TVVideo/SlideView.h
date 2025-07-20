// SlideView.h : Declaration of the SlideView class

#pragma once
#include <gadgets\gadbase.h>
#include <gadgets\textframe.h>
#include <gadgets\FigureFrame.h>
#include "SlideViewRender.h"

class SlideView : public CRenderGadget
{
private:
  COutputConnector *  m_pOutput;
  COutputConnector *  m_pSpectrumOutput ;
  COutputConnector *  m_pSpectrumAsTextOutput ;
  CSlideViewRender   *m_wndOutput;
  FXLockObject        m_Lock;
  bool                m_Rescale;
  int                 m_Scale;
  bool                m_Monochrome;
  int                 m_TargetWidth;
  int                 m_FramesLen;
  int                 m_FramesInRow;
  bool                m_ShowMetafiles;
  bool                m_Shift;
  bool                m_bShiftWasPressed ;
  bool                m_bCtrlWasPressed ;
  int                 m_iIntegrationRadius ;
  FXString            m_FigureLabel ;
  int                 m_iColorCnt ;
  FXStringArray       m_Colors ;
  FXString            m_LastCaption ;
  FXStringArray       m_Captions ;
public:
  void ShutDown();
  void Attach( CWnd* pWnd );
  void Detach();
private:
  SlideView( void );
  int     GetOutputsCount() { return 3; }
  COutputConnector* GetOutputConnector( int n ) 
  {
    switch ( n )
    {
      case 0: return m_pOutput ;
      case 1: return m_pSpectrumOutput ;
      case 2: return m_pSpectrumAsTextOutput ;
      default: return NULL ;
    }
  }
  void    Render( const CDataFrame* pDataFrame );
  CWnd*   GetRenderWnd() { return m_wndOutput; }
  bool    PrintProperties( FXString& text );
  bool    ScanProperties( LPCTSTR text , bool& Invalidate );
  bool    ScanSettings( FXString& text );
  void    GetDefaultWndSize( RECT& rc ) { rc.left = rc.top = 0; rc.right = GADGET_VIDEO_WIDTH; rc.bottom = GADGET_VIDEO_HEIGHT; }
  void    DispFunc( CDataFrame* pDataFrame );
  void    ApplySettings();
  double GetAverageValue( LPBYTE pImage , DWORD dwShift , 
    DWORD dwZone , DWORD dwStep , bool b16 ) ;

  DECLARE_RUNTIME_GADGET( SlideView );
  friend void SlideView_DispFunc( CDataFrame* pDataFrame , void *pParam );
};
