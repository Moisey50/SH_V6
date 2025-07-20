#if !defined(AFX_FFTFILTER_H__59EF7DD7_1F0C_11D3_84EE_00A0C9616FBC__INCLUDED_)
#define AFX_FFTFILTER_H__59EF7DD7_1F0C_11D3_84EE_00A0C9616FBC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FFTFilter.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFFTFilter dialog

class CFFTFilter : public CDialog
{
// Construction
public:
	CFFTFilter(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFFTFilter)
	enum { IDD = IDD_FFT_FILTER };
	CString	m_Rect1;
	CString	m_Rect2;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFFTFilter)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFFTFilter)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FFTFILTER_H__59EF7DD7_1F0C_11D3_84EE_00A0C9616FBC__INCLUDED_)
