#if !defined(AFX_SIMPLEENTERDLG_H__AE4F43CA_41F3_42EA_B16C_E90E03A7ABB8__INCLUDED_)
#define AFX_SIMPLEENTERDLG_H__AE4F43CA_41F3_42EA_B16C_E90E03A7ABB8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SimpleEnterDlg.h : header file
//

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CSimpleEnterDlg dialog

class CSimpleEnterDlg : public CDialog
{
// Construction
public:
	CSimpleEnterDlg(CWnd* pParent = NULL);   // standard constructor
	CString m_Title;

// Dialog Data
	//{{AFX_DATA(CSimpleEnterDlg)
	enum { IDD = IDD_SIMPLE_ENTER_DLG };
	CString	m_Enter;
	//}}AFX_DATA
	CRect m_rc;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSimpleEnterDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSimpleEnterDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SIMPLEENTERDLG_H__AE4F43CA_41F3_42EA_B16C_E90E03A7ABB8__INCLUDED_)
