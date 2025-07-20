#if !defined(AFX_HELPVIEW_H__393D7FB5_1CD2_43C3_80C1_E1B96A61B9E1__INCLUDED_)
#define AFX_HELPVIEW_H__393D7FB5_1CD2_43C3_80C1_E1B96A61B9E1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HelpView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CHelpView html view

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif
#include <afxhtml.h>

class CHelpView : public CHtmlView
{
    CString m_tmpFileName;
    CString m_DefaultDir;
    CMapStringToString m_AliasedFiles;
protected:
	CHelpView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CHelpView)

// html Data
public:
	//{{AFX_DATA(CHelpView)
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

// Attributes
public:

// Operations
public:
	bool CreateAliasedURL(LPCTSTR alias, LPCTSTR doc_htm);
	void SetDefaultBrowseDirectory(LPCTSTR path);
	void Navigate2(LPCTSTR lpszURL, DWORD dwFlags = 0,
		LPCTSTR lpszTargetFrameName = NULL, LPCTSTR lpszHeaders = NULL,
		LPVOID lpvPostData = NULL, DWORD dwPostDataLen = 0);
	void Navigate(LPCTSTR URL, DWORD dwFlags = 0,
		LPCTSTR lpszTargetFrameName = NULL,
		LPCTSTR lpszHeaders = NULL, LPVOID lpvPostData = NULL,
		DWORD dwPostDataLen = 0);
    bool MemoryNavigate(LPCTSTR httpdoc);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHelpView)
	public:
	virtual BOOL Create(CWnd* pParentWnd, DWORD dwAddStyle);
	virtual void OnDocumentComplete(LPCTSTR lpszURL);
	virtual void OnBeforeNavigate2(LPCTSTR lpszURL, DWORD nFlags, LPCTSTR lpszTargetFrameName, CByteArray& baPostedData, LPCTSTR lpszHeaders, BOOL* pbCancel);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool PreProcessURL(CString& URL);
	virtual ~CHelpView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CHelpView)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnDestroy();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HELPVIEW_H__393D7FB5_1CD2_43C3_80C1_E1B96A61B9E1__INCLUDED_)
