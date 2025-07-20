#if !defined(AFX_STATICFRAME_H__1AB7FD79_B700_4C67_99B6_6B9293A503DA__INCLUDED_)
#define AFX_STATICFRAME_H__1AB7FD79_B700_4C67_99B6_6B9293A503DA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// staticframe.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStaticFrame window
class CStaticFrame : public CStatic
{
// Construction
public:
	CStaticFrame();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStaticFrame)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CStaticFrame();

	// Generated message map functions
protected:
	//{{AFX_MSG(CStaticFrame)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    afx_msg LRESULT OnCheckStateChange(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STATICFRAME_H__1AB7FD79_B700_4C67_99B6_6B9293A503DA__INCLUDED_)
