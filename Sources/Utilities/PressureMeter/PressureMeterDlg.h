// PressureMeterDlg.h : header file
//

#if !defined(AFX_PRESSUREMETERDLG_H__3FEA675C_A930_4AFE_B780_ED5B3E7196B6__INCLUDED_)
#define AFX_PRESSUREMETERDLG_H__3FEA675C_A930_4AFE_B780_ED5B3E7196B6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CPressureMeterDlg dialog
#include "trayicon.h"
#include <gadgets\shkernel.h>

class CPressureMeterDlg : public CDialog
{
friend BOOL CALLBACK CPressureMeterDlg_OutputCallback(CDataFrame*& lpData, FXString& idPin, void* lpParam);
private:
	IGraphbuilder *m_pBuilder;
	IPluginLoader *m_PluginLoader;
	CString		   m_TVGFileName;
	FXLockObject	   m_Lock;
	double		   m_data;
	int			   m_count;
	UINT_PTR		   m_Timer;
protected:
	BOOL OnData(CDataFrame* lpData);
public:
	CPressureMeterDlg(CWnd* pParent = NULL);	// standard constructor
	~CPressureMeterDlg();
// Dialog Data
	//{{AFX_DATA(CPressureMeterDlg)
	enum { IDD = IDD_PRESSUREMETER_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPressureMeterDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
	CTrayIcon	*m_trayIcon; 
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CPressureMeterDlg)
	virtual BOOL OnInitDialog();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg LRESULT OnTrayNotification(WPARAM wp, LPARAM lp);
    afx_msg void OnTrayRestore();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnSetup();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PRESSUREMETERDLG_H__3FEA675C_A930_4AFE_B780_ED5B3E7196B6__INCLUDED_)
