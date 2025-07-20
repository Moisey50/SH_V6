#pragma once

#include <gadgets\tview.h>
#include <gadgets\shkernel.h>
#include "resource.h"
#include "afxcmn.h"

typedef CResizebleDialog CResizableDialog;

// CGraphViewDlg dialog

class CGraphViewDlg : public CResizableDialog
{
	DECLARE_DYNAMIC(CGraphViewDlg)

	typedef struct _CDebugDocTemplate
	{
		IGraphbuilder* pBuilder;
		ISketchView* pView;
	}CDebugDocTemplate;

	typedef CArray<CDebugDocTemplate, CDebugDocTemplate&> CDebugTemplates;

	CDebugTemplates m_DebugTemplates;

	BOOL			m_bOLEInitialized;
public:
	CGraphViewDlg(CWnd* pParent, IGraphbuilder* Builder);   // non-standard constructor
	virtual ~CGraphViewDlg();
	void OpenView(IGraphbuilder* pBuilder, LPCTSTR title = _T(""));
	void RegisterActiveDragDrop();

// Dialog Data
	enum { IDD = IDD_GRAPH_VIEW_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	void RepositionView(ISketchView* View);
	ISketchView* GetActiveView();
	IGraphbuilder* GetActiveBuilder();

	DECLARE_MESSAGE_MAP()
	virtual void OnCancel();
public:
	afx_msg void OnDestroy();
protected:
	virtual void PostNcDestroy();
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CTabCtrl m_Tabs;
	afx_msg void OnBnClickedRun();
	afx_msg void OnBnClickedStop();
	afx_msg LRESULT OnCreateNewView(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTcnSelchangeTabview(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTcnSelchangingTabview(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedCloseTab();
  afx_msg void OnSaveCurrentGraph();
};
