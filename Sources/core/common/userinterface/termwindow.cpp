// TermWindow.cpp : implementation file
//

#include "stdafx.h"
#include <shbase\shbase.h>
#include <gadgets\TextFrame.h>
#include <gadgets\QuantityFrame.h>
#include <gadgets\RectFrame.h>
#include <gadgets\videoframe.h>
#include <gadgets\figureframe.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define THIS_MODULENAME "TermWindow"

#define TERMWINDOW_SELBGCOLOR	RGB(0, 128, 255)
#define TERMWINDOW_SELFGCOLOR	RGB(255, 255, 255)
#define TERMWINDOW_DEFBGCOLOR	RGB(255, 255, 255)
#define TERMWINDOW_DEFFGCOLOR	RGB(0, 0, 0)

/////////////////////////////////////////////////////////////////////////////
// CTermWindow

CTermWindow::CTermWindow() :
  m_WriteQueue(400),
  m_CurLine(""),
  m_nSelLine(0),
  m_bDrawPartial(FALSE),
  m_nTopLine(0),
  m_OnlyOneFrame(FALSE),
  m_Invalidate(false),
  m_bViewLabel(false),
  m_bViewTiming(false),
  m_bClear(false)
{}

CTermWindow::~CTermWindow()
{
  while (m_WriteQueue.ItemsInQueue())
  {
    FXString d;
    m_WriteQueue.GetQueueObject(d);
  }
}

BEGIN_MESSAGE_MAP(CTermWindow, CWnd)
  //{{AFX_MSG_MAP(CTermWindow)
  ON_WM_CREATE()
  ON_WM_VSCROLL()
  ON_WM_SIZE()
  ON_WM_PAINT()
  ON_WM_LBUTTONDOWN()
  ON_WM_DESTROY()
  ON_WM_TIMER()
  ON_WM_MOUSEWHEEL()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTermWindow message handlers


BOOL CTermWindow::Create(CWnd* pParentWnd)
{
  RECT rect;
  pParentWnd->GetClientRect(&rect);

  m_Brush.CreateSolidBrush(RGB(255, 255, 255));
  LPCTSTR cName = ::AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_PARENTDC, AfxGetApp()->LoadCursor(IDC_ARROW), HBRUSH(m_Brush));
  DWORD style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_BORDER;

  m_Font.CreateFont(-10, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY, VARIABLE_PITCH | FF_SWISS, "Arial");
  BOOL res = CWnd::Create(cName, "", style, rect, pParentWnd, AFX_IDW_PANE_FIRST);
  if (res)
    m_Timer = SetTimer(1, 10, NULL);
  return res;
}

void CTermWindow::OnDestroy()
{
  CWnd::OnDestroy();
}


int CTermWindow::GetLinesInView()
{
  CDC* pDC = GetDC();
  CFont* oldFont = pDC->SelectObject(&m_Font);
  FXString text("A");
  CSize sz = pDC->GetTextExtent(text, (int)text.GetLength());
  CRect rc;
  GetClientRect(rc);
  pDC->SelectObject(oldFont);
  ReleaseDC(pDC);
  int nLines = rc.Height() / sz.cy;
  if (m_bDrawPartial && (rc.Height() - nLines * sz.cy >= 3 * sz.cy / 4))
    nLines++;
  else if (rc.Height() % sz.cy)
    nLines--;
  return nLines;
}

int CTermWindow::GetLineHeight()
{
  CDC* pDC = GetDC();
  CFont* oldFont = pDC->SelectObject(&m_Font);
  FXString text("A");
  CSize sz = pDC->GetTextExtent(text, (int)text.GetLength());
  pDC->SelectObject(oldFont);
  ReleaseDC(pDC);
  return sz.cy;
}

int CTermWindow::GetLinesCount()
{
  int cLines = m_WriteQueue.ItemsInQueue();
  if (!m_CurLine.IsEmpty())
    cLines++;
  return cLines;
}

int CTermWindow::GetTopIndex()
{
  return m_nTopLine;
}

int CTermWindow::GetBottomIndex()
{
  int index = GetTopIndex() + GetLinesInView();
  if (index >= GetLinesCount())
    return GetLinesCount() - 1;
  return index;
}

int CTermWindow::IndexFromPt(CPoint& pt)
{
  int nIndex = pt.y / GetLineHeight() + GetTopIndex();
  if (nIndex >= GetLinesCount())
    return -1;
  return nIndex;
}

void CTermWindow::SelectLine(int index)
{
  if (index < 0)
    return;
  if (index != m_nSelLine)
  {
    m_nSelLine = index;
    UpdateVertBar();
    Invalidate();
  }
}

void CTermWindow::PutLine(FXString& Line)
{
  while (!m_WriteQueue.PutQueueObject(Line))
  {
    FXString Tmp;
    m_WriteQueue.GetQueueObject(Tmp);
    if (m_nSelLine >= 0)
      m_nSelLine--;
  }
}

void CTermWindow::UpdateVertBar()
{
  BOOL bNeedScroll = (GetLinesCount() > GetLinesInView());
  ShowScrollBar(SB_VERT, bNeedScroll);
  if (bNeedScroll)
  {
    SetScrollRange(SB_VERT, 0, GetLinesCount() - GetLinesInView());
    if (m_nSelLine > GetBottomIndex())
      m_nTopLine = m_nSelLine - GetLinesInView() + 1;
    SetScrollPos(SB_VERT, GetTopIndex());
  }
}

void CTermWindow::AutoScroll()
{
  m_nSelLine = GetLinesCount() - 1;
}

int CTermWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (CWnd::OnCreate(lpCreateStruct) == -1)
    return -1;
  return 0;
}

bool CTermWindow::AppendText(LPCTSTR Str)
{
  if (!Str)
    return false;
  LPCTSTR ptr = Str;
  while (*ptr)
  {
    if (*ptr == '\t')
      m_CurLine += _T("    ");
    else if (*ptr == '\n'
      || *ptr == '\r')
    {
      FXString copy = m_CurLine;
      m_CurLine.Empty();
      PutLine(copy);
      if (*(ptr + 1) == '\r'
        || *(ptr + 1) == '\n')
      {
        ptr++;
      }
    }
    else if (*ptr != '\r')
      m_CurLine += *ptr;
    ptr++;
  }
  m_Invalidate = true;
  return true;
}

bool CTermWindow::SetText(LPCTSTR Str)
{
  if (!Str)
    return false;
  m_WriteQueue.ClearQueue();
  return AppendText(Str);
}


void CTermWindow::Render(const CDataFrame* pDataFrame)
{
  if (!Tvdb400_IsEOS(pDataFrame)) //Do net render EOS
  {
    FXString outp;
    if ( !pDataFrame->IsContainer() )
    {
      const CTextFrame * pText = pDataFrame->GetTextFrame();
      if ( pText )
      {
        LPCTSTR pLabel = pText->GetLabel();
        if ( pLabel && *pLabel )
        {
//           if ( strstr( pLabel , "ToggleT" ) == pLabel )
//           {
//             if ( pText->GetString() == _T( "Set" ) )
//               m_bViewTiming = true;
//             else if ( pText->GetString() == _T( "Reset" ) )
//               m_bViewTiming = false;
//             else
//               m_bViewTiming = !m_bViewTiming;
//             return;
//           }
//           else if ( strstr( pLabel , "ToggleL" ) == pLabel )
//           {
//             if ( pText->GetString() == _T( "Set" ) )
//               m_bViewLabel = true;
//             else if ( pText->GetString() == _T( "Reset" ) )
//               m_bViewLabel = false;
//             else
//               m_bViewLabel = !m_bViewLabel;
//             return;
//           }
//          else 
          if ( strstr( pLabel , "ClearScreen" ) == pLabel )
          {
            if ( pText->GetString() == _T( "OneFrameShow" ) )
              m_OnlyOneFrame = TRUE;
            else
              m_OnlyOneFrame = FALSE;
            m_bClear = true;
            return;
          }
        }
      }
    }
    m_iLevel = 0 ;
    FormFrameTextView(pDataFrame, outp );
    if (!outp.IsEmpty())
    {
      if (m_OnlyOneFrame)
        SetText(outp);
      else
        AppendText(outp);
    }
  }
  return;
  //  FXString outp;
  //
  //    if ( Tvdb400_TypesCompatible( pDataFrame->GetDataType() , text ) )
  //    {
  //      CFramesIterator* Iterator = pDataFrame->CreateFramesIterator( text );
  //      if ( Iterator )
  //      {
  //        CTextFrame* TextFrame = (CTextFrame*) Iterator->Next( DEFAULT_LABEL );
  //        while ( TextFrame )
  //        {
  //          FormFrameTextView( TextFrame , outp ) ;
  //          //FXString tmpS;
  //          //FXString Text = TextFrame->GetString() ;
  //          //int iTextLen = Text.GetLength() ;
  //          //bool bIsCRLF = (Text[ iTextLen - 1 ] == '\r' || Text[ iTextLen - 1 ] == '\n') ;
  //          //if ( m_bViewTiming )
  //          //{
  //          //  tmpS.Format( "text [%d:%s:%d]:\t%s%s" , pDataFrame->GetId() ,
  //          //    TextFrame->GetLabel() , (UINT) GetHRTickCount() , 
  //          //    (LPCTSTR) Text , (bIsCRLF) ? "" : "\r\n" );
  //          //}
  //          //else
  //          //{
  //          //  tmpS.Format( "text [%d:%s]:\t%s%s" , pDataFrame->GetId() ,
  //          //    TextFrame->GetLabel() , (LPCTSTR) Text , (bIsCRLF) ? "" : "\r\n" );
  //          //}
  //          //outp += tmpS;
  //          TextFrame = (CTextFrame*) Iterator->Next( DEFAULT_LABEL );
  //        }
  //        delete Iterator;
  //      }
  //      else
  //      {
  //        const CTextFrame* TextFrame = pDataFrame->GetTextFrame( DEFAULT_LABEL );
  //        if ( TextFrame )
  //        {
  //          LPCTSTR pLabel = TextFrame->GetLabel() ;
  //          if ( pLabel && *pLabel && (strstr( pLabel , "ToggleT" ) == pLabel) )
  //          {
  //            m_bViewTiming = !m_bViewTiming ;
  //            return ;
  //          }
  //          else if ( pLabel && *pLabel && (strstr( pLabel , "ToggleL" ) == pLabel) )
  //          {
  //            m_bViewLabel = !m_bViewLabel ;
  //            return ;
  //          }
  //          FormFrameTextView( TextFrame , outp ) ;
  //
  // /*         LPCTSTR pLabel = TextFrame->GetLabel() ;
  //          if ( pLabel && *pLabel && (strstr( pLabel , _T( "TimingOn" ) ) == pLabel) )
  //            m_bViewTiming = true ;
  //          else if ( pLabel && *pLabel && (strstr( pLabel , _T( "TimingOff" ) ) == pLabel) )
  //            m_bViewTiming = false ;
  //          else
  //          {
  //            FXString Text = TextFrame->GetString() ;
  //            int iTextLen = Text.GetLength() ;
  //            bool bIsCRLF = iTextLen
  //              && (Text[ iTextLen - 1 ] == '\r' || Text[ iTextLen - 1 ] == '\n') ;
  //            if ( m_bViewTiming )
  //            {
  //              outp.Format( "text [%d:%s:%u]:\t%s%s" , pDataFrame->GetId() ,
  //                pLabel , (UINT)GetHRTickCount() , (LPCTSTR) Text , (bIsCRLF) ? "" : "\r\n" ) ;
  //            }
  //            else
  //            {
  //              outp.Format( "text [%d:%s]:\t%s%s" , pDataFrame->GetId() ,
  //                pLabel , (LPCTSTR) Text , (bIsCRLF) ? "" : "\r\n" );
  //            }
  //          }*/
  //        }
  //      }
  //    }
  //    else if ( Tvdb400_TypesCompatible( pDataFrame->GetDataType() , quantity ) )
  //    {
  //      CFramesIterator* Iterator = pDataFrame->CreateFramesIterator( quantity );
  //      if ( Iterator )
  //      {
  //        const CQuantityFrame* QuantityFrame = (CQuantityFrame*) Iterator->Next( DEFAULT_LABEL );
  //
  //        FXString values;
  //        while ( QuantityFrame )
  //        {
  //          values += QuantityFrame->ToString() + _T( "; " );
  //          QuantityFrame = (CQuantityFrame*) Iterator->Next( DEFAULT_LABEL );
  //        }
  //        delete Iterator;
  //        if ( m_bViewTiming )
  //        {
  //          outp.Format( "quantity [%d:%s:%u]\t%s\r\n" , pDataFrame->GetId() , 
  //            pDataFrame->GetLabel() , (UINT) GetHRTickCount() , values );
  //        }
  //        else
  //        {
  //          outp.Format( "quantity:\t%d:%s\t%s\r\n" , pDataFrame->GetId() , 
  //            pDataFrame->GetLabel() , values );
  //        }
  //      }
  //      else
  //      {
  //        const CQuantityFrame* QuantityFrame = pDataFrame->GetQuantityFrame( DEFAULT_LABEL );
  //        if ( QuantityFrame )
  //        {
  //          if ( m_bViewTiming )
  //          {
  //            outp.Format( "quantity [%d:%s:%u]\t%s\r\n" , pDataFrame->GetId() ,
  //              pDataFrame->GetLabel() , (UINT) GetHRTickCount() , QuantityFrame->ToString() );
  //          }
  //          else
  //          {
  //            outp.Format( "quantity: [%d:%s]\t%s\r\n" , pDataFrame->GetId() ,
  //              pDataFrame->GetLabel() , QuantityFrame->ToString() );
  //          }
  //        }
  //      }
  //    }
  //    else if ( Tvdb400_TypesCompatible( pDataFrame->GetDataType() , logical ) )
  //    {
  //      const CBooleanFrame* BooleanFrame = pDataFrame->GetBooleanFrame( DEFAULT_LABEL );
  //      if ( m_bViewTiming )
  //      {
  //        outp.Format( "logical [%d:%s:%u]\t%s\r\n" , pDataFrame->GetId() ,
  //          pDataFrame->GetLabel() , (UINT) GetHRTickCount() , 
  //          (BooleanFrame->operator bool() ? "true" : "false") );
  //      }
  //      else
  //      {
  //        outp.Format( "logical [%d:%s]\t%s\r\n" , pDataFrame->GetId() ,
  //          pDataFrame->GetLabel() , (BooleanFrame->operator bool() ? "true" : "false") );
  //      }
  //    }
  //    else if ( Tvdb400_TypesCompatible( pDataFrame->GetDataType() , rectangle ) )
  //    {
  //      const CRectFrame* RectFrame = pDataFrame->GetRectFrame( DEFAULT_LABEL );
  //      if ( RectFrame )
  //      {
  //        CRect rc = (LPRECT) RectFrame;
  //        if ( m_bViewTiming )
  //        {
  //          outp.Format( "rectangle [%d:%s:%u]\t(%d,%d,%d,%d)\r\n" , pDataFrame->GetId() ,
  //            pDataFrame->GetLabel() , (UINT) GetHRTickCount() ,
  //            rc.left , rc.top , rc.right , rc.bottom );
  //        }
  //        else
  //        {
  //          outp.Format( "rectangle [%d:%s]\t(%d,%d,%d,%d)\r\n" , pDataFrame->GetId() ,
  //            pDataFrame->GetLabel() , rc.left , rc.top , rc.right , rc.bottom );
  //        }
  //      }
  //    }
  //    else
  //    {
  //      if ( m_bViewTiming )
  //      {
  //        outp.Format( "[%d:%s:%u]\ttype: %s\r\n" , pDataFrame->GetId() ,
  //          pDataFrame->GetLabel() , (UINT) GetHRTickCount() ,
  //          Tvdb400_TypeToStr( pDataFrame->GetDataType() ) );
  //      }
  //      else
  //      {
  //        outp.Format( "[%d:%s]\ttype: %s\r\n" , pDataFrame->GetId() ,
  //          pDataFrame->GetLabel() , Tvdb400_TypeToStr( pDataFrame->GetDataType() ) );
  //      }
  //    }
  //    if ( !outp.IsEmpty() )
  //    {
  //      if ( m_OnlyOneFrame )
  //      SetText( outp );
  //      else
  //      AppendText( outp );
  //    }
  ////  }
}

void CTermWindow::FormFrameTextView(
  const CDataFrame * pDataFrame, FXString& outp , int iLevel )
{
  FXString tmpS;
  tmpS.Format("%s%s [%u-%u", (LPCTSTR)m_Prefix ,
    pDataFrame->IsContainer() ?
    _T("Container") : (LPCTSTR)Tvdb400_TypeToStr(pDataFrame->GetDataType()),
          pDataFrame->GetId() , pDataFrame->GetUserCnt() );
  outp += tmpS;
  if (m_bViewLabel)
  {
    tmpS.Format(":%s", pDataFrame->GetLabel());
    outp += tmpS;
  }
  if (m_bViewTiming)
  {
    DWORD dwTickCount = ROUND(GetHRTickCount());
    tmpS.Format(":%u(%u)", dwTickCount, dwTickCount - m_dwPrevTickCount);
    outp += tmpS;
    m_dwPrevTickCount = dwTickCount;
  }
  outp += ']';
  if ( pDataFrame->IsContainer() )
  {
    outp += _T( "\n" ) ;
    FXString OldPrefix = m_Prefix ;
    m_Prefix += _T( "  " ) ;
    m_iLevel++ ;
    CFramesIterator* Iterator = pDataFrame->CreateFramesIterator( transparent );
    if ( Iterator )
    {
      const CDataFrame* pNextFrame = Iterator->NextChild( DEFAULT_LABEL );
      while ( pNextFrame )
      {
        FormFrameTextView( pNextFrame , outp );
        pNextFrame = Iterator->NextChild( DEFAULT_LABEL );
      }
      delete Iterator;
    }
    m_Prefix = OldPrefix ;
    m_iLevel-- ;
  }
  else
  {
    switch ( pDataFrame->GetDataType() )
    {
    case text:
      {
        const CTextFrame* TextFrame = pDataFrame->GetTextFrame();
        FXString Text = TextFrame->GetString();
        int iTextLen = (int) Text.GetLength();
        bool bIsCRLF = iTextLen
          && (Text[ iTextLen - 1 ] == '\r' || Text[ iTextLen - 1 ] == '\n');
        tmpS.Format( "\t%s%s" ,
          (LPCTSTR) Text , (bIsCRLF) ? "" : "\r\n" );
      }
      break;
    case quantity:
      {
        const CQuantityFrame* QuantityFrame =
          pDataFrame->GetQuantityFrame();
        tmpS.Format( "\t%s\r\n" , QuantityFrame->ToString() );
      }
      break;
    case logical:
      {
        const CBooleanFrame* BooleanFrame =
          pDataFrame->GetBooleanFrame();
        tmpS.Format( "\t%s\r\n" ,
          (BooleanFrame->operator bool() ? "true" : "false") );
      }
      break;
    case rectangle:
      {
        const CRectFrame* RectFrame = pDataFrame->GetRectFrame();
        CRect rc = (LPRECT) RectFrame;
        tmpS.Format( "\t(%d,%d,%d,%d)\r\n" ,
          rc.left , rc.top , rc.right , rc.bottom );
      }
      break;
    case figure:
      {
        const CFigureFrame * pFig = pDataFrame->GetFigureFrame() ;
        tmpS.Format( "\t%d pts\r\n" , pFig->GetCount() ) ;
      }
      break ;
    case vframe:
      {
        const CVideoFrame * pFr = pDataFrame->GetVideoFrame() ;
        if ( pFr && pFr->lpBMIH )
        {
          tmpS.Format( "\t[%dx%d]\r\n" , 
            pFr->lpBMIH->biWidth , pFr->lpBMIH->biHeight ) ;
        }
      }
      break ;
    default:
      tmpS = "\r\n";
      break;
    }
    outp += tmpS;
  }
}

void CTermWindow::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
  switch (nSBCode)
  {
  case SB_LINEDOWN:
  case SB_PAGEDOWN:
    m_nTopLine++;
    if (GetTopIndex() > GetLinesCount() - GetLinesInView())
      m_nTopLine--;
    //m_nSelLine++;
    m_nSelLine = GetBottomIndex();
    if (m_nSelLine >= GetLinesCount() - 1)
      AutoScroll();
    SetScrollPos(SB_VERT, GetTopIndex());
    break;
  case SB_LINEUP:
  case SB_PAGEUP:
    m_nTopLine--;
    if (GetTopIndex() < 0)
      m_nTopLine++;
    m_nSelLine = GetBottomIndex();
    SetScrollPos(SB_VERT, GetTopIndex());
    break;
  case SB_THUMBTRACK:
    m_nTopLine = nPos;
    if (GetTopIndex() > GetLinesCount() - GetLinesInView())
      m_nTopLine = GetLinesCount() - GetLinesInView();
    m_nSelLine = GetBottomIndex();
    if (m_nSelLine > GetLinesCount() - 1)
      AutoScroll();
    SetScrollPos(SB_VERT, GetTopIndex());
    break;
  }
  CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
  Invalidate();
}

void CTermWindow::OnSize(UINT nType, int cx, int cy)
{
  CWnd::OnSize(nType, cx, cy);
  UpdateVertBar();
  Invalidate();
}

void CTermWindow::OnPaint()
{
  CPaintDC dc(this); // device context for painting
  if (m_bClear)
  {
    m_WriteQueue.ClearQueue();
    m_CurLine.Empty();
    m_nSelLine = 0;
    m_bDrawPartial = FALSE;
    m_nTopLine = 0;
    m_bClear = false;
    UpdateVertBar();
  }
  int nFirst = GetTopIndex(), nLast = GetBottomIndex();
  int offset = 0;
  int height = GetLineHeight();
  CFont* oldFont = dc.SelectObject(&m_Font);
  COLORREF bkColor = dc.GetBkColor();
  COLORREF fgColor = dc.GetTextColor();
  while (nFirst <= nLast)
  {
    FXString line;
    m_WriteQueue.Peep(nFirst, line);
    if (nFirst == m_nSelLine)
    {
      dc.SetBkColor(TERMWINDOW_SELBGCOLOR);
      dc.SetTextColor(TERMWINDOW_SELFGCOLOR);
    }
    else
    {
      dc.SetBkColor(TERMWINDOW_DEFBGCOLOR);
      dc.SetTextColor(TERMWINDOW_DEFFGCOLOR);
    }
    dc.TextOut(5, offset, line, (int)line.GetLength());
    nFirst++;
    offset += height;
  }
  dc.SelectObject(oldFont);
  dc.SetBkColor(bkColor);
}

void CTermWindow::OnLButtonDown(UINT nFlags, CPoint point)
{
  SelectLine(IndexFromPt(point));
  CWnd::OnLButtonDown(nFlags, point);
}

void CTermWindow::OnTimer(UINT_PTR nIDEvent)
{
  if (m_Invalidate)
  {
    BOOL bAutoScrollOn = (m_nSelLine < GetLinesCount() - 1);
    UpdateVertBar();
    Invalidate();
    if (bAutoScrollOn || (m_nSelLine < GetTopIndex()))
      AutoScroll();
    m_Invalidate = false;
  }
  CWnd::OnTimer(nIDEvent);
}

BOOL CTermWindow::OnMouseWheel( UINT nFlags , short zDelta , CPoint pt )
{
  int iRotation = zDelta / 120 ;
  m_nTopLine -= iRotation * ((nFlags & MK_SHIFT) ? 5 : 1) ;
  if ( m_nTopLine < 0 )
    m_nTopLine = 0 ;
  int iHeight = GetLineHeight() ;
  int iNLines = GetLinesCount() ;
  if ( m_nTopLine > iNLines - iHeight + 2 )
    m_nTopLine = (iHeight >= iNLines) ? 0 : iNLines - iHeight + 2 ;

  SetScrollPos( SB_VERT , GetTopIndex() );
  Invalidate() ;

  return CWnd::OnMouseWheel( nFlags , zDelta , pt );
}
