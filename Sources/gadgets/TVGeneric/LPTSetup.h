#if !defined(AFX_LPTSETUP_H__CEF96AF4_6B07_4EC2_B1FC_80199C11792D__INCLUDED_)
#define AFX_LPTSETUP_H__CEF96AF4_6B07_4EC2_B1FC_80199C11792D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LPTSetup.h : header file
//
#include <Gadgets\gadbase.h>
#include <gadgets\stdsetup.h>

/////////////////////////////////////////////////////////////////////////////
// CLPTSetup dialog
class LPTBitRender;
class CLPTSetup : public CGadgetSetupDialog
{
// Construction
public:
	CLPTSetup(CGadget* pGadget, CWnd* pParent = NULL);
// Dialog Data
	//{{AFX_DATA(CLPTSetup)
	enum { IDD = IDD_LPTSETTUPDIALOG };
	CComboBox	m_LptCombo;
	//}}AFX_DATA
// Overrides
    virtual bool Show(CPoint point, LPCTSTR uid);
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLPTSetup)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
    void UploadParams();
protected:

	// Generated message map functions
	//{{AFX_MSG(CLPTSetup)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LPTSETUP_H__CEF96AF4_6B07_4EC2_B1FC_80199C11792D__INCLUDED_)
