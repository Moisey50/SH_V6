#if !defined(AFX_CODE128READER_H__AB355B5C_9902_4499_ADD1_45D3F56DCB8A__INCLUDED_)
#define AFX_CODE128READER_H__AB355B5C_9902_4499_ADD1_45D3F56DCB8A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Code128Reader.h : header file
//

#include "DispDlg.h"
#include <video\dibview.h>
#include <imageproc\clusters\Clusters.h>
#include <userinterface\DataView.h>

/////////////////////////////////////////////////////////////////////////////
// CCode128Reader dialog

class CCode128Reader : public CDispDlg
{
private:
	CDIBView    m_Display;
    CDataView   m_DataView;
    TVFrame     m_Frame;
    CString		m_ResultStr;
public:
	CCode128Reader(CWnd* pParent = NULL);   // standard constructor
	void OnLineSelection(CPoint& lu, CPoint& rb);
	bool LoadDIB(BITMAPINFOHEADER *bmih);

// Dialog Data
	//{{AFX_DATA(CCode128Reader)
	enum { IDD = ID_CODE128 };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCode128Reader)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCode128Reader)
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	afx_msg void OnRunTvdb();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CODE128READER_H__AB355B5C_9902_4499_ADD1_45D3F56DCB8A__INCLUDED_)
