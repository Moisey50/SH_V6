#if !defined(AFX_CODE39READER_H__EFBD432A_1897_4117_9489_F69781905BA3__INCLUDED_)
#define AFX_CODE39READER_H__EFBD432A_1897_4117_9489_F69781905BA3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Code39Reader.h : header file
//
#include "DispDlg.h"
#include <imageproc\clusters\Clusters.h>
#include <userinterface\DataView.h>

/////////////////////////////////////////////////////////////////////////////
// CCode39Reader dialog

class CCode39Reader : public CDispDlg
{
private:
	CDIBView    m_Display;
    CDataView   m_DataView;
    TVFrame     m_Frame;
    CString		m_ResultStr;
public:
	CCode39Reader(CWnd* pParent = NULL);   // standard constructor
	void OnLineSelection(CPoint& lu, CPoint& rb);
	bool LoadDIB(BITMAPINFOHEADER *bmih);

// Dialog Data
	//{{AFX_DATA(CCode39Reader)
	enum { IDD = ID_CODE39 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCode39Reader)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCode39Reader)
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedRunTvdb();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CODE39READER_H__EFBD432A_1897_4117_9489_F69781905BA3__INCLUDED_)
