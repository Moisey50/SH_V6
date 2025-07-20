#if !defined(AFX_RIGHTBUTTONDLG_H__878B6D01_F872_11D6_95D4_00A024775726__INCLUDED_)
#define AFX_RIGHTBUTTONDLG_H__878B6D01_F872_11D6_95D4_00A024775726__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RightButtonDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRightButtonDlg dialog

class CRightButtonDlg : public CDialog
{
// Construction
public:
	CRightButtonDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRightButtonDlg)
	enum { IDD = _UNKNOWN_RESOURCE_ID_ };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRightButtonDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRightButtonDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RIGHTBUTTONDLG_H__878B6D01_F872_11D6_95D4_00A024775726__INCLUDED_)
