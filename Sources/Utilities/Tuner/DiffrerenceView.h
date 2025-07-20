#if !defined(AFX_DIFFRERENCEVIEW_H__91ABEC41_BC1B_11D5_9463_0080AD70FF26__INCLUDED_)
#define AFX_DIFFRERENCEVIEW_H__91ABEC41_BC1B_11D5_9463_0080AD70FF26__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DiffrerenceView.h : header file
//
#include "DispDlg.h"
#include <userinterface\DataView.h>
#include <imageproc\MotionDetector.h>

/////////////////////////////////////////////////////////////////////////////
// CDiffrerenceView dialog

class CDiffrerenceView : public CDispDlg, public CMotionDetector
{
private:
    CDIBView      m_Display;
    CDataView   m_TimeLine;
// Construction
public:
	    CDiffrerenceView(CWnd* pParent = NULL);   // standard constructor
        bool LoadDIB(BITMAPINFOHEADER *bmih);
        virtual void Reset() { m_TimeLine.Reset(); }; 
    
// Dialog Data
	//{{AFX_DATA(CDiffrerenceView)
	enum { IDD = IDD_DIFFERENCEVIEW };
	CProgressCtrl	m_LevelView2;
	CProgressCtrl	m_LevelView;
	CString	m_Value;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDiffrerenceView)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDiffrerenceView)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIFFRERENCEVIEW_H__91ABEC41_BC1B_11D5_9463_0080AD70FF26__INCLUDED_)
