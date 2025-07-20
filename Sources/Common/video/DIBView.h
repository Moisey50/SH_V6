//  $File : DIBView.h : header file
//  (C) Copyright The File X Ltd 2002. 
//
//  DIB View class...
//
//  Revision History (excluding minor changes for specific compilers)
//   13 May 02 Firts release version, all followed changes must be listed below  (Andrey Chernushich)

#if !defined(AFX_DIBVIEW_H__7F1414D0_8027_11D5_9463_F04F70C1402B__INCLUDED_)
#define AFX_DIBVIEW_H__7F1414D0_8027_11D5_9463_F04F70C1402B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#ifdef __INTEL_COMPILER
#pragma warning(disable: 1125)
#endif

// DIBView.h : header file
//

#include <video\DIBViewBase.h>

#define FL_HORZ 1
#define FL_VERT 2

/////////////////////////////////////////////////////////////////////////////
// CDIBView window
class FX_EXT_SHVIDEO CDIBView : public CDIBViewBase
{
protected:
    UINT m_ScrBarEnabled;
    int  m_MoveX,m_MoveY;
public:
    CDIBView(LPCTSTR name=_T(""));
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDIBView)
	public:
    virtual BOOL Create(CWnd* pParentWnd, DWORD dwAddStyle=0,
      UINT nID=0 , LPCTSTR szWindowName = _T("DIBView"));
	//}}AFX_VIRTUAL
// Implementation
public:
    bool Draw(HDC hdc,RECT& rc);
    void ShowScrollBar( UINT nBar, BOOL bShow = TRUE );
    void SetScale(double s);
    bool DoScrolling(int& x, int& y, RECT& rc);
    void ShiftPos(int dx, int dy);
protected:
	//{{AFX_MSG(CDIBView)
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIBVIEW_H__7F1414D0_8027_11D5_9463_F04F70C1402B__INCLUDED_)
