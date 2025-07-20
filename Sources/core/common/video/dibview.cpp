//  $File : DIBView.cpp : implementation file
//  (C) Copyright The File X Ltd 2008. 
//
//  DIB View class...
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)


#include "stdafx.h"
#include <video\shvideo.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define BOUND(x,min,max)        ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

/////////////////////////////////////////////////////////////////////////////
// CDIBView

CDIBView::CDIBView(LPCTSTR name):
  m_ScrBarEnabled(0),
  CDIBViewBase(name),
  m_MoveX(-1),m_MoveY(-1)
{
}

BEGIN_MESSAGE_MAP(CDIBView, CDIBViewBase)
  //{{AFX_MSG_MAP(CDIBView)
  ON_WM_HSCROLL()
  ON_WM_VSCROLL()
  ON_WM_MOUSEWHEEL()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDIBView message handlers

BOOL CDIBView::Create(CWnd* pParentWnd, DWORD dwAddStyle, 
                      UINT nID , LPCTSTR szWindowName ) 
{
  BOOL res = CDIBViewBase::Create(pParentWnd, dwAddStyle,
    nID , szWindowName ); 
  if (res)
  {
    SetScrollRange(SB_HORZ,0,1000);
    SetScrollRange(SB_VERT,0,1000);
    ShowScrollBar(SB_HORZ, FALSE);
    ShowScrollBar(SB_VERT, FALSE);
  }
  return res;
}

void CDIBView::ShowScrollBar( UINT nBar, BOOL bShow ) 
{
  UINT mask=(nBar==SB_VERT)?FL_VERT:(nBar==SB_HORZ)?FL_HORZ:0;
  if (bShow) 
    m_ScrBarEnabled|=mask; 
  else 
    m_ScrBarEnabled&=~mask; 
  CDIBViewBase::ShowScrollBar(nBar, bShow);
}

bool CDIBView::DoScrolling(int& x, int& y, RECT& rc)
{
  UINT oldScrBarEnabled=m_ScrBarEnabled;
  ShowScrollBar(SB_HORZ, x<0);
  ShowScrollBar(SB_VERT, y<0);
  if (x<0) 
  {
    int max,min;
    GetScrollRange(SB_VERT,&min, &max);
    x=((rc.right-(int)(m_Scale* m_Width))*GetScrollPos(SB_HORZ))/(max-min);
  }
  if (y<0) 
  {
    int max,min;
    GetScrollRange(SB_VERT,&min, &max);
    y=((rc.bottom-(int)(m_Scale* m_Height))*GetScrollPos(SB_VERT))/(max-min);
  }
  m_ScrOffset=CPoint(x,y); 
  return (oldScrBarEnabled!=m_ScrBarEnabled);
}

bool CDIBView::Draw(HDC hdc, RECT& rc)
{
  ResetData() ;
  LPBITMAPINFOHEADER lpBMIH= GetDrawData();
  if (!lpBMIH) 
    return false; 

  ASSERT(GetDrawData()==lpBMIH);

  int x,y;
  bool res=false, redraw=m_SizeChanged; m_SizeChanged=false;
  int iNLines = 0 ;
  SetStretchBltMode(hdc,STRETCH_DELETESCANS);
//   if (m_Scale==1)
//   {
//     x=(rc.right - lpBMIH->biWidth)/2;
//     y=(rc.bottom - lpBMIH->biHeight)/2;
//     CRect clrc;
//     GetClientRect(clrc); 
//     CPoint rb((int)(clrc.right/m_ScrScale),(int)(clrc.bottom/m_ScrScale));
//     if (m_MoveX!=-1)
//     {
//       int scrXMin, scrXMax;
//       GetScrollRange(SB_HORZ,&scrXMin, &scrXMax);
//       if ((lpBMIH->biWidth-rb.x)!=0) // scroll bar exists
//       {
//         int newPos=scrXMin+((m_MoveX-rb.x/2)*(scrXMax-scrXMin))/(lpBMIH->biWidth-rb.x);
//         SetScrollPos(SB_HORZ,newPos);
//       }
//     }
//     if (m_MoveY!=-1)
//     {
//       int scrYMin, scrYMax;
//       GetScrollRange(SB_VERT,&scrYMin, &scrYMax);
//       if ((lpBMIH->biHeight-rb.y)!=0) // scroll bar exists
//       {
//         int newPos=scrYMin+((m_MoveY-rb.y/2)*(scrYMax-scrYMin))/(lpBMIH->biHeight-rb.y);
//         SetScrollPos(SB_VERT,newPos);
//       }
//     } 
//     redraw|=DoScrolling(x, y, clrc);
//     m_ScrScale=1.0;
//     iNLines = SetDIBitsToDevice(hdc,x,y,lpBMIH->biWidth,lpBMIH->biHeight,
//       0,0,0,lpBMIH->biHeight,
//       GetData(lpBMIH), (BITMAPINFO*)lpBMIH,DIB_RGB_COLORS
//       )!=0;
//   }
//   else
//   {
    if (m_Scale<0)
    {
      ASSERT(GetDrawData()==lpBMIH);
      double xscale=((double)rc.right)/lpBMIH->biWidth;
      double yscale=((double)rc.bottom)/lpBMIH->biHeight;
      m_ScrScale=(xscale<yscale)?xscale:yscale;
    }
    else
    {
      m_ScrScale=m_Scale;
    }
    x=(rc.right -  (int)(m_ScrScale*lpBMIH->biWidth))/2;
    y=(rc.bottom - (int)(m_ScrScale*lpBMIH->biHeight))/2;
    CRect rcl; // local ????
    GetClientRect(rcl); 
    CPoint rb((int)(rcl.right/m_ScrScale),(int)(rcl.bottom/m_ScrScale));
    if (m_Scale>0)
    {
      if (m_MoveX!=-1)
      {
        int scrXMin, scrXMax;
        GetScrollRange(SB_HORZ,&scrXMin, &scrXMax);
        if ((lpBMIH->biWidth-rb.x)!=0) // scroll bar exists
        {
          int newPos=scrXMin+((m_MoveX-rb.x/2)*(scrXMax-scrXMin))/(lpBMIH->biWidth-rb.x);
          SetScrollPos(SB_HORZ,newPos);
        }
      }
      if (m_MoveY!=-1)
      {
        int scrYMin, scrYMax;
        GetScrollRange(SB_VERT,&scrYMin, &scrYMax);
        if ((lpBMIH->biHeight-rb.y)!=0) // scroll bar exists
        {
          int newPos=scrYMin+((m_MoveY-rb.y/2)*(scrYMax-scrYMin))/(lpBMIH->biHeight-rb.y);
          SetScrollPos(SB_VERT,newPos);
        }
      } 
    }
    redraw|=DoScrolling(x, y, rcl);
    ASSERT(GetDrawData()==lpBMIH);
    iNLines = StretchDIBits(
      hdc,x,y,(int)(m_ScrScale*lpBMIH->biWidth),(int)(m_ScrScale*lpBMIH->biHeight),
      0,0,lpBMIH->biWidth,lpBMIH->biHeight,
      GetData(lpBMIH), (BITMAPINFO*)lpBMIH,
      DIB_RGB_COLORS,SRCCOPY
      ) ;
    redraw = false ; //we did redraw
//   }
  if (redraw)
  {
    Invalidate();
    m_SizeChanged=false;
  }
  return ((iNLines != 0) && (iNLines != GDI_ERROR));
}

void CDIBView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
  int     iMax;
  int     iMin;
  int     iPos;
  int     dn;
  RECT    rc;

  GetScrollRange(SB_HORZ, &iMin, &iMax);
  iPos = GetScrollPos(SB_HORZ);
  GetClientRect(&rc);
  switch (nSBCode) 
  {
  case SB_LINEDOWN:
    dn =  1;
    break;
  case SB_LINEUP:
    dn = -1;
    break;
  case SB_PAGEDOWN:
    dn =  rc.right / 4  + 1;
    break;
  case SB_PAGEUP:
    dn = -rc.right / 4  + 1;
    break;
  case SB_THUMBTRACK:
  case SB_THUMBPOSITION:
    dn = nPos - iPos;
    break;
  default:
    dn = 0;
    break;
  }
  UpdateWindow();
  if (dn = BOUND(iPos + dn, iMin, iMax) - iPos) 
  {
    CDIBViewBase::ScrollWindow(-dn, 0, NULL, NULL);
    SetScrollPos(SB_HORZ, iPos + dn, TRUE);
    UpdateWindow();
    Invalidate(FALSE);
  }
  CDIBViewBase::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CDIBView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
  int     iMax;
  int     iMin;
  int     iPos;
  int     dn;
  RECT    rc;

  GetScrollRange(SB_VERT, &iMin, &iMax);
  iPos = GetScrollPos(SB_VERT);
  GetClientRect(&rc);
  switch (nSBCode) 
  {
  case SB_LINEDOWN:
    dn =  1;
    break;
  case SB_LINEUP:
    dn = -1;
    break;
  case SB_PAGEDOWN:
    dn =  rc.bottom / 4  + 1;
    break;
  case SB_PAGEUP:
    dn = -rc.bottom / 4  + 1;
    break;
  case SB_THUMBTRACK:
  case SB_THUMBPOSITION:
    dn = nPos - iPos;
    break;
  default:
    dn = 0;
    break;
  }
  UpdateWindow();
  if (dn = BOUND(iPos + dn, iMin, iMax) - iPos) 
  {
    CDIBViewBase::ScrollWindow(0, -dn, NULL, NULL);
    SetScrollPos(SB_VERT, iPos + dn, TRUE);
    UpdateWindow();
    Invalidate(FALSE);
  }
  CDIBViewBase::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CDIBView::ShiftPos(int dx, int dy)
{
  m_MoveX=dx; m_MoveY=dy;
}

BOOL CDIBView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
  if (nFlags & MK_MBUTTON)
  {
    if (m_ScrBarEnabled&FL_HORZ) OnHScroll(SB_THUMBTRACK,GetScrollPos(SB_HORZ)-4*zDelta/WHEEL_DELTA,NULL);
  }
  else
  {
    if (m_ScrBarEnabled&FL_VERT) OnVScroll(SB_THUMBTRACK,GetScrollPos(SB_VERT)-4*zDelta/WHEEL_DELTA,NULL);
  }
  return CDIBViewBase::OnMouseWheel(nFlags, zDelta, pt);
}

void CDIBView::SetScale(double s)  
{
  if (s<0)
    m_Scale=-1;
  else if (s<=1)
    m_Scale=1;
  else if (s<=2)
    m_Scale=2;
  else if (s<=4)
    m_Scale=4;
  else if (s<=8)
    m_Scale=8;
  else 
    m_Scale=16;
  Invalidate();
}
