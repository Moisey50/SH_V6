// TextView.cpp : implementation file
//

#include "stdafx.h"
#include <math\hbmath.h>
#include <shbase\shbase.h>
#include <gadgets\TextFrame.h>
#include <gadgets\QuantityFrame.h>
#include <gadgets\RectFrame.h>
#include <gadgets\VideoFrame.h>
#include <gadgets\FigureFrame.h>
#include <gadgets\arrayframe.h>
#include <helpers\GraphicsOnImage.h>
#include <helpers\FXParser2.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define REFRESH_TIMER	1
#define REFRESH_RATE	1

/////////////////////////////////////////////////////////////////////////////
// CTextView

CTextView::CTextView()
{
  m_Font.CreateFont(-10, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, 
    OUT_TT_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY, VARIABLE_PITCH | FF_SWISS, "Arial");
  m_TextColor = COLORREF( 0 ) ;
  m_BkColor = COLORREF( 0xffffff ) ;
}

CTextView::~CTextView()
{
  m_Font.DeleteObject();
}


BEGIN_MESSAGE_MAP(CTextView, CStatic)
  //{{AFX_MSG_MAP(CTextView)
  ON_WM_TIMER()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTextView message handlers

BOOL CTextView::Create(CWnd* pParentWnd)
{
  CRect rect(0, 0, 0, 0);
  if (pParentWnd)
    pParentWnd->GetClientRect(rect);
  if (!CStatic::Create("0", WS_CHILD | WS_VISIBLE | SS_LEFT, rect, pParentWnd, AFX_IDW_PANE_FIRST))
    return FALSE;
  SetFont(&m_Font);
  return TRUE;
}

BOOL CTextView::DestroyWindow() 
{
  BOOL bRes = FALSE ;
  m_Lock.Lock();
  if ( m_hWnd )
  {
    KillTimer( REFRESH_TIMER );
    bRes = CStatic::DestroyWindow() ;
  }
  m_Lock.Unlock();
  return bRes ;
}

void CTextView::OnTimer(UINT_PTR nIDEvent) 
{
  m_Lock.Lock();
  SetWindowText(m_Text);
  KillTimer(REFRESH_TIMER);
  m_Lock.Unlock();
}

void CTextView::SetText(LPCTSTR text)
{
  if (!GetSafeHwnd( )) return;
  m_Lock.Lock();
  m_Text = text;
  SetTimer(REFRESH_TIMER, REFRESH_RATE, NULL);
  m_Lock.Unlock();
}

void CTextView::Render(const CDataFrame* pDataFrame)
{
  if (!IsValid()) return;
  FXString text;
  double frTime=pDataFrame->GetTime();
  FXString Label = pDataFrame->GetLabel() ;
  if (!Tvdb400_IsEOS(pDataFrame))
  {
    const CArrayFrame* ArrayFrame = pDataFrame->GetArrayFrame(pDataFrame->GetLabel());
    if (ArrayFrame)
    {
      text.Format("[%d CArrayFrame \"%s\"] {%d %s}", ArrayFrame->GetId(), 
        ArrayFrame->GetLabel(), ArrayFrame->GetCount(),
        Tvdb400_TypeToStr(pDataFrame->GetDataType() / arraytype));
    }
    else if (pDataFrame->GetTextFrame(DEFAULT_LABEL))
    {
      FXString ViewText ;
      ConvBinStringToView( pDataFrame->GetTextFrame( DEFAULT_LABEL )->GetString() , ViewText ) ;
      text.Format("[%d text \"%s\"] \"%s\"",pDataFrame->GetTextFrame(DEFAULT_LABEL)->GetId(),
        pDataFrame->GetTextFrame(DEFAULT_LABEL)->GetLabel(), ViewText );
    }
    else if (pDataFrame->GetRectFrame(DEFAULT_LABEL))
    {
      CRect rc=pDataFrame->GetRectFrame(DEFAULT_LABEL);
      text.Format("[%d rectangle \"%s\"] (%d %d %d %d)",pDataFrame->GetRectFrame(DEFAULT_LABEL)->GetId(),
        pDataFrame->GetRectFrame(DEFAULT_LABEL)->GetLabel(), rc.left,rc.top,rc.right,rc.bottom );
    }
    else if (pDataFrame->GetQuantityFrame(DEFAULT_LABEL))
    {
      FXString AsString = pDataFrame->GetQuantityFrame(DEFAULT_LABEL)->ToString() ;
      text.Format("[%d quantity \"%s\"] (%s)",pDataFrame->GetQuantityFrame(DEFAULT_LABEL)->GetId(),
        pDataFrame->GetQuantityFrame(DEFAULT_LABEL)->GetLabel(), (LPCTSTR)AsString );
    }
    else if (pDataFrame->GetVideoFrame(DEFAULT_LABEL))
    {
      char fourcc[5]; fourcc[4]=0;
      memcpy(fourcc,&pDataFrame->GetVideoFrame(DEFAULT_LABEL)->lpBMIH->biCompression,4);
      int width=pDataFrame->GetVideoFrame(DEFAULT_LABEL)->lpBMIH->biWidth;
      int height=pDataFrame->GetVideoFrame(DEFAULT_LABEL)->lpBMIH->biHeight;
      text.Format("[%d video \"%s\"] %s(%dx%d)",pDataFrame->GetVideoFrame(DEFAULT_LABEL)->GetId(),
        pDataFrame->GetVideoFrame(DEFAULT_LABEL)->GetLabel(), fourcc,width,height);
    }
    else if (pDataFrame->GetFigureFrame(DEFAULT_LABEL))
    {
      int size = (int) pDataFrame->GetFigureFrame(DEFAULT_LABEL)->GetSize();
      text.Format("[%d figure \"%s\"] (%d)",pDataFrame->GetFigureFrame(DEFAULT_LABEL)->GetId(),
        pDataFrame->GetFigureFrame(DEFAULT_LABEL)->GetLabel(), size);
    }
    else if (pDataFrame->GetBooleanFrame(DEFAULT_LABEL))
    {
      bool value=*pDataFrame->GetBooleanFrame(DEFAULT_LABEL);
      text.Format("[%d bool \"%s\"] %s",pDataFrame->GetBooleanFrame(DEFAULT_LABEL)->GetId(),
        pDataFrame->GetBooleanFrame(DEFAULT_LABEL)->GetLabel(), (value)?"true":"false");
    }
    else
    {
      text.Format("[%d %s \"%s\"]",pDataFrame->GetId(),
        Tvdb400_TypeToStr(pDataFrame->GetDataType()), pDataFrame->GetLabel());
    }
    if ( !Label.IsEmpty() )
    {
      int iPos = (int) Label.Find( "color=" ) ;
      if ( iPos >= 0 )
        m_TextColor =(COLORREF) ConvToBinary( (LPCTSTR)Label + 6 ) ;
      iPos = (int) Label.Find( "bkcolor=" ) ;
      if ( iPos >= 0 )
        m_BkColor = (COLORREF) ConvToBinary( (LPCTSTR)Label + 8 ) ;
    }
    SetText(text);
  }
}
