#if !defined(AFX_HELPDIALOG_H__9D2D953E_2C6D_432B_9901_D2B13EF4286A__INCLUDED_)
#define AFX_HELPDIALOG_H__9D2D953E_2C6D_432B_9901_D2B13EF4286A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HelpDialog.h : header file
//
#include "HelpView.h"
/////////////////////////////////////////////////////////////////////////////
// CHelpDialog dialog

#define INDEX_ALIAS "@INDEX"

class CHelpDialog : public CResizebleDialog
{
protected:
	CRuntimeClass* m_pClass;
    CHelpView*     m_pView;
	HICON          m_hIcon;
    CMapStringToString m_Plugins;
    CMapStringToString m_PluginHelps;
public:
	CHelpDialog(CWnd* pParent = NULL);   // standard constructor
    virtual ~CHelpDialog();
    void Navigate( LPCTSTR URL ) { if (m_pView) m_pView->Navigate(URL); }
    bool MemoryNavigate(LPCTSTR httpdoc) { if (m_pView) return m_pView->MemoryNavigate(httpdoc); return false; }
    CString LookUpPlugin(LPCTSTR gadgetclass);
    CString LookUpHelp(LPCTSTR gadgetclass);
	//{{AFX_DATA(CHelpDialog)
	enum { IDD = IDD_HELP_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHelpDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
protected:
	// Generated message map functions
	//{{AFX_MSG(CHelpDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnBack();
	afx_msg void OnForward();
	afx_msg void OnIndex();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void PrepareDynamicPart();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HELPDIALOG_H__9D2D953E_2C6D_432B_9901_D2B13EF4286A__INCLUDED_)
