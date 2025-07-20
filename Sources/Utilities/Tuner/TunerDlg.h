// TunerDlg.h : header file
//

#if !defined(AFX_TUNERDLG_H__7F1414C8_8027_11D5_9463_F04F70C1402B__INCLUDED_)
#define AFX_TUNERDLG_H__7F1414C8_8027_11D5_9463_F04F70C1402B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CTunerDlg dialog
#include <files\SrcList.h>
#include "DispDlg.h"

class CTunerDlg : public CDialog
{
private:
    CDispDlg*   m_DispDlg;
    CDIBView    m_Display;
    CSrcList    *m_pFL;
    FXString    m_LastDir;
    FXString    m_fName;
    FXString    m_AppName;
    BOOL        m_Status;
    HACCEL      m_hAccel;
    bool        m_TranslateAccel;
    bool        m_SliderChangePos;
    int         m_StatePlay;
public:
	CString		m_CmdLine;
// Construction
public:
	bool LoadDIB(BITMAPINFOHEADER* bmih);
	void SaveSettings();
	void LoadSettings();
	CTunerDlg(CWnd* pParent = NULL);	// standard constructor
	bool LoadFile(const char * fName);
// Dialog Data
	//{{AFX_DATA(CTunerDlg)
	enum { IDD = IDD_TUNER_DIALOG };
	CSliderCtrl	m_ClipSlider;
	CString	m_FileName;
	CString	m_FileNmbInfo;
	CString	m_SaveAsFileName1;
	int		m_DspMethod;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTunerDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	// Generated message map functions
	//{{AFX_MSG(CTunerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnFileopen();
	afx_msg void OnDestroy();
	afx_msg void OnFileNext();
	afx_msg void OnFilePrevious();
	afx_msg void OnRunTvdb();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnHome();
	afx_msg void OnEnd();
	afx_msg void OnFileDelete();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnFilePlay();
	afx_msg void OnSaveas1();
	afx_msg void OnHistogram();
	afx_msg void OnFeaturedetectors();
	afx_msg void OnDiffrence();
	afx_msg void OnZonedetector();
	afx_msg void OnSeeknumber();
	afx_msg void OnUpcreader();
	afx_msg void OnCode39();
	afx_msg void OnCode128();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TUNERDLG_H__7F1414C8_8027_11D5_9463_F04F70C1402B__INCLUDED_)
