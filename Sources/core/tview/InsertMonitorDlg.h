#if !defined(AFX_INSERTMONITORDLG_H__80898479_9CF1_477F_B112_514312098D9F__INCLUDED_)
#define AFX_INSERTMONITORDLG_H__80898479_9CF1_477F_B112_514312098D9F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// InsertMonitorDlg.h : header file
//

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CInsertMonitorDlg dialog

class CInsertMonitorDlg : public CDialog
{
protected:
    FXString m_GadgetUID;
// Construction
public:
	CInsertMonitorDlg(LPCTSTR GadgetUID, CWnd* pParent = NULL);   // standard constructor
	int GetMonitorName(CString& name);
	enum
	{
		IM_INPLACE,
		IM_NEWWINDOW,
		IM_EXISTING,
	}INSERT_METHOD;
// Dialog Data
	CStringArray m_Monitors;
	//{{AFX_DATA(CInsertMonitorDlg)
	enum { IDD = IDD_INSERT_MONITOR_DLG };
	int		m_InsertMethod;
	int		m_Selected;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CInsertMonitorDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CInsertMonitorDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_INSERTMONITORDLG_H__80898479_9CF1_477F_B112_514312098D9F__INCLUDED_)
