// DlgProxy.h : header file
//

#if !defined(AFX_DLGPROXY_H__A7F453BE_3255_11D3_8D4E_000000000000__INCLUDED_)
#define AFX_DLGPROXY_H__A7F453BE_3255_11D3_8D4E_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "ImagView.h"

class CImCNTLDlg;

/////////////////////////////////////////////////////////////////////////////
// CImCNTLDlgAutoProxy command target

class CImCNTLDlgAutoProxy : public CCmdTarget
{
	DECLARE_DYNCREATE(CImCNTLDlgAutoProxy)

	CImCNTLDlgAutoProxy();           // protected constructor used by dynamic creation

// Attributes
public:
	CImCNTLDlg* m_pDialog;

// Operations
public:
	int m_iIndex;
	double m_dGrabRequestTime;
	int m_iLock;
	CColorSpot m_LastSpot;
	CImageView * m_View;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CImCNTLDlgAutoProxy)
	public:
	virtual void OnFinalRelease();
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CImCNTLDlgAutoProxy();

	// Generated message map functions
	//{{AFX_MSG(CImCNTLDlgAutoProxy)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	DECLARE_OLECREATE(CImCNTLDlgAutoProxy)

	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CImCNTLDlgAutoProxy)
	afx_msg long GrabBack();
	afx_msg long Grab(long bSubstrBack);
	afx_msg long MeasureBlobs(double dNormThreshold); // content commented out for Eilat ATJ
	afx_msg BSTR GetBlobParam(long iBlobNumber);
	afx_msg void SetFFTFlag(long bDoFFT);
	afx_msg void SetCameraScale(double dScaleXmicronPerPixel, double dScaleYmicronPerPixel);
	afx_msg double GetCameraScale(long iAxeNumber);
	afx_msg long InitOperation(long InitValue);
	afx_msg long GetExposure(); // content commented out for Eilat ATJ
	afx_msg long SetExposure(long iNewExposureInScans);// content commented out for Eilat ATJ
	afx_msg long MeasureLines(double dNormThres);
	afx_msg BSTR GetLineParam(long LineNumber);
	afx_msg long GetLastMinIntensity();
	afx_msg long GetLastMaxIntensity();
	afx_msg void SetFFTZone(long iXc, long iYc);
	afx_msg long DoFFTFiltration();
	afx_msg long SaveRestoreImage(long bSave);
	afx_msg double SetGetMeasAreaExpansion(long bSet, double dExp_in_um);
	afx_msg double GetMaxIntensity();
	afx_msg long SetWinPos(long lx, long ly);
	afx_msg void SetExposureStartPosition(long iExpStartPos); // content commented out for Eilat ATJ
	afx_msg long GetExposureStartPosition(); // content commented out for Eilat ATJ
	afx_msg long MeasurePower();
	afx_msg BSTR GetPowerData(long iBlobNumber);
	afx_msg void SetIntegrationRadius(long iRadius);
	afx_msg long GetIntegrationRadius();
	afx_msg double MeasLinePower(long iLineDirection, long iMeasureHalfWidth, long iCompensationWidth);
	afx_msg void SetMinArea(long iMinArea);
	afx_msg double AbstractCall(long iCallID, LPCTSTR pszParameters, long iPar, double dPar);
	afx_msg long SetSyncMode(long bAsyncMode); // edited for Eilat ATJ dev
	afx_msg long SetMarkerPosisiton(long iMarkerType, long iXPos, long iYPos, long dwMarkerColor);
	afx_msg BSTR GetDiffractionMeasPar();
	afx_msg BSTR SetDiffractionMeasPar(long iMeasMode, long iXDist_pix, long iYDist_pix, long iBackgoundDist_pix);
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGPROXY_H__A7F453BE_3255_11D3_8D4E_000000000000__INCLUDED_)
