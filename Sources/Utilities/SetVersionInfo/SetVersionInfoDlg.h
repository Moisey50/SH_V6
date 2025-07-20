// SetVersionInfoDlg.h : header file
//

#include "afxwin.h"
#if !defined(AFX_SETVERSIONINFODLG_H__D1F929D5_5CB5_42A0_87D8_010003992A62__INCLUDED_)
#define AFX_SETVERSIONINFODLG_H__D1F929D5_5CB5_42A0_87D8_010003992A62__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CSetVersionInfoDlg dialog

#define COMPANYNAME_DONE	0x01
#define FILEVERSION_DONE	0x02
#define PRODUCTVERSION_DONE	0x04
#define COMMENTS_DONE		0x08
#define LEGALTRADEMARK_DONE	0x10
#define LEGALCOPYRIGHT_DONE	0x20
#define PRIVATEBUILD_DONE	0x40
#define SPECIALBUILD_DONE	0x80


class CSetVersionInfoDlg : public CDialog
{
private:
	DWORD m_DoneBM;
	DWORD m_FieldChanged;
	CString m_ParentDir;
public:
	CSetVersionInfoDlg(CWnd* pParent = NULL);	// standard constructor
    void AddResult(LPCTSTR res);
	void SetFileName(LPCTSTR name);
	bool ProcessVersionInfoBlock(CFile& fl, char*& pntr, char* eod , CString * pResult = NULL );
	void ProcessFile(const char* path);
	bool SeekFiles(const char* path);
	

// Dialog Data
	//{{AFX_DATA(CSetVersionInfoDlg)
	enum { IDD = IDD_SETVERSIONINFO_DIALOG };
	CString	m_CompanyName;
    CString	m_FileVersion;
	CString	m_ProductVersion;
	CString	m_Comments;
	CString	m_LegalTradeMark;
	CString	m_LegalCopyright;
	CString	m_PrivateBuild;
	CString	m_SpecialBuild;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSetVersionInfoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	LPCTSTR GetValue(DWORD id);
	// Generated message map functions
	//{{AFX_MSG(CSetVersionInfoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnProcess();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	CCheckListBox m_FilesList;
  afx_msg void OnBnClickedCheckAll();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SETVERSIONINFODLG_H__D1F929D5_5CB5_42A0_87D8_010003992A62__INCLUDED_)
