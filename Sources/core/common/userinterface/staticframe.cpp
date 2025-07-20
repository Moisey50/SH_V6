// staticframe.cpp : implementation file
//

#include "stdafx.h"
#include "staticframe.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStaticFrame

CStaticFrame::CStaticFrame()
{
}

CStaticFrame::~CStaticFrame()
{
}


BEGIN_MESSAGE_MAP(CStaticFrame, CStatic)
	//{{AFX_MSG_MAP(CStaticFrame)
	ON_WM_SIZE()
	ON_WM_SHOWWINDOW()
	ON_WM_RBUTTONUP()
    ON_MESSAGE(UM_CHECKSTATECHANGE, OnCheckStateChange) 
	//}}AFX_MSG_MAP
    ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStaticFrame message handlers

void CStaticFrame::OnSize(UINT nType, int cx, int cy) 
{
	CStatic::OnSize(nType, cx, cy);
    CRect rc; GetClientRect(rc);	

	if (GetTopWindow( ))
    {
        GetTopWindow( )->MoveWindow(rc);
        GetTopWindow( )->Invalidate();
    }
}

void CStaticFrame::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CStatic::OnShowWindow(bShow, nStatus);
    if (GetTopWindow( ))
    {
        GetTopWindow( )->ShowWindow((bShow)?SW_SHOW:SW_HIDE);
    }
}

void CStaticFrame::OnRButtonUp(UINT nFlags, CPoint point) 
{
    GetParent()->SendMessage(WM_RBUTTONUP,(WPARAM)this->m_hWnd,MAKELPARAM(point.x,point.y));	
	CStatic::OnRButtonUp(nFlags, point);
}

LRESULT CStaticFrame::OnCheckStateChange(WPARAM wParam, LPARAM lParam)
{
    return GetParent()->SendMessage(UM_CHECKSTATECHANGE, wParam, lParam);
}

void CStaticFrame::OnMouseMove(UINT nFlags, CPoint point)
{
    GetParent()->SendMessage(WM_MOUSEMOVE,(WPARAM)this->m_hWnd,MAKELPARAM(point.x,point.y));	
    CStatic::OnMouseMove(nFlags, point);
}
