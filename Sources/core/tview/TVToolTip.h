#if !defined(AFX_TVTOOLTIP_H__F0639688_3C99_452C_AEBD_A61034866811__INCLUDED_)
#define AFX_TVTOOLTIP_H__F0639688_3C99_452C_AEBD_A61034866811__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TVToolTip.h : header file
//
#include "resource.h"

#define TIMETOSHOWTOOLTIP 1000

/////////////////////////////////////////////////////////////////////////////
// CTVToolTip dialog

class CTVToolTip : public CDialog
{
	CBrush	m_Brush;
    DWORD   m_StartTime;
    bool    m_Active;
    UINT_PTR    m_Timer;
// Construction
public:
	CTVToolTip(CWnd* pParent = NULL);   // standard constructor
// Dialog Data
	//{{AFX_DATA(CTVToolTip)
	enum { IDD = IDD_TVTOOLTIP };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA
	void Activate(LPCTSTR text, CPoint& pt);
	void Pop();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTVToolTip)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTVToolTip)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TVTOOLTIP_H__F0639688_3C99_452C_AEBD_A61034866811__INCLUDED_)
