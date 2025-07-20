#if !defined(AFX_GRABBERCONTROL_H__9DDD339E_06AC_11D3_84DD_00A0C9616FBC__INCLUDED_)
#define AFX_GRABBERCONTROL_H__9DDD339E_06AC_11D3_84DD_00A0C9616FBC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GrabberControl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGrabberControl dialog
class CImageView ;

class CGrabberControl : public CDialog
{
// Construction
public:
	CImageView * m_Owner;
	CGrabberControl(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CGrabberControl)
	enum { IDD = IDD_GRABBER_CONTROL };
	int		m_Trigger;
	int		m_TriggerMode;
	int		m_TriggerSource;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGrabberControl)
	public:
	virtual void OnFinalRelease();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CGrabberControl)
	afx_msg void OnTrigger();
	afx_msg void OnTriggerMode();
	afx_msg void OnTriggerSource();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CGrabberControl)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GRABBERCONTROL_H__9DDD339E_06AC_11D3_84DD_00A0C9616FBC__INCLUDED_)
