
// totalrenameDlg.h : header file
//

#pragma once
#include "FileScanner.h"

// CtotalrenameDlg dialog
class CtotalrenameDlg : public CDialog
{
protected:
    CFileScanner m_fs;
// Construction
public:
	CtotalrenameDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_TOTALRENAME_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedStart();
    CString m_FilePath;
    afx_msg void OnBnClickedSelectPath();
    bool SetRootPath(LPCTSTR path);
    int m_RenameMode;
};
