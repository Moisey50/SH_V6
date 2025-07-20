#if !defined(AFX_FLWCAPSTPDIALOG_H__57BCA030_C12C_4324_80CE_55CC9578B675__INCLUDED_)
#define AFX_FLWCAPSTPDIALOG_H__57BCA030_C12C_4324_80CE_55CC9578B675__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FLWCapStpDialog.h : header file
//
#include <Gadgets\gadbase.h>
#include <gadgets\stdsetup.h>

/////////////////////////////////////////////////////////////////////////////
// CFLWCapStpDialog dialog

class FLWCapture;
class CFLWCapStpDialog : public CGadgetSetupDialog
{
// Construction
public:
    CFLWCapStpDialog(CGadget* pGadget, CWnd* pParent = NULL);
// Dialog Data
	//{{AFX_DATA(CFLWCapStpDialog)
	enum { IDD = IDD_FLW_CAPTURE_DLG };
	CString	m_FileName;
	BOOL	m_bLoop;
	BOOL	m_bFrameRate;
	UINT	m_FrameRate;
	BOOL	m_bExtTrigger;
  BOOL  m_bDoLog;
  BOOL  m_bAutoRewindToStart;
  int    m_FrameNumberMin = 0;
  int    m_FrameNumberMax = 100 ;
  double m_dFrameTimeMin_ms = 0. ;
  double m_dFrameTimeMax_ms = 1000. ;
  int    m_PlayNumberMin = 0 ;
  int    m_PlayNumberMax = 100 ;
  double m_dPlayTimeMin_ms = 0. ;
  double m_dPlayTimeMax_ms = 1000. ;
  double m_dTimeZoom = 1.0 ;
  BOOL   m_bIterateByFrames = TRUE ;
  BOOL   m_bIterateByTime = FALSE ;
  int    m_bByFRames = 0 ; // for radio button auto update
//}}AFX_DATA
    virtual bool Show(CPoint point, LPCTSTR uid);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFLWCapStpDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL
    virtual void UploadParams();
// Implementation
protected:
	void UpdateCtrls();

	// Generated message map functions
	//{{AFX_MSG(CFLWCapStpDialog)
	afx_msg void OnBrowseFilename();
	afx_msg void OnConstFrameRate();
	afx_msg void OnSoftwareTrigger();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnBnClickedApply();
  afx_msg void OnBnClickedIterateByFrames();
  afx_msg void OnBnClickedIterateByTime();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FLWCAPSTPDIALOG_H__57BCA030_C12C_4324_80CE_55CC9578B675__INCLUDED_)
