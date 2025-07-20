#if !defined(AFX_UPCREADERDSP_H__952008C1_B9C8_11D5_9463_0080AD70FF26__INCLUDED_)
#define AFX_UPCREADERDSP_H__952008C1_B9C8_11D5_9463_0080AD70FF26__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DispDlg.h"
// FeatureDetectorDsp.h : header file
/////////////////////////////////////////////////////////////////////////////
// CUPCReaderDsp dialog
#include <imageproc\clusters\Clusters.h>
#include <userinterface\DataView.h>

class CUPCReaderDsp : public CDispDlg
{
private:
    CDIBView      m_Display;
    CDataView   m_DataView;
    TVFrame     m_Frame;
// Construction
public:
	CUPCReaderDsp(CWnd* pParent = NULL);   // standard constructor
	void OnLineSelection(CPoint& lu, CPoint& rb);
	bool LoadDIB(BITMAPINFOHEADER *bmih);
// Dialog Data
	//{{AFX_DATA(CUPCReaderDsp)
	enum { IDD = ID_UPCREADER};
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CUPCReaderDsp)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CUPCReaderDsp)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnDestroy();
    afx_msg void OnBnClickedRunTvdb();
    CString m_ResultStr;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FEATUREDETECTORDSP_H__952008C1_B9C8_11D5_9463_0080AD70FF26__INCLUDED_)
