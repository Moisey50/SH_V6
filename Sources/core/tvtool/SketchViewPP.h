#if !defined(AFX_SKETCHVIEWPP_H__554B9984_03F0_469F_A757_637D34F2F35A__INCLUDED_)
#define AFX_SKETCHVIEWPP_H__554B9984_03F0_469F_A757_637D34F2F35A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SketchViewPP.h : header file
//

//#include <gadgets/tvdbase.h>
#include <gadgets/shkernel.h>
#include <gadgets/tview.h>
/////////////////////////////////////////////////////////////////////////////
// CSketchViewPP dialog

class CSketchViewPP : public CPropertyPage
{
	DECLARE_DYNCREATE(CSketchViewPP)

// Construction
public:
	CSketchViewPP();
	~CSketchViewPP();

  void InitViewType(ViewType vt);
  ViewType GetViewType();

// Dialog Data
	//{{AFX_DATA(CSketchViewPP)
	enum { IDD = IDD_VIEWSETUP };
	CString	m_sViewType;
	BOOL	m_UseStraightLines;
	BOOL	m_ShowSplash;
	CString	m_OffsetValue;
    BOOL    m_ViewActivity;
	//}}AFX_DATA


// Overrides
public:
	BOOL OnInitDialog();
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CSketchViewPP)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CSketchViewPP)
	afx_msg void OnShowSplash();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SKETCHVIEWPP_H__554B9984_03F0_469F_A757_637D34F2F35A__INCLUDED_)
