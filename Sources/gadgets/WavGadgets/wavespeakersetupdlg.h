#if !defined(AFX_WAVESPEAKERSETUPDLG_H__C5C76C71_EB3F_471A_93BD_7B63DE0D0E94__INCLUDED_)
#define AFX_WAVESPEAKERSETUPDLG_H__C5C76C71_EB3F_471A_93BD_7B63DE0D0E94__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WaveSpeakerSetupDlg.h : header file
//

#include <Gadgets\gadbase.h>
#include <gadgets\stdsetup.h>

/////////////////////////////////////////////////////////////////////////////
// WaveSpeakerSetupDlg dialog

class WaveSpeaker;
class WaveSpeakerSetupDlg : public CGadgetSetupDialog
{
    // Construction
public:
    WaveSpeakerSetupDlg(CGadget* pGadget, CWnd* pParent = NULL);   // standard constructor
    virtual void UploadParams();
    // Dialog Data
    //{{AFX_DATA(WaveSpeakerSetupDlg)
    enum { IDD = IDD_SPEAKERSETUPDLG };
    int	m_ComboIndex;
    //}}AFX_DATA
    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(WaveSpeakerSetupDlg)
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL
    // Implementation
protected:

    // Generated message map functions
    //{{AFX_MSG(WaveSpeakerSetupDlg)
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WAVESPEAKERSETUPDLG_H__C5C76C71_EB3F_471A_93BD_7B63DE0D0E94__INCLUDED_)
