// DIBFRender.h: interface for the CDIBFRender class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DIBFRENDER_H__FC26D7DC_550D_4660_9D21_48F8851CE955__INCLUDED_)
#define AFX_DIBFRENDER_H__FC26D7DC_550D_4660_9D21_48F8851CE955__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <video\DIBView.h>
#include <gadgets\gadbase.h>
#include <gadgets\FigureFrame.h>
#include <Helpers\GraphicsOnImage.h>
#include <math/Intf_sup.h>
#include <files/imgfiles.h>
#include <helpers/BigViewWnd.h>

#define MSG_SELECT_WINDOWHANDLE (WM_APP + 335)

#define SL_DISABLE 0
#define SL_SHOW_MAIN 1

#define TMR_MOUSE_MOVE 10
#define MOUSE_MOVE_TIMEOUT 200

enum LengthViewMode
{
  LVM_Disabled = 0 ,
  LVM_ViewSimple,
  LVM_ViewTenth,
  LVM_ViewBoth
};

class CDIBFRender : public CDIBView
{
public:
  CRect	       m_Rect;
  FXLockObject m_Lock;
  const CDataFrame * m_OrgFrame ;
  FXString           m_OrgFrameContent ;
  const CDataFrame * m_pTmpFrame ;
// #ifdef _DEBUG
  int                m_iTmpFrameId;
  FXString           m_TmpFrameLabel;
  int                m_iTmpUserCnt;
// #endif
  FXString           m_TmpFrameContent ;
  FXArray <CFont* , CFont*> m_Fonts;
  CUIntArray   m_Sizes;
  CPen         m_RectanglePen;
  GraphicsData m_GraphicsData ;
  cmplx        m_LastCursor ;
  CPoint       m_LastCursorPt ;
  CPoint       m_CursorPtWhenUpdated ;

  int          m_iCursorForm ;
  LPCSTR       m_LastCalculatedCursor ;
  LPCSTR       m_CurrentCursor ;
  int          m_iLastNearest ;
  CWnd *       m_pView = NULL ;
  CBigViewWnd  m_HelpWnd ;
  CPoint       m_ScrollOffsets[ 17 ] ;
  CPoint       m_NewScrollOffset ;
  int		       m_nIndex;
  BOOL         m_bCutOverlapMode ;
  BOOL         m_bShowLabel ;
  FXString     m_sVideoFrameLabel ;
  double       m_dLastLineLength ;
  CRect        m_rcLastSelectedLine ;
  CRect        m_rcLastSelectedRect ;
  CPoint       m_LastLineLengthTextPos ;
  bool         m_bTrackingTT ;
  bool         m_bTrackingSwitch ;
  bool         m_bShowRGB ;
  bool         m_bFullScreen = false ;
  double       m_dLastUpdateTime ;
  double       m_dPictureDrawTime ;
  double       m_dFullDrawTime ;
  DWORD        m_dwLastFrameTime ;
  int          m_iFrameInterval ;
  cmplx        m_cScale = 1.0 ;
  double       m_dScaleTenthPerUnit ;
  LengthViewMode m_LengthViewMode = LVM_Disabled ;
  FXString     m_sUnits ;
  bool         m_bConjugateScaleConvert ;
  int          m_iSaveImage ;
  int          m_iSendOutImage ;
  FXString     m_FileNameSuffix ;
  FXString     m_FileNamePrefix ;
  FXString     m_ImagesDir ;
  FXString     m_NewHandleAsString ;
  CPoint       m_PointOfInterest;
  bool         m_bSomeSelected ;

  FXSIZE       m_iMouseMoveTimer ;
  CPoint       m_NewImageCenter ;
  int          m_iNewScale = -2 ;

public:
  CDIBFRender( LPCTSTR name = "" ) ;
  virtual ~CDIBFRender();
  virtual BOOL Create( CWnd* pParentWnd , DWORD dwAddStyle = 0 , UINT nID = 0 , LPCTSTR szWindowName = _T( "DIBView" ) );
  void    Render( const CDataFrame* pDataFrame );
  bool    Draw( HDC hdc , RECT& rc );
  void    DrawData( HDC hdc );
  int     CheckCursorForm( bool bMouseMOve , CPoint& CursorPos ) ;
  void    ShowScrollBar( UINT nBar , BOOL bShow = TRUE );
  void    SetScale( double s );
  void    SetScale( double s , CPoint CursorPt );
  void    SetNewViewCenter( CPoint Center ) ;
  double  GetScale() { return m_Scale ; } ;
  void    SetShowLabel( BOOL bShow )
  {
    m_bShowLabel = bShow ;
  }
  void    SetLocalDrawLineEnabled( bool bSet )
  {
    bool bRectEnable = m_Selection->RectEnable ;
    InitSelBlock( bSet , bRectEnable ) ;
    m_dLastLineLength = 0. ;
  }
  void    SetLocalDrawRectEnabled( bool bSet )
  {
    bool bLineEnable = m_Selection->LineEnable ;
    InitSelBlock( bLineEnable , bSet ) ;
    m_dLastLineLength = 0. ;
  }
  pSelection GetSelection() { return m_Selection ; }
  bool    DoScrolling( int& x , int& y , RECT& rc );
  void    ShiftPos( int dx , int dy );
  void    SetViewPos( CPoint LeftTop ) { m_ScrOffset = LeftTop ; };
  CPoint  GetViewPos() { return m_ScrOffset; }
  //CToolTipCtrl * GetToolTip() { return &m_ToolTip ; }
//   HWND    GetToolTipHandle() { return m_hWndTrackingTT; }
//   TOOLINFO * GetToolTipInfo() { return &m_ToolTipInfo; }
  bool    GetTrackingState() { return m_bTrackingTT; }
  void    SetTrackingState( bool bSet ) { m_bTrackingTT = bSet ; }
  CPoint  m_LastMousePos ;

private:
  bool    Pic2Scr( DPOINT& point , CPoint& res );
  bool    Pic2Scr( POINT& point , CPoint& res );
  bool    CreateFontWithSize( UINT uiSize ) ;
  CFont * GetFont( UINT uiSize )
  {
    for ( int i = 0 ; i <= m_Fonts.GetUpperBound(); i++ )
    {
      if ( m_Sizes[ i ] == uiSize ) return m_Fonts[ i ];
    }
    if ( CreateFontWithSize( uiSize ) )
      return m_Fonts[ m_Fonts.GetUpperBound() ];
    else
      return NULL ;
  }
  bool    DrawTexts( CDC* dc );
  bool    DrawFigures( CDC* dc );
  bool    DrawRectangles( CDC* dc );
  bool    LoadGraphics( const CDataFrame * df ) ;
  afx_msg void OnHScroll( UINT nSBCode , UINT nPos , CScrollBar* pScrollBar );
  afx_msg void OnVScroll( UINT nSBCode , UINT nPos , CScrollBar* pScrollBar );
  afx_msg BOOL OnMouseWheel( UINT nFlags , short zDelta , CPoint pt );
  afx_msg void OnMouseMove( UINT nFlags , CPoint point );
  afx_msg void OnLButtonDown( UINT nFlags , CPoint point );
  afx_msg void OnLButtonUp( UINT nFlags , CPoint point );
  //   afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
  //   afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
public:
  DECLARE_MESSAGE_MAP()
  afx_msg void OnSetFocus( CWnd* pOldWnd );
  afx_msg void OnKeyDown( UINT nChar , UINT nRepCnt , UINT nFlags );
  afx_msg void OnMButtonDown( UINT nFlags , CPoint point );
protected:
  virtual void SetExtData( const void * pDataFrameWithGraphics ) ;

  //   virtual BOOL PreTranslateMessage(MSG* pMsg);
  //   virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
public:
  afx_msg void OnLButtonDblClk( UINT nFlags , CPoint point );
  afx_msg void OnSize( UINT nType , int cx , int cy );
  HWND CreateTrackingToolTip( int toolID , TCHAR* pText ) ;
  afx_msg void OnMouseLeave();
  int DrawToolTip( CDC * dc , LPCTSTR pTip = NULL );
  bool FormToolTipText( CPoint& OnImagePt , DWORD dwBitMask ,
    FXString& OutS , bool bExt = false , bool bViewRGB = false , bool bOneString = false ) ;
  afx_msg void OnKeyUp( UINT nChar , UINT nRepCnt , UINT nFlags );
  afx_msg void OnTimer( UINT_PTR nIDEvent );
  cmplx Scr2PicCmplx( CPoint &point );
  cmplx ConvertCoordsRelativeToCenter( cmplx& cRelToCenter ) ;
  void SetImagesDir( LPCTSTR psDir ) { m_ImagesDir = psDir ; }
} ;


#endif // !defined(AFX_DIBFRENDER_H__FC26D7DC_550D_4660_9D21_48F8851CE955__INCLUDED_)
