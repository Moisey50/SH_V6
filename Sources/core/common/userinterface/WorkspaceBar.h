#if !defined(AFX_WORKSPACEBAR_H__2723FD8C_4BE9_42B1_95BD_F826537B63F8__INCLUDED_)
#define AFX_WORKSPACEBAR_H__2723FD8C_4BE9_42B1_95BD_F826537B63F8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WorkspaceBar.h : header file
//

#define TVDB_WORKSPACE_AS_TOOLBAR
//#define TVDB_WORKSPACE_AS_CONTROLBAR
//#define TVDB_WORKSPACE_AS_DOCKBAR
//#define TVDB_WORKSPACE_AS_REBAR

#if defined TVDB_WORKSPACE_AS_TOOLBAR
 #define WRKSPC_BASE	CToolBar
#elif defined TVDB_WORKSPACE_AS_CONTROLBAR
 #define WRKSPC_BASE	CControlBar
#elif defined TVDB_WORKSPACE_AS_DOCKBAR
 #include <afxpriv.h>
 #define WRKSPC_BASE	CDockBar
#elif defined TVDB_WORKSPACE_AS_REBAR
 #define WRKSPC_BASE	CReBar
#endif

#ifndef WRKSPC_BASE
 #error Base class for Tvdb400 workspace not defined
#endif

#ifndef TVDB_WORKSPACE_BAR_ID_FIRST
# define TVDB_WORKSPACE_BAR_ID_FIRST	(AFX_IDW_DIALOGBAR + 1)
#endif

/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar window

class CWorkspaceBar : public WRKSPC_BASE
{
	HCURSOR	m_hCursor;
	CPoint	m_dragPt;
	int		m_dragType;
	UINT	m_nID;
	BOOL	m_bInRecalcingLayout;
    int     m_nWidth, m_nHeight;
// Construction
public:
	CWorkspaceBar();

	CWnd*	m_pSite;

// Attributes
public:
	virtual CSize CalcDynamicLayout(int nLength, DWORD dwMode);
// Operations
public:
	BOOL Create(CWnd* pParent, LPCTSTR title = _T(""), DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_LEFT
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, UINT uID = TVDB_WORKSPACE_BAR_ID_FIRST);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWorkspaceBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CWorkspaceBar();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
#ifndef TVDB_WORKSPACE_AS_CONTROLBAR
	virtual void OnUpdateCmdUI( CFrameWnd* pTarget, BOOL bDisableIfNoHndler ) { };
#endif
#ifdef TVDB_WORKSPACE_AS_REBAR
	void EnableDocking(DWORD) {};
#endif
	// Generated message map functions
protected:
	int HitTest(CPoint& ptCursor);
	void DragTo(CPoint& pt);

	//{{AFX_MSG(CWorkspaceBar)
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WORKSPACEBAR_H__2723FD8C_4BE9_42B1_95BD_F826537B63F8__INCLUDED_)
