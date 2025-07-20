#if !defined(AFX_GADGETSTREEDLG_H__60206392_FCAB_4A94_8471_7B63663EEDA7__INCLUDED_)
#define AFX_GADGETSTREEDLG_H__60206392_FCAB_4A94_8471_7B63663EEDA7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GadgetsTreeDlg.h : header file
//

#include "GadgetsTree.h"
#include "resource.h"
#include <gadgets\tview.h>

/////////////////////////////////////////////////////////////////////////////
// CGadgetsTreeDlg dialog

class CGadgetsTreeDlg : public CDialog
{
	BOOL			m_bUserLibrary;
	CImageList*		m_pDragItem;
	HTREEITEM		m_hDragged;
	ISketchView*	m_IView;
// Construction
public:
	CGadgetsTreeDlg(CWnd* pParent = NULL, BOOL bUserLibrary = FALSE);   // standard constructor
	void SetView(ISketchView* IView) { m_IView = IView; };

// Dialog Data
	//{{AFX_DATA(CGadgetsTreeDlg)
	enum { IDD = IDD_GADGETS_TREE_DLG };
	CGadgetsTree	m_GadgetsTree;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGadgetsTreeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	virtual void OnCancel();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGadgetsTreeDlg)
	afx_msg void OnBegindragGadgetsTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangedGadgetsTree(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL OnInitDialog();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GADGETSTREEDLG_H__60206392_FCAB_4A94_8471_7B63663EEDA7__INCLUDED_)
