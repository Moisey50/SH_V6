#if !defined(AFX_SELECTGADGETCLASSNEWDLG_H__7BA2054B_3E9F_466F_8ACA_0C267FA19EE2__INCLUDED_)
#define AFX_SELECTGADGETCLASSNEWDLG_H__7BA2054B_3E9F_466F_8ACA_0C267FA19EE2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SelectGadgetClassDlg.h : header file
//

#include "GadgetsTree.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CSelectGadgetClassDlg dialog

class CSelectGadgetClassDlg : public CDialog
{
	CGadgetsTree	m_GadgetsTree;
	CStringArray*	m_pGadgetClasses;
	CStringArray*	m_pGadgetLineages;
	CUIntArray*		m_pGadgetTypes;
// Construction
public:
	CSelectGadgetClassDlg(CWnd* pParent = NULL);   // standard constructor
	void SetGadgetsInfo(CUIntArray* Types, CStringArray* Classes, CStringArray* Lineages);

// Dialog Data
	CString m_GadgetClass;
	//{{AFX_DATA(CSelectGadgetClassDlg)
	enum { IDD = IDD_SELECT_GADGET_CLASS_NEW };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectGadgetClassDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void BuildTree();
	// Generated message map functions
	//{{AFX_MSG(CSelectGadgetClassDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnGadgetsTreeSelChanged(NMHDR* pNotifyStruct, LRESULT* result);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SELECTGADGETCLASSNEWDLG_H__7BA2054B_3E9F_466F_8ACA_0C267FA19EE2__INCLUDED_)
