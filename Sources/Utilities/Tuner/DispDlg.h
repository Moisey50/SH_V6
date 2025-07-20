#if !defined(AFX_DISPDLG_H__952008C2_B9C8_11D5_9463_0080AD70FF26__INCLUDED_)
#define AFX_DISPDLG_H__952008C2_B9C8_11D5_9463_0080AD70FF26__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// DispDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDispDlg dialog

class CDispDlg : public CDialog
{
    UINT  m_Template;
// Construction
public:
	CDispDlg( UINT nIDTemplate, CWnd* pParentWnd = NULL ); // standard constructor
	virtual bool LoadDIB(BITMAPINFOHEADER *bmih)=0;
    virtual void Reset() {};
    bool         IsType(UINT Template) { return (m_Template==Template);};
// Dialog Data
	//{{AFX_DATA(CDispDlg)
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDispDlg)
	public:
	virtual BOOL Create( CWnd* pParentWnd = NULL );
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDispDlg)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DISPDLG_H__952008C2_B9C8_11D5_9463_0080AD70FF26__INCLUDED_)
