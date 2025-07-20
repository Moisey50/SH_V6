#if !defined(AFX_FIMPARAMDIALOG_H__21C1A172_2EAF_11D3_8501_00A0C9616FBC__INCLUDED_)
#define AFX_FIMPARAMDIALOG_H__21C1A172_2EAF_11D3_8501_00A0C9616FBC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FImParamDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFImParamDialog dialog

class CFImParamDialog : public CDialog
{
// Construction
public:
	CFImParamDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFImParamDialog)
	enum { IDD = IDD_IM_PARAM_DIALOG };
	int		m_iFFTXc;
	int		m_iFFTYc;
	double	m_BinThreshold;
  int  m_iMeasExpansion ;
	BOOL	m_UseFFTFilter;
	CString	m_PowerSaveName;
	UINT	m_iPowerRadius;
	//BOOL	m_bSavePowerData;
	UINT	m_iMinAmplitude;
	int		m_iExposureBegin;
	int		m_iViewCX;
	int		m_iViewCY;
	BOOL	m_SaveExtendedInformation;
	int		m_iMinArea;
	int		m_iDiffractionRadius;
	int		m_iMaxSpotDia;
	int		m_iDiffractionMeasurementMethod;
	int		m_iDiffractionRadius_Y;
	int		m_iBackgroundDist;
	int		m_iImageSaveMode;
	BOOL	m_bImageZip;
	BOOL	m_bSave16Bits;
	CString	m_ImageSaveDir;
	BOOL	m_bAutoSave;
	int		m_iDiffrMaxSearchDist;
	BOOL	m_MeasBeamRotBySectors;
	BOOL	m_bViewDir;
  BOOL   m_bClear;
  BOOL m_bWriteHeadlines;
  BOOL  m_bSaveImage;
  BOOL m_bSaveVideo;
  BOOL m_bCam0;
  BOOL m_bCam1;


  BOOL m_bShowRadDisable;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFImParamDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFImParamDialog)
	afx_msg void OnPaint();
	afx_msg void OnViewDir();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnBnClickedAutoSave();
  int m_iThresholdValue;
  BOOL m_bFixThreshold;
  BOOL m_bBackSubstract;
  bool m_bUseExposureForBackground;
  afx_msg void OnBnClickedUseExposureForBackground();
  afx_msg void OnBnClickedSavePowerGrades();
  afx_msg void OnBnClickedClear();
  afx_msg void OnBnClickedOk();

  virtual afx_msg LRESULT OnEnableDisable(WPARAM wpar, LPARAM lpar);
  BOOL m_bLowPassOn;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FIMPARAMDIALOG_H__21C1A172_2EAF_11D3_8501_00A0C9616FBC__INCLUDED_)
