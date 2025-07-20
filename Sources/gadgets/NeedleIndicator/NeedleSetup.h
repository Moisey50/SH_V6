#if !defined(AFX_NEEDLESETUP_H__DCAFFCC9_3DC9_4F6C_B83A_1BCE8755DDD6__INCLUDED_)
#define AFX_NEEDLESETUP_H__DCAFFCC9_3DC9_4F6C_B83A_1BCE8755DDD6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// NeedleSetup.h : header file
//
#include <gadgets\shkernel.h>
#include <gadgets\stdsetup.h>

/////////////////////////////////////////////////////////////////////////////
// CNeedleSetup dialog

class NeedleDetector;
class CNeedleSetup : public CGadgetSetupDialog
{
friend bool NeedleDrawExFunc(HDC hdc,RECT& rc,CDIBViewBase* view, LPVOID lParam);
private:
	CDIBView		 m_View;
	CPen			 m_rPen, m_gPen;
	CSize			 m_FrameSize;
protected:
	bool Draw(HDC hdc,RECT& rc,CDIBViewBase* view);
public:
	CNeedleSetup(NeedleDetector* pGadget, CWnd* pParent = NULL);   // standard constructor
	void LoadFrame(const pTVFrame frame);

// Dialog Data
	//{{AFX_DATA(CNeedleSetup)
	enum { IDD = IDD_NEEDLE_SETUP };
	double	m_FstPointValue;
	int		m_FstPointX;
	int		m_LinePos;
	double	m_SdPointValue;
	int		m_SdPointX;
	//}}AFX_DATA


// Overrides
    virtual bool Show(CPoint point, LPCTSTR uid);
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNeedleSetup)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual void UploadParams();
	// Generated message map functions
	//{{AFX_MSG(CNeedleSetup)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnChangeEditFstpointValue();
	afx_msg void OnChangeEditFstpointX();
	afx_msg void OnChangeEditLineposition();
	afx_msg void OnChangeEditSdpointValue();
	afx_msg void OnChangeEditSdpointX();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEEDLESETUP_H__DCAFFCC9_3DC9_4F6C_B83A_1BCE8755DDD6__INCLUDED_)
