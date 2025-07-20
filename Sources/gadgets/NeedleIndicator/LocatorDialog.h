#if !defined(AFX_LOCATORDIALOG_H__FE62953F_2A07_4572_878D_1C41332A8E5B__INCLUDED_)
#define AFX_LOCATORDIALOG_H__FE62953F_2A07_4572_878D_1C41332A8E5B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LOcatorDialog.h : header file
//
#include <gadgets\shkernel.h>
#include "afxwin.h"
#include <gadgets\stdsetup.h>

/////////////////////////////////////////////////////////////////////////////
// LocatorDialog dialog
class Locator;
class LocatorDialog : public CGadgetSetupDialog
{
friend bool LocatorDrawExFunc(HDC hdc,RECT& rc,CDIBViewBase* view, LPVOID lParam);
friend void LocatorDipPPEvent(int Event, void *Data, void *pParam, CDIBViewBase* wParam);
protected:
	CDIBView m_View;
	Locator *m_pParent;
	CPen	 m_rPen;
    int      m_AngleInt,m_RadiusInt;
protected:
	bool DrawPoint(HDC hdc,RECT& rc,CDIBViewBase* view);
	void OnChangePoint(CPoint& pt,  CDIBViewBase* wParam);
    CComboBox m_Angle;
    CEdit     m_Radius;
public:
	LocatorDialog(Locator* pGadget, CWnd* pParent = NULL);
	void LoadFrame(const pTVFrame frame);

// Dialog Data
	//{{AFX_DATA(LocatorDialog)
	enum { IDD = IDD_VIDEO_LOCATOR };
	long	m_XPos;
	long	m_YPos;
	//}}AFX_DATA
// Overrides
    virtual bool Show(CPoint point, LPCTSTR uid);
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(LocatorDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual void UploadParams();
	// Generated message map functions
	//{{AFX_MSG(LocatorDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnChangeEditXpos();
	afx_msg void OnChangeEditYpos();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnCbnSelchangeAngle();
    afx_msg void OnEnKillfocusEditRadius();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOCATORDIALOG_H__FE62953F_2A07_4572_878D_1C41332A8E5B__INCLUDED_)
