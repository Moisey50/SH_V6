#pragma once
#include "afxcmn.h"


// CBrowseRemoteGraphsDlg dialog

class CBrowseRemoteGraphsDlg : public CDialog
{
	DECLARE_DYNAMIC(CBrowseRemoteGraphsDlg)

	CStringArray m_GraphNames;
	CString m_CurDir;
	CString m_RootDir;
	CString m_SelGraph;
	CImageList m_Icons;
public:
	CBrowseRemoteGraphsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CBrowseRemoteGraphsDlg();

	void SetRemoteGraphs(CStringArray* Graphs) { m_GraphNames.Append(*Graphs); }

// Dialog Data
	enum { IDD = IDD_BROWSEREMOTEGRAPHS_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void ChangeCurDir(CString& dir);
	void ProcessItem(int nItem);

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_Graphs;
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();
public:
	afx_msg void OnNMDblclkGraphs(NMHDR *pNMHDR, LRESULT *pResult);
};
