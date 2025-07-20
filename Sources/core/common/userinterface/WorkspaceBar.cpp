// WorkspaceBar.cpp : implementation file
//

#include "stdafx.h"
#include "WorkspaceBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define WB_MINIMUM_WIDTH	100
#define WB_DEFAULT_WIDTH	200
#define WB_MARGIN_WIDTH		4

/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar

CWorkspaceBar::CWorkspaceBar():
m_pSite(NULL),
m_nWidth(WB_DEFAULT_WIDTH),
m_nHeight(WB_DEFAULT_WIDTH),
m_hCursor(NULL),
m_dragType(HTNOWHERE),
m_dragPt(CPoint(0, 0)),
m_nID(0xFFFF),
m_bInRecalcingLayout(FALSE)
{
#ifdef TVDB_WORKSPACE_AS_DOCKBAR
	m_bAutoDelete = FALSE;
#endif
}

CWorkspaceBar::~CWorkspaceBar()
{
}


BEGIN_MESSAGE_MAP(CWorkspaceBar, WRKSPC_BASE)
	//{{AFX_MSG_MAP(CWorkspaceBar)
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CWorkspaceBar::Create(CWnd* pParent, LPCTSTR title, DWORD dwStyle, UINT uID)
{
	if (!pParent || !::IsWindow(pParent->GetSafeHwnd()))
		return FALSE;
	CRect rc(0, 0, 0, 0);
#if defined TVDB_WORKSPACE_AS_CONTROLBAR
	if (!CControlBar::Create(TOOLBARCLASSNAME, "", dwStyle, rc, pParent, uID))
#elif defined TVDB_WORKSPACE_AS_REBAR
	if (!CReBar::Create(pParent, RBS_BANDBORDERS, dwStyle, uID))
#else
	if (!WRKSPC_BASE::Create(pParent, dwStyle, uID))
#endif
		return FALSE;
#ifdef TVDB_WORKSPACE_AS_DOCKBAR
	SetBorders(WB_MARGIN_WIDTH, WB_MARGIN_WIDTH, WB_MARGIN_WIDTH, WB_MARGIN_WIDTH);
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
#endif
	m_nID = uID;
	if (dwStyle & (CBRS_LEFT | CBRS_RIGHT))
	{
		EnableDocking(CBRS_ALIGN_RIGHT | CBRS_ALIGN_LEFT);
	}
	if (dwStyle & (CBRS_TOP | CBRS_BOTTOM))
	{
		EnableDocking(CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM);
	}
	SetWindowText(title);
	return TRUE;
}

LRESULT CWorkspaceBar::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_NOTIFY)
	{
		NMHDR* pNMHDR = (NMHDR*)lParam;
		if (pNMHDR->code == -407)
		{
			TRACE("WM_NOTIFY! nCode = %d\n", pNMHDR->code);
		}
	}
	return WRKSPC_BASE::WindowProc(message, wParam, lParam);
}

CSize CWorkspaceBar::CalcDynamicLayout(int nLength, DWORD dwMode)
{
	if (IsFloating())
    {
//        TRACE("+++ cdl: floating 0x%x -> %d\n", dwMode,nLength);
		return CSize(m_nWidth, m_nHeight);
//        return CSize((dwMode & LM_HORZ) ? 32767 : m_nWidth,
//                      (dwMode & LM_HORZ) ? m_nHeight : 32767);

    }
	if (m_bInRecalcingLayout)
		return CSize(0, 0);
	CFrameWnd* pFrame = GetDockingFrame();
	ASSERT(pFrame && ::IsWindow(pFrame->GetSafeHwnd()));
	CRect rcFrame;
	m_bInRecalcingLayout = TRUE;
	pFrame->RepositionBars(AFX_IDW_CONTROLBAR_FIRST, m_nID - 1, AFX_IDW_PANE_FIRST, CWnd::reposQuery, rcFrame);
	m_bInRecalcingLayout = FALSE;
	DWORD dwStyle = GetBarStyle();
	if (dwStyle & (CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT))
		return CSize(m_nWidth, rcFrame.Height());
	else if (dwStyle & (CBRS_ALIGN_TOP | CBRS_ALIGN_BOTTOM))
    {
        m_nWidth=rcFrame.Width();
		return CSize(rcFrame.Width(), m_nHeight);
    }
	return CSize(m_nWidth, m_nHeight);
}


/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar helpers

int CWorkspaceBar::HitTest(CPoint& ptCursor)
{
	CRect rcWnd;
	GetWindowRect(rcWnd);
	if (!rcWnd.PtInRect(ptCursor))
		return HTNOWHERE;
	DWORD dwStyle = GetBarStyle();
	if (IsFloating() || (dwStyle & CBRS_ALIGN_RIGHT))
	{
		rcWnd.DeflateRect(WB_MARGIN_WIDTH, 0, 0, 0);
		if (!rcWnd.PtInRect(ptCursor))
			return HTLEFT;
	}
	if (IsFloating() || (dwStyle & CBRS_ALIGN_LEFT))
	{
		rcWnd.DeflateRect(0, 0, WB_MARGIN_WIDTH, 0);
		if (!rcWnd.PtInRect(ptCursor))
			return HTRIGHT;
	}
	if (IsFloating() || (dwStyle & CBRS_ALIGN_TOP))
	{
		rcWnd.DeflateRect(0, 0, 0, WB_MARGIN_WIDTH);
		if (!rcWnd.PtInRect(ptCursor))
			return HTBOTTOM;
	}
	if (IsFloating() || (dwStyle & CBRS_ALIGN_BOTTOM))
	{
		rcWnd.DeflateRect(0, WB_MARGIN_WIDTH, 0, 0);
		if (!rcWnd.PtInRect(ptCursor))
			return HTTOP;
	}
	return HTNOWHERE;
}

void CWorkspaceBar::DragTo(CPoint& pt)
{
	switch (m_dragType)
	{
	case HTRIGHT:
		m_nWidth += pt.x - m_dragPt.x;
		break;
	case HTLEFT:
		if (!IsFloating())
			m_nWidth -= pt.x - m_dragPt.x;
		break;
	case HTTOP:
		if (!IsFloating())
			m_nHeight -= pt.y - m_dragPt.y;
		break;
	case HTBOTTOM:
		m_nHeight += pt.y - m_dragPt.y;
		break;
	}
	CFrameWnd* pFrameWnd = GetDockingFrame();
	ASSERT_VALID(pFrameWnd);
	pFrameWnd->RecalcLayout();
}

/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar message handlers


BOOL CWorkspaceBar::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	CPoint pt;
	GetCursorPos(&pt);
	HCURSOR hCursor;
	switch (HitTest(pt))
	{
	case HTNOWHERE:
		hCursor = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
		break;
	case HTLEFT:
	case HTRIGHT:
		hCursor = AfxGetApp()->LoadStandardCursor(IDC_SIZEWE);
		break;
	case HTTOP:
	case HTBOTTOM:
		hCursor = AfxGetApp()->LoadStandardCursor(IDC_SIZENS);
		break;
	}
	if (m_hCursor != hCursor)
	{
		m_hCursor = hCursor;
		if (!SetClassLongPtr(GetSafeHwnd(), GCLP_HCURSOR, (LONG_PTR)m_hCursor))
		{
			TRACE("SetClassLong fails with error %d\n", GetLastError());
		}
	}
	return WRKSPC_BASE::OnSetCursor(pWnd, nHitTest, message);
}

void CWorkspaceBar::OnLButtonDown(UINT nFlags, CPoint point) 
{
	CPoint pt;
	GetCursorPos(&pt);
	m_dragType = HitTest(pt);
	if (m_dragType != HTNOWHERE)
	{
		SetCapture();
		m_dragPt = pt;
	}
	else
		WRKSPC_BASE::OnLButtonDown(nFlags, point);
}

void CWorkspaceBar::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CPoint pt;
	GetCursorPos(&pt);
	if (m_dragType != HTNOWHERE)
	{
		m_dragType = HTNOWHERE;
		ReleaseCapture();
		DragTo(pt);
	}
	else
		WRKSPC_BASE::OnLButtonUp(nFlags, point);
}

void CWorkspaceBar::OnMouseMove(UINT nFlags, CPoint point) 
{
	CPoint pt;
	GetCursorPos(&pt);
	if (m_dragType != HTNOWHERE)
	{
		DragTo(pt);
		m_dragPt = pt;
	}
	else
		WRKSPC_BASE::OnMouseMove(nFlags, point);
}

void CWorkspaceBar::OnSize(UINT nType, int cx, int cy) 
{
	WRKSPC_BASE::OnSize(nType, cx, cy);
	if (m_pSite && ::IsWindow(m_pSite->GetSafeHwnd()))
	{
		CRect rcClient;
		GetClientRect(rcClient);
		m_pSite->MoveWindow(rcClient);
	}
}
