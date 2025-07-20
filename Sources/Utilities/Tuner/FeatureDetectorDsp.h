#if !defined(AFX_FEATUREDETECTORDSP_H__952008C1_B9C8_11D5_9463_0080AD70FF26__INCLUDED_)
#define AFX_FEATUREDETECTORDSP_H__952008C1_B9C8_11D5_9463_0080AD70FF26__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "DispDlg.h"
// FeatureDetectorDsp.h : header file
/////////////////////////////////////////////////////////////////////////////
// CFeatureDetectorDsp dialog

class CFeatureDetectorDsp : public CDispDlg
{
private:
    CDIBView    m_Display;
    TVFrame     m_Frame;
// Construction
public:
	bool LoadDIB(BITMAPINFOHEADER *bmih);
	CFeatureDetectorDsp(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFeatureDetectorDsp)
	enum { IDD = ID_FEATUREDETECTOR };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFeatureDetectorDsp)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFeatureDetectorDsp)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FEATUREDETECTORDSP_H__952008C1_B9C8_11D5_9463_0080AD70FF26__INCLUDED_)
