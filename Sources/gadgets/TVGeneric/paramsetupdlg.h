#if !defined(AFX_PARAMSETUPDLG_H__5521A272_48FA_4985_8595_90F3F6C73772__INCLUDED_)
#define AFX_PARAMSETUPDLG_H__5521A272_48FA_4985_8595_90F3F6C73772__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// paramsetupdlg.h : header file
//

#include <gadgets\gadbase.h>
#include <gadgets\stdsetup.h>

/////////////////////////////////////////////////////////////////////////////
// CParamSetupDlg dialog

class CParamSetupDlg : public CGadgetSetupDialog
{
// Construction
	CGadget* m_Gadget;
    FXString m_Key;
public:
	CParamSetupDlg(CGadget* Gadget, LPCTSTR key="", CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CParamSetupDlg)
	enum { IDD = IDD_SETUP_DLG };
	CString	m_Params;
	//}}AFX_DATA
// Overrides
    virtual bool Show(CPoint point, LPCTSTR uid);
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CParamSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void UploadParams();
	// Generated message map functions
	//{{AFX_MSG(CParamSetupDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PARAMSETUPDLG_H__5521A272_48FA_4985_8595_90F3F6C73772__INCLUDED_)
