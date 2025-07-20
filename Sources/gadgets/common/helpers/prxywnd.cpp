// PrxyWnd.cpp : implementation file
//

#include "stdafx.h"
#include "PrxyWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPrxyWnd

CPrxyWnd::CPrxyWnd():
    m_InnerWnd(NULL),
    m_Callback(NULL)
{
}

CPrxyWnd::~CPrxyWnd()
{
}


BEGIN_MESSAGE_MAP(CPrxyWnd, CStatic)
	//{{AFX_MSG_MAP(CPrxyWnd)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
    ON_WM_VSCROLL()
    ON_WM_HSCROLL()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrxyWnd message handlers

BOOL CPrxyWnd::Create(CWnd* pParentWnd)
{
	CRect rect(0, 0, 0, 0);
	if (pParentWnd)
		pParentWnd->GetClientRect(rect);
	return CStatic::Create("", WS_CHILD | WS_VISIBLE | SS_CENTER, rect, pParentWnd, AFX_IDW_PANE_FIRST);
}

BOOL CPrxyWnd::DestroyWindow() 
{
	return CStatic::DestroyWindow();
}

void CPrxyWnd::OnSize(UINT nType, int cx, int cy) 
{
	CStatic::OnSize(nType, cx, cy);
    if (m_InnerWnd->GetSafeHwnd())
        m_InnerWnd->MoveWindow(0,0,cx,cy);
}

BOOL CPrxyWnd::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
    //BN_CLICKEDx
    if (m_Callback)
        m_Callback(nID, nCode, m_UserParam,0);
	return CStatic::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CPrxyWnd::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    ScrollCmd sCmd={nSBCode,nPos,pScrollBar};
    if (m_Callback)
        m_Callback(pScrollBar->GetDlgCtrlID(), WM_VSCROLL, m_UserParam,&sCmd);
    CStatic::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CPrxyWnd::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    ScrollCmd sCmd={nSBCode,nPos,pScrollBar};
    if (m_Callback)
        m_Callback(pScrollBar->GetDlgCtrlID(), WM_HSCROLL, m_UserParam,&sCmd);
    CStatic::OnHScroll(nSBCode, nPos, pScrollBar);
}
