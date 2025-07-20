#if !defined(AFX_WAVEFILECAPTURESETUPDLG_H__231AF1DA_0739_40CC_8F53_CE6D8471AA2D__INCLUDED_)
#define AFX_WAVEFILECAPTURESETUPDLG_H__231AF1DA_0739_40CC_8F53_CE6D8471AA2D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WaveFileCaptureSetupDlg.h : header file
//
#include <Gadgets\gadbase.h>
#include "Resource.h"
#include <gadgets\stdsetup.h>

/////////////////////////////////////////////////////////////////////////////
// CWaveFileCaptureSetupDlg dialog
class WavFileCapture;
class CWaveFileCaptureSetupDlg : public CGadgetSetupDialog
{
    // Construction
public:
    CWaveFileCaptureSetupDlg(CGadget* pGadget, CWnd* pParent = NULL);   // standard constructor
    virtual void UploadParams();
    // Dialog Data
    //{{AFX_DATA(CWaveFileCaptureSetupDlg)
    enum { IDD = IDD_WAVSRCSETUPDLG };
    CString	m_Filename;
    BOOL m_bLoop;
    //}}AFX_DATA
    // Overrides
    virtual bool Show(CPoint point, LPCTSTR uid);
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CWaveFileCaptureSetupDlg)
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

    // Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(CWaveFileCaptureSetupDlg)
    afx_msg void OnBrowseFilename();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
public:
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WAVEFILECAPTURESETUPDLG_H__231AF1DA_0739_40CC_8F53_CE6D8471AA2D__INCLUDED_)
