// ImCNTLDlg.h : header file
//

#if !defined(AFX_IMCNTLDLG_H__A7F453BC_3255_11D3_8D4E_000000000000__INCLUDED_)
#define AFX_IMCNTLDLG_H__A7F453BC_3255_11D3_8D4E_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define SCAN_TIME 250.00 // micro seconds for Carmel
//#define SCAN_TIME 307.25  // micro seconds for Jericho and US
#define WM_MY_MSG WM_USER+6

class CImCNTLDlgAutoProxy ;
class CImageView ;


/////////////////////////////////////////////////////////////////////////////
// CImCNTLDlg dialog

class CImCNTLDlg : public CDialog
{
	DECLARE_DYNAMIC(CImCNTLDlg);
	friend class CImCNTLDlgAutoProxy;

// Construction
public:
	CString * m_pstrMsg;
	LRESULT OnRecMessage(WPARAM wpstr, LPARAM par);
	int GetSubBackgroundMode();
	CImageView * m_ViewFrame;
	CImCNTLDlg(CWnd* pParent = NULL);	// standard constructor
	virtual ~CImCNTLDlg();
	BOOL CanExit();

// Dialog Data
	//{{AFX_DATA(CImCNTLDlg)
	enum { IDD = IDD_IMCNTL_DIALOG };
	CString	m_Version;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImCNTLDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CImCNTLDlgAutoProxy* m_pAutoProxy;
	HICON m_hIcon;


	// Generated message map functions
	//{{AFX_MSG(CImCNTLDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IMCNTLDLG_H__A7F453BC_3255_11D3_8D4E_000000000000__INCLUDED_)
