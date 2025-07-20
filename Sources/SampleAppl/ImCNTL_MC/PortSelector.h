#include "afxcmn.h"
#if !defined(AFX_PORTSELECTOR_H__92DFF6E8_69F7_415F_B1F6_FBA21B6BD85C__INCLUDED_)
#define AFX_PORTSELECTOR_H__92DFF6E8_69F7_415F_B1F6_FBA21B6BD85C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PortSelector.h : header file
//
#include "resource.h"
/////////////////////////////////////////////////////////////////////////////
// CPortSelector dialog
#define UM_INITPORT WM_USER+900
#define UM_INITDLL	WM_USER+810

class CPortSelector : public CDialog
{
// Construction
public:
	CPortSelector(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPortSelector)
	enum { IDD = IDD_PORT_SELECTOR };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPortSelector)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPortSelector)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	CString m_dllFileName;
	int	m_nPort;
	CListCtrl m_PortList;
	afx_msg void OnNMDblclkPortList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButton1();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PORTSELECTOR_H__92DFF6E8_69F7_415F_B1F6_FBA21B6BD85C__INCLUDED_)
