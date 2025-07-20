#if !defined(AFX_HISTOGRAMVIEW_H__32A46301_BAA2_11D5_9463_0080AD70FF26__INCLUDED_)
#define AFX_HISTOGRAMVIEW_H__32A46301_BAA2_11D5_9463_0080AD70FF26__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HistogramView.h : header file
//
#include "DispDlg.h"
/////////////////////////////////////////////////////////////////////////////
// CHistogramView dialog
#include <userinterface\DataView.h>

class CHistogramView : public CDispDlg
{
    CDataView   m_Display;
    TVFrame     m_Frame;
// Construction
public:
	bool LoadDIB(BITMAPINFOHEADER *bmih);
	CHistogramView(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CHistogramView)
	enum { IDD = IDD_HISTORGAMVIEW };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHistogramView)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CHistogramView)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HISTOGRAMVIEW_H__32A46301_BAA2_11D5_9463_0080AD70FF26__INCLUDED_)
