#if !defined(AFX_FIMPROCPARAMDIALOG_H__21C1A171_2EAF_11D3_8501_00A0C9616FBC__INCLUDED_)
#define AFX_FIMPROCPARAMDIALOG_H__21C1A171_2EAF_11D3_8501_00A0C9616FBC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FImProcParamDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFImProcParamDialog dialog

class CFImProcParamDialog : public CDialog
{
// Construction
public:
	CFImProcParamDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFImProcParamDialog)
	enum { IDD = IDD_DIALOG1 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFImProcParamDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFImProcParamDialog)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FIMPROCPARAMDIALOG_H__21C1A171_2EAF_11D3_8501_00A0C9616FBC__INCLUDED_)
