#if !defined(AFX_POLYSETUPDIALOG_H__7BCEE044_9902_4ECC_A987_CA4A63F5A695__INCLUDED_)
#define AFX_POLYSETUPDIALOG_H__7BCEE044_9902_4ECC_A987_CA4A63F5A695__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PolySetupDialog.h : header file
//

#include <gadgets\gadbase.h>
#include <gadgets\stdsetup.h>
#include "FileX.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CPolySetupDialog dialog

class CPolySetupDialog : public CGadgetSetupDialog
{
// Construction
public:
	CPolySetupDialog(CGadget* pGadget, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPolySetupDialog)
	enum { IDD = IDD_POLY_SETUP_DLG };
	double	m_ax;
	double	m_ay;
	double	m_bx;
	double	m_by;
	double	m_cx;
	double	m_cy;
	double	m_dx;
	double	m_dy;
	double	m_ex;
	double	m_ey;
	double	m_fx;
	double	m_fy;
	double	m_xC;
	double	m_yC;
	//}}AFX_DATA

  virtual bool Show( CPoint point , LPCTSTR uid) ;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPolySetupDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual void UploadParams(CGadget* Gadget);
	// Generated message map functions
	//{{AFX_MSG(CPolySetupDialog)
	afx_msg void OnLoad();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_POLYSETUPDIALOG_H__7BCEE044_9902_4ECC_A987_CA4A63F5A695__INCLUDED_)
