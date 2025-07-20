#if !defined(AFX_AVICAPTUREDIALOG_H__D665BE2D_C8A4_43B5_B786_8E123213B1B4__INCLUDED_)
#define AFX_AVICAPTUREDIALOG_H__D665BE2D_C8A4_43B5_B786_8E123213B1B4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AviCaptureDialog.h : header file
//

#include <Gadgets\gadbase.h>
#include "Resource.h"
#include <gadgets\stdsetup.h>

/////////////////////////////////////////////////////////////////////////////
// AviCaptureDialog dialog

class AviCapture;
class AviCaptureDialog : public CGadgetSetupDialog
{
// Construction
public:
	AviCaptureDialog(CGadget* pGadget, CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(AviCaptureDialog)
	enum { IDD = IDD_AVI_CAPTURE_DLG };
	FXString	m_FileName;
	BOOL	m_bSoftwareTrigger;
	DWORD	m_FrameRate;
	BOOL	m_LoopFilm;
    BOOL    m_ReservePins;
    BOOL    m_ChunkMode;
    int     m_ReservePinsNumber;
	//}}AFX_DATA
// Overrides
    virtual bool Show(CPoint point, LPCTSTR uid);
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(AviCaptureDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual void UploadParams();
	
	// Generated message map functions
	//{{AFX_MSG(AviCaptureDialog)
	afx_msg void OnBrowseFilename();
	virtual BOOL OnInitDialog();
	afx_msg void OnSoftwareTrigger();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedReserveOutputs();
    FXString GetFileName();
    afx_msg void OnBnClickedChankmode();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AVICAPTUREDIALOG_H__D665BE2D_C8A4_43B5_B786_8E123213B1B4__INCLUDED_)
