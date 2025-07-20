#if !defined(AFX_RESIZEBLEDIALOG_H__1C979853_17A9_46AE_8007_9A6BEBEE0360__INCLUDED_)
#define AFX_RESIZEBLEDIALOG_H__1C979853_17A9_46AE_8007_9A6BEBEE0360__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ResizebleDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CResizebleDialog dialog
#define BOTTOM_ALIGN 0x01
#define RIGHT_ALIGN  0x02
#define BOTTOM_MOVE  0x04
#define RIGHT_MOVE   0x08

typedef struct tagDlgItems
{
    DWORD iD;
    DWORD flags;
    RECT orgpos;
    DWORD AlignTo1;
}DlgItem,*pDlgItem;

class FX_EXT_SHBASE CResizebleDialog : public CDialog
{
protected:
    pDlgItem m_pDlgItems;
    CRect    m_DlgOrgRC;
    unsigned m_ItemsCnt;
    bool     m_Updated;
public:
	void FitDialog();
	CResizebleDialog(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);   // standard constructor
    CResizebleDialog(UINT nIDTemplate, CWnd* pParentWnd = NULL);
    void    EnableFitting() { m_Updated=true; };
// Dialog Data
	//{{AFX_DATA(CResizebleDialog)
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResizebleDialog)
    //
	public:
	virtual BOOL Create(LPCTSTR lpszTemplateName, CWnd* pParentWnd = NULL);
    virtual BOOL Create( UINT nIDTemplate, CWnd* pParentWnd = NULL );
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CResizebleDialog)
	virtual BOOL OnInitDialog(pDlgItem pDlgItems, unsigned ItemsCnt);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESIZEBLEDIALOG_H__1C979853_17A9_46AE_8007_9A6BEBEE0360__INCLUDED_)
