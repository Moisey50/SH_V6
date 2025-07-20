#if !defined(AFX_DEBUGENVELOPDLG_H__A035431B_03EF_4DE1_9B7C_4414A340FADC__INCLUDED_)
#define AFX_DEBUGENVELOPDLG_H__A035431B_03EF_4DE1_9B7C_4414A340FADC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DebugEnvelopDlg.h : header file
//

#include "resource.h"
#include <userinterface\StaticFrame.h>

/////////////////////////////////////////////////////////////////////////////
// CDebugEnvelopDlg dialog

class CTiedDebugRender;

class CDebugEnvelopDlg : public CDialog
{
public:
	CStaticFrame* m_pOutFrame;	// visualizing output stream
	CStaticFrame* m_pInFrame;	// visualizing input stream
    CTiedDebugRender *m_pInGadget, *m_pOutGadget, *m_pGadget;
    CMenu             m_Menu;
// Construction
public:
	CDebugEnvelopDlg(CWnd* pParent = NULL);   // standard constructor
	void ArrangeWindows();
    void SetInGadget(CTiedDebugRender* pGadget) { m_pInGadget=pGadget; };
    void SetOutGadget(CTiedDebugRender* pGadget) { m_pOutGadget=pGadget; };
// Dialog Data
	//{{AFX_DATA(CDebugEnvelopDlg)
	enum { IDD = IDD_DEBUG_ENVELOP };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDebugEnvelopDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CDebugEnvelopDlg)
	virtual void OnCancel();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnDefault();
	afx_msg void OnFramerate();
	afx_msg void OnText();
	afx_msg void OnVideo();
	afx_msg void OnContainerview();
    afx_msg LRESULT OnCheckStateChange(WPARAM wParam, LPARAM lParam);
	afx_msg void OnFrameinfo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnMove(int x, int y);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DEBUGENVELOPDLG_H__A035431B_03EF_4DE1_9B7C_4414A340FADC__INCLUDED_)
