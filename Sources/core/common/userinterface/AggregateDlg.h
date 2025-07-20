#if !defined(AFX_AGGREGATEDLG_H__10A8429F_B1BA_4870_BBF4_82AD263D9C3D__INCLUDED_)
#define AFX_AGGREGATEDLG_H__10A8429F_B1BA_4870_BBF4_82AD263D9C3D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AggregateDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAggregateDlg dialog

class CAggregateDlg : public CDialog
{
// Construction
public:
	CAggregateDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CAggregateDlg)
	enum { IDD = IDD_AGGREGATE_DLG };
	CString	m_BlockName;
	BOOL	m_bAddToLibrary;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAggregateDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAggregateDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AGGREGATEDLG_H__10A8429F_B1BA_4870_BBF4_82AD263D9C3D__INCLUDED_)
