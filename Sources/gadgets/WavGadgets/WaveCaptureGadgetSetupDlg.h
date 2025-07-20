#if !defined(AFX_WAVECAPTUREGADGETSETUPDLG_H__9561899B_A36B_4624_8BD7_3986DE89F28F__INCLUDED_)
#define AFX_WAVECAPTUREGADGETSETUPDLG_H__9561899B_A36B_4624_8BD7_3986DE89F28F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WaveCaptureGadgetSetupDlg.h : header file
//

#include <Gadgets\gadbase.h>

/////////////////////////////////////////////////////////////////////////////
// WaveCaptureGadgetSetupDlg dialog

class WaveCapture;
class WaveCaptureGadgetSetupDlg : public CGadgetSetupDialog
{
// Construction
public:
	WaveCaptureGadgetSetupDlg(WaveCapture* pGadget, CWnd* pParent = NULL);   // standard constructor

   virtual void UploadParams(CGadget* Gadget);
// Dialog Data
	//{{AFX_DATA(WaveCaptureGadgetSetupDlg)
	enum { IDD = IDD_MICSETUPDLG };
	int	m_ComboIndex;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(WaveCaptureGadgetSetupDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(WaveCaptureGadgetSetupDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WAVECAPTUREGADGETSETUPDLG_H__9561899B_A36B_4624_8BD7_3986DE89F28F__INCLUDED_)
