#if !defined(AFX_CTRLPINTESTDLG_H__9E5B6C44_50E7_455C_B8B0_DA2B05078979__INCLUDED_)
#define AFX_CTRLPINTESTDLG_H__9E5B6C44_50E7_455C_B8B0_DA2B05078979__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CtrlPinTestDlg.h : header file
//
#include "resource.h"
#include <Gadgets\gadbase.h>
#include <helpers\PrxyWnd.h>

/////////////////////////////////////////////////////////////////////////////
// CCtrlPinTestDlg dialog
class ControlPinTester;
class CCtrlPinTestDlg : public CDialog
{
protected:
    PrxyWndCallback m_Callback;
    void*           m_UserParam;
public:
	CCtrlPinTestDlg(CWnd* pParent = NULL);   // standard constructor
    void Init(PrxyWndCallback pwc, void *pUserParam) { m_Callback=pwc; m_UserParam=pUserParam; }
    void SetText(LPCTSTR str);
// Dialog Data
	//{{AFX_DATA(CCtrlPinTestDlg)
	enum { IDD = IDD_CTRLPIN_TESTDLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCtrlPinTestDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCtrlPinTestDlg)
	afx_msg void OnSend();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CTRLPINTESTDLG_H__9E5B6C44_50E7_455C_B8B0_DA2B05078979__INCLUDED_)
