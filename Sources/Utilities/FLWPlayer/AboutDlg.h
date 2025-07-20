#if !defined(AFX_ABOUTDLG_H__AFCE7D31_B6D8_48AF_AE07_0B745D179D02__INCLUDED_)
#define AFX_ABOUTDLG_H__AFCE7D31_B6D8_48AF_AE07_0B745D179D02__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AboutDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// AboutDlg dialog

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ABOUTDLG_H__AFCE7D31_B6D8_48AF_AE07_0B745D179D02__INCLUDED_)
