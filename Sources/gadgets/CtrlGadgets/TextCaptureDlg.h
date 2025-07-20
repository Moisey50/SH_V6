#if !defined(AFX_TEXTCAPTUREDLG_H__CCC485B3_D1FA_4989_9765_D3BB396432E4__INCLUDED_)
#define AFX_TEXTCAPTUREDLG_H__CCC485B3_D1FA_4989_9765_D3BB396432E4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextCaptureDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTextCaptureDlg dialog
#include <helpers\PrxyWnd.h>
#include <helpers\FXParser2.h>

#define UM_SEND_DATA (WM_APP+100)
 
class CTextCaptureDlg : public CResizebleDialog
{
protected:
    PrxyWndCallback m_Callback;
    void*           m_UserParam;
public:
    FXString        m_FileName ;
	CTextCaptureDlg(CWnd* pParent = NULL);   // standard constructor
    void Init(PrxyWndCallback pwc, void *pUserParam) { m_Callback=pwc; m_UserParam=pUserParam; }
// Dialog Data
	//{{AFX_DATA(CTextCaptureDlg)
	enum { IDD = IDD_TEXTCAPTURE_DLG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextCaptureDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTextCaptureDlg)
	afx_msg void OnSend();
	afx_msg void OnChangeEditCtrl();
	virtual BOOL OnInitDialog();
  afx_msg LRESULT OnSendData( WPARAM wParam , LPARAM lParam ) ;
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL Create(int iTemplate, CWnd* pParentWnd = NULL);
    afx_msg void OnBnClickedFromFile();
    int ReadFile(LPCTSTR pFileName);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTCAPTUREDLG_H__CCC485B3_D1FA_4989_9765_D3BB396432E4__INCLUDED_)
