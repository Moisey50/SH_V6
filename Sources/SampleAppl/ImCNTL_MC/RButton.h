#if !defined(AFX_RBUTTON_H__DBB988A0_F87B_11D6_95D4_00A024775726__INCLUDED_)
#define AFX_RBUTTON_H__DBB988A0_F87B_11D6_95D4_00A024775726__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RButton.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRButton dialog

class CRButton : public CDialog
{
// Construction
public:
	double m_dScaleYCam1,m_dScaleYCam2;
	double m_dScaleXCam1,m_dScaleXCam2;
	CRButton(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRButton)
	enum { IDD = IDD_RIGHT_BUTTON_DLG };
	int		m_iViewMode;
	BOOL	m_bShowMicrons;
	CString	m_StringScales;
	CString	m_AutoSaveFileName;
	BOOL	m_IsAutoSave;
  BOOL m_bWriteHeadlines;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRButton)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRButton)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
  CString m_SpotSearchCoordinates;
  CPoint  m_SpotSearch ;
  afx_msg void OnBnClickedAutomaticSave();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RBUTTON_H__DBB988A0_F87B_11D6_95D4_00A024775726__INCLUDED_)
