#if !defined(AFX_COMPLEXGADGETDLG_H__E189895F_35DC_4DEB_9C74_756EF5EF1A4A__INCLUDED_)
#define AFX_COMPLEXGADGETDLG_H__E189895F_35DC_4DEB_9C74_756EF5EF1A4A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ComplexGadgetDlg.h : header file
//

#include <gadgets\stdsetup.h>
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// ComplexDlg dialog

class ComplexDlg : public CGadgetSetupDialog
{
	enum { CMD_NONE, CMD_LOAD, CMD_SAVE };
	UINT m_Cmd;
// Construction
public:
	ComplexDlg(IGraphbuilder* pBuilder, FXString& uid, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(ComplexDlg)
	enum { IDD = IDD_COMPLEX_GADGET_SETUP_DLG };
	CString	m_LoadPath;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ComplexDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual void UploadParams(CGadget* Gadget);

	// Generated message map functions
	//{{AFX_MSG(ComplexDlg)
	afx_msg void OnBrowse();
	afx_msg void OnSave();
	afx_msg void OnLoad();
	afx_msg void OnIncorporate();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMPLEXGADGETDLG_H__E189895F_35DC_4DEB_9C74_756EF5EF1A4A__INCLUDED_)
