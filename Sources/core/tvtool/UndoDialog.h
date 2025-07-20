#if !defined(AFX_UNDODIALOG_H__A003603B_5815_40AD_9985_50664411CD5B__INCLUDED_)
#define AFX_UNDODIALOG_H__A003603B_5815_40AD_9985_50664411CD5B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// UndoDialog.h : header file
//

#include <gadgets/tview.h>

/////////////////////////////////////////////////////////////////////////////
// CUndoDialog dialog

class CUndoDialog : public CDialog
{
	ISketchView* m_IView;
	CImageList m_UndoStates;
	int m_iCurrent;
// Construction
public:
	CUndoDialog(CWnd* pParent = NULL);   // standard constructor
	void SetView(ISketchView* IView) { m_IView = IView; };
// Dialog Data
	//{{AFX_DATA(CUndoDialog)
	enum { IDD = IDD_UNDO_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUndoDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void ReloadCommands();

	// Generated message map functions
	//{{AFX_MSG(CUndoDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnItemchangedUndoList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_UNDODIALOG_H__A003603B_5815_40AD_9985_50664411CD5B__INCLUDED_)
